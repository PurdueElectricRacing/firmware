# DAQ Log Processing and Parquet Export: Ordered Implementation Roadmap

**Status:** Output types, shared layout, and bulk Parquet export implemented. CLI/app selection is next. The desired event lifecycle, serialization, and detection are deferred to a follow-up PR; this PR keeps typed empty event files. Streaming remains a later PR; permanent fixtures remain deferred.
**Design:** [Simplified Parquet design](daq-parquet-design.md)

## Scope

Organize common log processing under `daqcore::log_parse`, with `csv_export` and `parquet_export` submodules. Shared modules own the current bulk parser, segmentation, and timestamp correlation for both exporters. The Parquet exporter uses that same bulk parser with optional raw-frame capture. Small supporting refactors preserve decoding/CSV behavior; streaming follows in a later PR. Keep format-specific table construction and file writing in the respective exporter.

Preserve CSV output behavior, existing CSV defaults, the desktop logger/raw format, and the fixed backwards-tick or `delta > 300_000 ms` segmentation rule. Plot Factory integration, daqcloud ingestion, and production event detectors remain out of scope. SuperDBC stays external to the output directory.

## Sequence and Dependencies

Complete the remaining stages in order. Use small, reviewable changes rather than combining module relocation, pipeline changes, and writer implementation into one change.

| Order | Stage | State/result |
|---|---|---|
| 1 | Define output types and schemas | Implemented. |
| 2 | Consolidate shared log modules and exporters | Implemented under `log_parse`. |
| 3 | Build Parquet export using the existing bulk parser | Implemented; raw-frame capture is optional and uses the same decoding path. |
| 4 | Expose CLI and app selection | Next: explicit Parquet selection; CSV remains the default. |
| 5 | Verify resource limits and document usage | Tune writer buffers; full-recording memory remains bulk until streaming work. |
| 6 | Add shared event lifecycle | Follow-up PR: retain the desired design; this PR writes typed empty event files. |
| 7 | Introduce shared streaming reading | Later PR, with focused parser/CSV validation. |
| 8 | Add bounded two-pass clock analysis | Later PR after streaming; not a prerequisite for the bulk exporter. |

## 1. Output Types and Schemas: Implemented

Existing work: [format module](../daq/daqcore/src/log_parse/parquet_export/mod.rs), [flat schemas](../daq/daqcore/src/log_parse/parquet_export/schema.rs), and [minimal manifest](../daq/daqcore/src/log_parse/parquet_export/manifest.rs).

Keep the signal/frame/event schemas, public writer-size constants, and segment output layout. No feature gate is required. The code is now under `log_parse::parquet_export`; the format was not redesigned during relocation.

Permanent round-trip fixtures remain deferred. Temporary on-disk validation is recorded in stage 3.

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
    writer.rs
