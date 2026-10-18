# DAQ Log Processing and Parquet Export: Ordered Implementation Roadmap

**Status:** Output types and shared `daqcore::log_parse` reorganization implemented. Streaming/parser changes are deferred to a later PR; fixtures remain deferred at the user's request.
**Design:** [Simplified Parquet design](daq-parquet-design.md)

## Scope

Organize common log processing under `daqcore::log_parse`, with `csv_export` and `parquet_export` submodules. Shared modules own the current bulk parser, segmentation, and timestamp correlation for both exporters. This PR reorganizes those implementations without rewriting the parser; streaming follows in a later PR. Keep format-specific table construction and file writing in the respective exporter.

Preserve CSV output behavior, existing CSV defaults, the desktop logger/raw format, and the fixed backwards-tick or `delta > 300_000 ms` segmentation rule. Plot Factory integration, daqcloud ingestion, and production event detectors remain out of scope. SuperDBC stays external to the output directory.

## Sequence and Dependencies

Complete the remaining stages in order. Use small, reviewable changes rather than combining module relocation, pipeline changes, and writer implementation into one change.

| Order | Stage | State/result |
|---|---|---|
| 1 | Define output types and schemas | Implemented under `log_parse::parquet_export`; fixtures deferred. |
| 2 | Consolidate shared log modules and exporters | Implemented: existing parser and CSV behavior unchanged. Current PR scope ends here. |
| 3 | Introduce shared streaming/raw-record reading | Later PR; requires focused parser validation. |
| 4 | Add shared segment/clock analysis for two-pass export | Shared correction and segment descriptors support bounded Parquet export. |
| 5 | Build the Parquet writer | Complete datasets can be exported through `log_parse::parquet_export`. |
| 6 | Add shared event lifecycle | `log_parse::events` emits events for segment-level serialization. |
| 7 | Expose CLI and app selection | Explicit Parquet selection; CSV remains the default. |
| 8 | Verify resource limits and document usage | Export is ready for representative recordings. |

## 1. Output Types and Schemas: Implemented

Existing work: [format module](../daq/daqcore/src/log_parse/parquet_export/mod.rs), [flat schemas](../daq/daqcore/src/log_parse/parquet_export/schema.rs), and [minimal manifest](../daq/daqcore/src/log_parse/parquet_export/manifest.rs).

Keep the signal/frame/event schemas, public writer-size constants, and segment output layout. No feature gate is required. The code is now under `log_parse::parquet_export`; the format was not redesigned during relocation.

Round-trip fixtures remain deferred. Current tests are not a prerequisite for updating this roadmap.

## 2. Shared Log Layout: Implemented in This PR

```text
daqcore/src/log_parse/
  mod.rs
  consts.rs
  parse.rs
  segment.rs
  correlate.rs
  csv_export/
    mod.rs
    table.rs
  parquet_export/
    mod.rs
    schema.rs
    manifest.rs
```

- The existing bulk parser and its `RawFrame`/`ParsedMessage` types live in `log_parse::parse` with no algorithm changes. All PR files follow the design's import style: root-level `crate`/`std` imports, qualified external types, traits imported as `_`, and no `super` or re-exports.
- The existing segmentation function lives in `log_parse::segment`; callers use that path directly.
- Existing timestamp correction lives in `log_parse::correlate`; constants are moved intact into `log_parse::consts`.
- Existing CSV orchestration/table code lives in `log_parse::csv_export` with unchanged function bodies apart from module references.
- Parquet schemas, manifest, paths, and writer constants live in `log_parse::parquet_export`.
- Use `log_parse` as the shared module name to avoid conflicting with the external `log` crate. Remove the `log` module name and top-level `parquet_export` alias. The desktop logger and CAN driver import the same raw-frame type and constants directly from `log_parse`; behavior is unchanged.
- CLI and app CSV calls use `log_parse::csv_export::parse_logs_to_tables`; CSV defaults are unchanged.
- The earlier untracked streaming-reader draft is excluded. No `read.rs`, reader replacement, new parser adapter, or streaming CSV path is introduced in this PR.

**Completion check:** Mechanically compare moved parser, segmentation, correlation, and CSV function bodies against the originals, allowing module-reference changes only. Compile all callers against the new module paths; run format and lint checks. No parser behavior is changed in this structural move; new tests remain deferred.

**Milestone:** Shared log processing and separate exporter modules exist with the original parser/CSV behavior intact. This is the current PR's implementation scope.

## 3. Shared Streaming Reader: Later PR

