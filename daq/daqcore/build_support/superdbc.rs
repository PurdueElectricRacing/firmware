use std::{error::Error, fs, io::Write, path::Path};

type Result<T> = std::result::Result<T, Box<dyn Error>>;

pub fn build(schema: &Path, fallback: &Path, output: &Path) -> Result<()> {
    let content = match fs::read_to_string(schema) {
        Ok(text) => {
            let content = generate(&text)
                .map_err(|error| format!("cannot generate from {}: {error}", schema.display()))?;
            write_if_changed(fallback, &content)
                .map_err(|error| format!("cannot refresh {}: {error}", fallback.display()))?;
            content
        }
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
    write_if_changed(output, &content)
        .map_err(|error| format!("cannot write {}: {error}", output.display()))?;
    Ok(())
}

fn write_if_changed(path: &Path, content: &str) -> Result<()> {
    match fs::read(path) {
        Ok(existing) if existing == content.as_bytes() => return Ok(()),
        Ok(_) => {}
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => {}
        Err(error) => return Err(error.into()),
    }
    let mut temporary = tempfile::NamedTempFile::new_in(path.parent().ok_or("missing parent")?)?;
    temporary.write_all(content.as_bytes())?;
    // Atomic replacement should preserve the source file's access permissions.
    match fs::metadata(path) {
        Ok(metadata) => temporary
            .as_file()
            .set_permissions(metadata.permissions())?,
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => {}
        Err(error) => return Err(error.into()),
    }
    temporary.persist(path)?;
    Ok(())
}

pub fn generate(schema: &str) -> Result<String> {
    let schema: serde_json::Value = serde_json::from_str(schema)?;
    let version = schema["$defs"]["VersionsExport"]["properties"]["schema_version"]["const"]
        .as_str()
        .ok_or("schema_version must be a string constant")?
        .to_owned();
    let mut settings = typify::TypeSpaceSettings::default();
    settings
        .with_struct_builder(false)
        .with_map_type("::indexmap::IndexMap");
    let mut types = typify::TypeSpace::new(&settings);
    types.add_root_schema(serde_json::from_value(schema)?)?;
    let syntax = syn::parse2::<syn::File>(types.to_stream())?;
    Ok(format!(
        "// Generated from canpiler's Pydantic SuperDBC schema. Do not edit.\n\
         // Refresh: python3 generators/generate.py, then cargo build --manifest-path daq/Cargo.toml -p daqcore.\n\n\
         pub const SCHEMA_VERSION: &str = {version:?};\n\n{}",
        prettyplease::unparse(&syntax),
    ))
}