```

- The bulk parser and its `RawFrame`/`ParsedMessage` types live in `log_parse::parse`; reading/decoding behavior is retained and optional raw capture supports Parquet. All PR files follow the design's import style: root-level `crate`/`std` imports, qualified external types, traits imported as `_`, and no `super` or re-exports.
- The existing segmentation function lives in `log_parse::segment`; callers use that path directly.
- Existing timestamp correction lives in `log_parse::correlate`; constants are moved intact into `log_parse::consts`.
- Existing CSV orchestration/table code lives in `log_parse::csv_export` with unchanged function bodies apart from module references.
- Parquet schemas, manifest, paths, and writer constants live in `log_parse::parquet_export`.
- Use `log_parse` as the shared module name to avoid conflicting with the external `log` crate. Remove the `log` module name and top-level `parquet_export` alias. The desktop logger and CAN driver import the same raw-frame type and constants directly from `log_parse`; behavior is unchanged.
- CLI and app CSV calls use `log_parse::csv_export::parse_logs_to_tables`; CSV defaults are unchanged.
- The earlier streaming-reader draft remains excluded. No `read.rs`, reader replacement, or streaming CSV path is introduced; raw-frame capture is optional in the shared bulk parser.

**Completion check:** Mechanically compare moved parser, segmentation, correlation, and CSV function bodies against the originals, allowing module-reference changes only. Compile all callers against the new module paths; run format and lint checks. No parser behavior is changed in this structural move; new tests remain deferred.

**Milestone:** Shared log processing and separate exporter modules exist with the original parser/CSV behavior intact. The following exporter work retains bulk parsing and allows small supporting refactors while preserving CSV behavior.

## 3. Bulk Parquet Export: Implemented

- [Shared parser](../daq/daqcore/src/log_parse/parse.rs) retains the original bulk reading, padding/skip behavior, file order, identity handling, and decoding. `parse_logs_with_frames` opts into raw capture; CSV's `parse_log_files` does not retain frames.
- Each complete record receives a capture ordinal before decoding. Captured frames include source index, source-record index, raw bytes/identity/ticks, and decode status. Parsed messages carry the matching ordinal.
- The exporter uses the existing segment/correlation functions. Segment boundaries depend only on decoded messages. Raw records preceding a decoded boundary remain in the preceding segment; leading/trailing raw-only records are preserved.
- [Parquet export](../daq/daqcore/src/log_parse/parquet_export/mod.rs) exposes `parse_logs_to_parquet`, returning a manifest. The output dataset directory must not exist.
- [Writer](../daq/daqcore/src/log_parse/parquet_export/writer.rs) batches rows, rotates only signal files between complete messages, and writes segment-level frames and typed empty events.
- The manifest records correction coefficients, time basis, inclusive segment/chunk bounds, source paths, and existing SuperDBC identity metadata. SuperDBC is not copied.
- Files are finalized through temporary names and the manifest is published last. Incomplete output can remain after an error and is cleared on retry if its manifest is missing or malformed. A directory with a deserializable manifest is never overwritten; manifest read errors and cleanup that would remove input logs are rejected.
- Empty inputs and raw-only recordings are supported. Overall input memory is still bulk; bounded writer batches do not make parsing incremental.

**Validation:** Check a generated recording before/after CSV export, read all Parquet tables in small batches, verify raw bytes and provenance, retain repeated timestamp updates, cover UTC and unanchored segments, and confirm typed empty event output and overwrite rejection. These checks use temporary files outside the repository; no permanent tests are added for now.

**Milestone:** Offline Parquet export works through the daqcore API without a streaming parser, CLI/app selection, or consumer integration.

## 4. Expose CLI and App Selection

- Add explicit CSV/Parquet selection while preserving CSV defaults and current CLI invocations.
- Call `log_parse::csv_export` or `log_parse::parquet_export` from the existing application worker.
- Use the new entry points directly; do not introduce old-path aliases or forwarding exports.
- Document output location and external SuperDBC requirements for raw re-decoding.

**Completion check:** Both selections reach their own writers through common log processing. Preserve existing CSV outputs and report useful errors without changing logger or consumer applications.

## 5. Verify Resource Limits and Finish Documentation

- Measure writer buffer use and batch reading with representative recordings. Do not claim whole-recording bounded memory while the bulk parser remains in use.
- Measure all writer buffers, segment summaries, and footer metadata; tune the size constants if needed.
- Confirm signal rotation does not alter segment frame/event output or decoded values.
- Run relevant Rust build, format, and clippy checks. Fixture tests remain deferred until requested; focused parser/CSV parity validation belongs to the later streaming work.
- Document usage and remaining limitations.

## 6. Shared Event Lifecycle: Follow-up PR

The event system below remains desired, but none of its lifecycle, detector plumbing, or populated-event serialization is required for this PR. Keep the existing event schema and typed empty `events.parquet` per segment.

- Introduce `log_parse::events` for ordinary Rust event values and detector state.
- Keep event detection independent of storage and reusable for future live messages.
- Carry state across source files and signal chunks; explicitly handle segment boundaries and end of input.
- Connect emitted events to the Parquet event writer. Keep the current four-column event schema; unfinished-interval handling needs an explicit policy.
- Do not change CSV output to add event columns/files in this work.
- Defer production SDC, watchdog, and lap rules.

**Completion check:** Detector code has no Arrow/Parquet dependency. Validate instant events, cross-chunk intervals, empty event files, unfinished intervals, and event time basis when tests resume.

## 7. Shared Streaming Reading: Later PR

Introduce `log_parse::read` and incremental consumption in a separate PR. Preserve current ordering, binary interpretation, source positions, ordinals, partial-tail handling, decoding, and CSV output. The current bulk exporter is the reference implementation; do not add a second decoding policy. Full-recording memory acceptance work belongs here.

## 8. Bounded Clock Analysis: Later PR

Use shared reading/decoding in two passes to finalize correction models before writing rows. Preserve existing GPS extraction, date checks, single-anchor fallback, and fit/rounding behavior. This is an optimization of the working bulk exporter, not a prerequisite for Parquet output. Keep source-change checks internal rather than expanding the manifest.

## Deferred Follow-ups

- Shared event lifecycle and populated-event serialization, followed by production SDC/fault, watchdog, and lap detectors.
- Plot Factory integration and daqcloud ingestion/upload protocols.
- Changes to GPS validation, the 300-second segmentation policy, desktop logging behavior, or CSV output semantics.
