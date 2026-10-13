# SuperDBC wire models

CANpiler's Pydantic export models define the SuperDBC format. The existing
`generators/generate.py` workflow exports `dbc/superdbc.schema.json` alongside
the CAN artifacts.

When that schema exists, `daqcore/build.rs` uses Typify to generate Rust wire
models and refreshes `src/superdbc/model_generated.rs` if its contents changed.
It writes a copy into Cargo's `OUT_DIR` for compilation. When the schema is
absent, it compiles the checked-in generated models instead. Only a missing
schema activates fallback; an unreadable or invalid schema fails the build.

After changing the Pydantic SuperDBC export models, run from the repository root:

```sh
python3 generators/generate.py
cargo build --manifest-path daq/Cargo.toml -p daqcore --locked
```

Commit the refreshed Rust models with the Pydantic changes. `dbc/` remains
ignored by Git, so a fresh checkout builds without running Python or firmware
generators. CI runs the existing generators and verifies that building leaves
the committed Rust models unchanged.

Cargo reruns the build script when the schema or fallback changes, including
when the schema appears or disappears. Unchanged generated files are not
rewritten. Schema-present builds need write access to the checked-in fallback
when it is stale.

The generated types are private deserialization models. Rust treats missing
nullable metadata (`limits`, `choices`, `nominal_period_ms`, and `display_format`)
the same as explicit null. These defaults are declared in the Pydantic models;
Rust generates directly from their exported schema without modifying it.

Maps preserve bus order and use ordinary Serde deserialization, including
replacing earlier values when a JSON map key repeats. Canpiler validates bus
IDs, message definitions, and choices before exporting its dictionary-based
JSON. Runtime database types, CAN codecs, and semantic validation remain
handwritten.
