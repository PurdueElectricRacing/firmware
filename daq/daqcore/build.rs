#[path = "build_support/superdbc.rs"]
mod superdbc;

fn main() {
    let manifest = std::path::PathBuf::from(std::env::var_os("CARGO_MANIFEST_DIR").unwrap());
    let schema = manifest.join("../../dbc/superdbc.schema.json");
    let fallback = manifest.join("src/superdbc/model_generated.rs");
    let output =
        std::path::PathBuf::from(std::env::var_os("OUT_DIR").unwrap()).join("superdbc_models.rs");

    println!("cargo::rerun-if-changed=build.rs");
    println!("cargo::rerun-if-changed=build_support");
    println!("cargo::rerun-if-changed={}", schema.display());
    println!("cargo::rerun-if-changed={}", fallback.display());
    // Watch the directory as well: Cargo must notice an absent schema appearing.
    let mut directory = schema.parent().unwrap();
    while !directory.exists() {
        directory = directory.parent().unwrap();
    }
    println!("cargo::rerun-if-changed={}", directory.display());

    superdbc::build(&schema, &fallback, &output)
        .unwrap_or_else(|error| panic!("SuperDBC model generation failed: {error}"));
}
