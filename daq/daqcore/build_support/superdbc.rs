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
    let mut schema: serde_json::Value = serde_json::from_str(schema)?;
    let version = schema["$defs"]["VersionsExport"]["properties"]["schema_version"]["const"]
        .as_str()
        .ok_or("schema_version must be a string constant")?
        .to_owned();
    // Typify does not infer a string-valued map from patternProperties alone.
    // Canonical integer choice keys remain validated by the runtime compiler.
    let choices = schema
        .pointer_mut("/$defs/SignalExport/properties/choices/anyOf/0")
        .ok_or("unsupported choices schema: expected an object/null union")?;
    if choices["type"] != "object"
        || choices["patternProperties"]
            != serde_json::json!({
                "^(0|-?[1-9][0-9]*)$": {"type": "string"}
            })
    {
        return Err(
            "unsupported choices schema: expected canonical integer keys and string values".into(),
        );
    }
    choices["additionalProperties"] = serde_json::json!({"type": "string"});
    choices
        .as_object_mut()
        .ok_or("choices must be an object")?
        .remove("patternProperties");

    let mut settings = typify::TypeSpaceSettings::default();
    settings
        .with_struct_builder(false)
        .with_map_type("::indexmap::IndexMap");
    let mut types = typify::TypeSpace::new(&settings);
    types.add_root_schema(serde_json::from_value(schema.clone())?)?;
    let mut syntax = syn::parse2::<syn::File>(types.to_stream())?;
    for item in &mut syntax.items {
        let syn::Item::Struct(item) = item else {
            continue;
        };
        let name = item.ident.to_string();
        let definition = if name == schema["title"].as_str().unwrap_or_default() {
            &schema
        } else {
            &schema["$defs"][&name]
        };
        for field in &mut item.fields {
            let Some(name) = field.ident.as_ref().map(ToString::to_string) else {
                continue;
            };
            let property = &definition["properties"][&name];
            let required = definition["required"]
                .as_array()
                .is_some_and(|fields| fields.iter().any(|field| field.as_str() == Some(&name)));
            let nullable = property["anyOf"]
                .as_array()
                .is_some_and(|variants| variants.iter().any(|variant| variant["type"] == "null"));
            if required && nullable {
                // Serde otherwise treats any Option<T> as optional, even without
                // #[serde(default)]. Keep null valid but make omission an error.
                for attribute in &mut field.attrs {
                    if attribute.path().is_ident("serde") {
                        let mut arguments = attribute.parse_args_with(
                            syn::punctuated::Punctuated::<syn::Meta, syn::Token![,]>::parse_terminated,
                        )?;
                        arguments = arguments
                            .into_iter()
                            .filter(|arg| {
                                !arg.path().is_ident("default")
                                    && !arg.path().is_ident("deserialize_with")
                            })
                            .collect();
                        *attribute = syn::parse_quote!(#[serde(#arguments)]);
                    }
                }
                field.attrs.push(if name == "choices" {
                    syn::parse_quote!(#[serde(deserialize_with = "nullable_unique_map")])
                } else {
                    syn::parse_quote!(#[serde(deserialize_with = "required_nullable")])
                });
                field.attrs.retain(|attribute| {
                    !matches!(&attribute.meta,
                    syn::Meta::List(list) if list.path.is_ident("serde") && list.tokens.is_empty())
                });
            } else if name == "buses" && definition == &schema {
                field
                    .attrs
                    .push(syn::parse_quote!(#[serde(deserialize_with = "unique_map")]));
            }
        }
    }
    Ok(format!(
        "// Generated from canpiler's Pydantic SuperDBC schema. Do not edit.\n\
         // Refresh: python3 generators/generate.py, then cargo build --manifest-path daq/Cargo.toml -p daqcore.\n\n\
         pub const SCHEMA_VERSION: &str = {version:?};\n\n{}",
        prettyplease::unparse(&syntax),
    ))
}
