#[path = "build_support/superdbc.rs"]
mod superdbc;

fn main() {
    // Resolve paths from this crate rather than the caller's working directory
    let manifest = std::path::PathBuf::from(std::env::var_os("CARGO_MANIFEST_DIR").unwrap());

    // CANpiler writes its schema into the repository's ignored dbc directory
    let schema = manifest.join("../../dbc/superdbc.schema.json");

    // The checked-in fallback is used when the schema is missing
    // The fallback is also always refreshed when the schema is present
    let fallback = manifest.join("src/superdbc/model_generated.rs");

    // model.rs always includes this OUT_DIR file, regardless of whether it was
    // generated from the schema or copied from the checked-in fallback
    let output =
        std::path::PathBuf::from(std::env::var_os("OUT_DIR").unwrap()).join("superdbc_models.rs");

    // Let Cargo rerun generation when its inputs or implementation change.
    println!("cargo::rerun-if-changed=build.rs");
    println!("cargo::rerun-if-changed=build_support");
    println!("cargo::rerun-if-changed={}", schema.display());
    println!("cargo::rerun-if-changed={}", fallback.display());
    // A fresh checkout may have neither the schema nor the dbc directory
    // Watch the nearest existing directory so Cargo notices them appearing
    let mut directory = schema.parent().unwrap();
    while !directory.exists() {
        directory = directory.parent().unwrap();
    }
    println!("cargo::rerun-if-changed={}", directory.display());

    // Only a missing schema permits fallback
    superdbc::build(&schema, &fallback, &output)
        .unwrap_or_else(|error| panic!("SuperDBC model generation failed: {error}"));
}
