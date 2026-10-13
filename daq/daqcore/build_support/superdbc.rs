//! Generate wire models directly from CANpiler's schema and maintain the fallback.

use std::{error, fs, path};
use std::io::Write as _;

type Result<T> = std::result::Result<T, Box<dyn error::Error>>;

/// Refresh the checked-in models when a schema is available
/// then supply the models to compile through OUT_DIR
pub fn build(schema: &path::Path, fallback: &path::Path, output: &path::Path) -> Result<()> {
    let content = match fs::read_to_string(schema) {
        Ok(text) => {
            // Keep the fallback current so it can be committed with Pydantic
            // changes and used by fresh checkouts that have no schema artifact.
            let content = generate(&text)
                .map_err(|error| format!("cannot generate from {}: {error}", schema.display()))?;
            write_if_changed(fallback, &content)
                .map_err(|error| format!("cannot refresh {}: {error}", fallback.display()))?;
            content
        }
        // Missing artifacts are expected on a fresh checkout
        // Other read errors propagate instead of activating fallback
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => fs::read_to_string(fallback)
            .map_err(|error| {
                format!(
                    "schema is missing; cannot read fallback {}: {error}",
                    fallback.display()
                )
            })?,
        Err(error) => {
            return Err(format!("cannot read schema {}: {error}", schema.display()).into());
        }
    };
    // Both paths compile through the same file
    // This means model.rs is independent of whether the firmware artifacts have been generated
    write_if_changed(output, &content)
        .map_err(|error| format!("cannot write {}: {error}", output.display()))?;
    Ok(())
}

fn write_if_changed(path: &path::Path, content: &str) -> Result<()> {
    // Preserve timestamps when content is identical, avoiding unnecessary
    // source changes and subsequent Cargo rebuilds
    match fs::read(path) {
        Ok(existing) if existing == content.as_bytes() => return Ok(()),
        Ok(_) => {}
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => {}
        Err(error) => return Err(error.into()),
    }
    // Stage beside the destination so replacement stays on the same filesystem
    // A failed write leaves the previous file intact
    // tempfile cleans up staging
    let mut temporary = tempfile::NamedTempFile::new_in(path.parent().ok_or("missing parent")?)?;
    temporary.write_all(content.as_bytes())?;
    // Atomic replacement should preserve the source file's access permissions
    match fs::metadata(path) {
        Ok(metadata) => temporary
            .as_file()
            .set_permissions(metadata.permissions())?,
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => {}
        Err(error) => return Err(error.into()),
    }
    // Replace the destination atomically after all contents have been written
    temporary.persist(path)?;
    Ok(())
}

/// Convert the exported schema without rewriting fields or Serde attributes
pub fn generate(schema: &str) -> Result<String> {
    let schema: serde_json::Value = serde_json::from_str(schema)?;

    // The loader probes this version before deserializing the full document
    // Emit it from the schema instead of maintaining a second version literal
    let version = schema["$defs"]["VersionsExport"]["properties"]["schema_version"]["const"]
        .as_str()
        .ok_or("schema_version must be a string constant")?
        .to_owned();
    
    // These are deserialization models, so builders are unnecessary
    // IndexMap preserves bus and signal orders from the artifact
    let mut settings = typify::TypeSpaceSettings::default();
    settings
        .with_struct_builder(false)
        .with_map_type("::indexmap::IndexMap");
    let mut types = typify::TypeSpace::new(&settings);
    types.add_root_schema(serde_json::from_value(schema)?)?;
    
    // Format Typify's tokens as readable, deterministic source for code review.
    let syntax = syn::parse2::<syn::File>(types.to_stream())?;
    Ok(format!(
        "// Generated from canpiler's Pydantic SuperDBC schema. Do not edit manually.\n\
         // HOWEVER MAKE SURE THIS IS CHECKED IN TO GIT!\n\n\
         pub const SCHEMA_VERSION: &str = {version:?};\n\n{}",
        prettyplease::unparse(&syntax),
    ))
}