- Introduce `log_parse::read` and incremental raw-record/source provenance in a separate PR.
- Preserve existing file selection/order and the fixed segmentation/decoding policies.
- Decide how shared reading and decoding are separated without duplicating raw-frame definitions or changing CSV output semantics.
- Retain unknown/invalid complete records for the future Parquet frames table.
- Warn about partial tails without inventing frames or adding manifest fields.
- Connect CSV to a new reader only in that PR, with focused before/after validation. This behavior change is explicitly excluded from the reorganization PR.

**Completion check:** When that later work is requested, verify binary interpretation, multiple files, identity flags, repeated ticks, unknown/invalid records, source positions, stable capture ordinals, partial tails, and unchanged CSV output. Fixture implementation remains deferred for now.

## 4. Add Shared Segment and Clock Analysis

- Use shared reading/decoding to identify segment boundaries on successfully decoded messages, preserving the current strict 300-second comparison.
- Map those boundaries to raw source ranges so retaining invalid or unknown frames does not introduce new segments.
- Provide the segment/correction analysis needed for Parquet's two passes. Retain CSV's collection-based input where needed to preserve the current table writer. This work follows the later streaming PR, not the current reorganization.
- Keep GPS extraction, date validation, single-anchor fallback, fitting, and correction application in `log_parse::correlate`. Support bounded accumulation without introducing a competing timestamp policy.
- Preserve the CSV calculation and rounding behavior. Treat numerical changes as deliberate changes requiring focused comparison, not an incidental cleanup.
- Keep input-change checks internal to processing; do not add checksum metadata to the manifest.

**Completion check:** Both exporters use shared segmentation and correlation logic. Compare segment boundaries and timestamps with the existing CSV behavior on zero/one/many-anchor recordings, repeated ticks, resets, and gaps below/exactly at/above 300 seconds when validation resumes.

## 5. Build the Parquet Writer

- Add the `log_parse::parquet_export` orchestration and writer using shared stages for analysis and replay.
- Write one raw frame row per complete record and one signal row per decoded signal.
- Rotate only signal files, between complete messages. Keep frames/events writers open for the segment.
- Bound batches and row groups with the existing public constants; keep the manifest minimal.
- Initially produce empty typed event files; stage 6 supplies event detection outputs.
- Verify inputs are unchanged between passes. Finalize Parquet files before publishing the manifest.

**Completion check:** Output uses `segment-NNN/signals/signals-NNN.parquet` with segment-level `frames.parquet` and `events.parquet`. Message grouping, nullable UTC, raw provenance, chunk-size independence, and interrupted-output handling match the design. No CSV writer or consumer integration is required for Parquet writing.

## 6. Add the Shared Event Lifecycle

- Introduce `log_parse::events` for ordinary Rust event values and detector state.
- Keep event detection independent of storage and reusable for future live messages.
- Carry state across source files and signal chunks; explicitly handle segment boundaries and end of input.
- Connect emitted events to the Parquet event writer. Keep the current four-column event schema; unfinished-interval handling needs an explicit policy.
- Do not change CSV output to add event columns/files in this work.
- Defer production SDC, watchdog, and lap rules.

**Completion check:** Detector code has no Arrow/Parquet dependency. Validate instant events, cross-chunk intervals, empty event files, unfinished intervals, and event time basis when tests resume.

## 7. Expose CLI and App Selection

- Add explicit CSV/Parquet selection while preserving CSV defaults and current CLI invocations.
- Call `log_parse::csv_export` or `log_parse::parquet_export` from the existing application worker.
- Use the new entry points directly; do not introduce old-path aliases or forwarding exports.
- Document output location and external SuperDBC requirements for raw re-decoding.

**Completion check:** Both selections reach their own writers through common log processing. Preserve existing CSV outputs and report useful errors without changing logger or consumer applications.

## 8. Verify Resource Limits and Finish Documentation

- Run recordings larger than the memory budget through Parquet export and batch reading.
- Measure all writer buffers, segment summaries, event state, and footer metadata; tune the size constants if needed.
- Confirm signal rotation does not alter segment frame/event output or decoded values.
- Run relevant Rust build, format, and clippy checks. Fixture tests remain deferred until requested; focused parser/CSV parity validation belongs to the later streaming work.
- Document usage and remaining limitations.

## Deferred Follow-ups

- Production SDC/fault, watchdog, and lap detectors.
- Plot Factory integration and daqcloud ingestion/upload protocols.
- Changes to GPS validation, the 300-second segmentation policy, desktop logging behavior, or CSV output semantics.
