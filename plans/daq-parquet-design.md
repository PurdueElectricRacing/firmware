# DAQ Parquet Intermediate Format: Simplified Design Plan

**Status:** Shared `log_parse` module reorganization implemented. The existing bulk parser remains unchanged; streaming reading and bounded analysis are deferred to a later PR.
**Date:** 2026-10-16  
**Requirements:** [DAQ Parquet Format — Design Goals](parquet.md)

## 1. Purpose and Decisions

Add a simple Parquet export for DAQ `.log` files. All parsing, timestamp correction, and event detection run offline. Plot Factory and daqcloud are future consumers only; their integration, application changes, and ingestion design are outside this plan.

The design uses three flat tables:

- `signals-NNN.parquet`: chunked signal updates, with readable message and signal metadata.
- `frames.parquet`: one file per segment containing captured messages, raw bytes, and source locations for debugging and reprocessing.
- `events.parquet`: one file per segment containing event names, colors, and time ranges.

A small `manifest.json` describes the source files, segments, chunks, timestamp correction, and versions. SuperDBC is referenced in that manifest; it is not copied or moved into the output directory.

### Scope constraints

- Leave the desktop logger, its rotation behavior, and the raw logging format unchanged.
- Preserve the existing fixed segmentation rule: backwards ticks or a forward jump strictly greater than `300_000 ms` (300 seconds). Do not add a configurable threshold or a 5-second rule.
- Preserve CSV behavior, including file ordering, segmentation, timestamp correction, headers, binning, filenames, and enum handling. CSV and Parquet use shared log-processing modules with separate output writers.
- Organize common functionality under `daqcore::log_parse` as a clean refactor. Update all callers directly; do not retain old module aliases or compatibility exports. This PR only moves code and updates module references; preserve parser, correlation, segmentation, and CSV writer bodies.
- Do not rewrite or replace the `.log` parser in this PR. Keep bulk reads, current decoding/filtering, padding handling, and sorting as implemented. Streaming reading and two-pass analysis belong to a later PR.
- Exclude Plot Factory integration, plotting/derived-signal changes, daqcloud ingestion, database migrations, upload protocols, and retry/checkpoint design.

These constraints supersede the broader proposals in the original requirements where they conflict. The flat schemas and segment-level frame/event layout remain the intended Parquet design.

## 2. Existing Code and Integration Points

| Area | Current behavior | Planned change |
|---|---|---|
| [Pipeline](../daq/daqcore/src/log_parse/csv_export/mod.rs) | Parse logs, split chunks, correlate time, export CSV. | Reorganized under `log_parse`; CSV entry point retains the same calls and writer behavior. |
| [Binary parser](../daq/daqcore/src/log_parse/parse.rs) | Reads complete files and accumulates decoded messages; drops undecodable frames. | Relocated without changing its implementation. Streaming and raw-record retention are future work. |
| [Raw frame](../firmware/common/timestamped_frame/timestamped_frame.h) | 16 bytes: ticks, packed identity, eight payload bytes. | Preserve those original values in the frames table. |
| [Desktop logger](../daq/daqcore/src/can/logger.rs) | Rotates `.log` files approximately every 60 seconds. | No changes. Consume its existing output. |
| [Segmentation](../daq/daqcore/src/log_parse/parse.rs) | Splits on backwards ticks or forward jumps greater than 300 seconds. | Preserve the same fixed rule and boundary decisions; split only Parquet signal storage further into size-bounded chunks. |
| [Timestamp correction](../daq/daqcore/src/log_parse/correlate.rs) | Fits an affine mapping using `gps_time`. | Move into shared `log_parse::correlate`; preserve CSV timestamp results and validation. |
| [CSV export](../daq/daqcore/src/log_parse/csv_export/table.rs) | Wide table on a 1 ms grid with seven metadata rows. | Move the existing table writer into `log_parse::csv_export` without changing output behavior. |
| [Plot Factory](../plot_factory/factory/loading.py) | Eagerly loads and concatenates wide CSVs. | Out of scope; leave its loaders, plots, and tests unchanged. |
| [daqcloud](../daq/daqcloud/src/main.rs) | Separate backend application. | Out of scope; no changes or ingestion planning. |

Reuse SuperDBC message identity and decoding. The canonical message definition key is `(bus_id, can_id, is_extended)`; signal name identifies a signal within that message.

### 2.1 Target daqcore Organization

```text
daqcore/src/
  log_parse/
    mod.rs
    consts.rs
    parse.rs
    read.rs          # later streaming PR
    segment.rs
    correlate.rs
    events.rs
    csv_export/
      mod.rs
      table.rs
    parquet_export/
      mod.rs
      schema.rs
      manifest.rs
      writer.rs
```

This shows the current shared/exporter organization plus future modules. `read.rs` is deferred to the later streaming PR; bulk file reading remains in the unchanged `parse.rs`. `events.rs` and the Parquet writer are introduced in their later implementation stages. Start with files for shared stages; turn a module into a directory only if its size warrants it.

| Shared module | Responsibility |
|---|---|
| `log_parse::read` (later PR) | Future streaming reading, source positions, and capture ordinals. Not introduced or wired in this PR. |
| `log_parse::parse` | Current bulk reading, raw frame/parsed-message types, identity interpretation, and SuperDBC decoding, relocated without algorithm changes. Raw-only record preservation is deferred. |
| `log_parse::segment` | The existing decoded-message discontinuity rule and mapping boundaries back to source records. This is separate from Parquet file chunking. |
| `log_parse::correlate` | GPS extraction/validation, affine fitting, and correction application shared by both exports. |
| `log_parse::events` | Storage-independent event values and detector lifecycle, added later. |
| `log_parse::consts` | Shared binary transport flags and the existing fixed 300-second jump threshold. |

`log_parse::csv_export` owns the existing CSV entry point, table header construction, 1 ms binning, enum formatting, and CSV filenames. `log_parse::parquet_export` owns Arrow schemas, the manifest, writer-size constants, dataset paths, Parquet chunking, and file finalization. CSV-only constants such as bin width can stay with the CSV exporter when moved.

Keep shared types as ordinary Rust values; shared reading, decoding, segmentation, and correction must not depend on Arrow or exporter-specific manifest types. The manifest can serialize or reference a shared correction model. Avoid introducing a generic writer framework or traits just to support two exporters.

### 2.2 Migration Strategy

The current reorganization moves the existing parser into `log_parse::parse`, the existing segmentation function into `log_parse::segment`, correlation into `log_parse::correlate`, CSV table generation/orchestration into `log_parse::csv_export`, and the existing Parquet output types into `log_parse::parquet_export`. Bodies remain unchanged apart from module references. Call segmentation through `log_parse::segment::chunk_parsed`.

The shared module is named `log_parse` to avoid a collision with the external `log` crate. There is no `log` module, top-level `parquet_export` alias, or `parse::chunk_parsed` forwarding export. CLI and app CSV calls use `log_parse::csv_export::parse_logs_to_tables`. The desktop logger and CAN driver import the same raw-frame type and constants directly from `log_parse`; their behavior is unchanged.

The earlier untracked streaming-reader draft is excluded from this PR. Do not add `log_parse::read`, change `fs::read` to a buffered iterator, replace decoding/error handling, or route CSV through a new parser. A later PR will introduce streaming and raw-record provenance in the shared pipeline. The present change establishes shared ownership of the current parser and correction code; it does not implement Parquet writing.

Mechanical comparison of moved function bodies plus compile/lint/format checks verifies this reorganization without reopening parser validation. If a later change alters parser behavior, validate it in that separate PR. Tests remain deferred at the user's request.

### 2.3 Import Style for This PR

- Use `crate` paths, never `super`, and import only the first level under `crate`.
- Import only the first level under `std`, except for `sync::mpsc`.
- Combine imports from the same root into one `use` declaration.
- Do not import external crate items directly; qualify their names at use sites. Import traits only as `use chrono::Datelike as _;` when needed for method resolution.
- Do not introduce re-exports or compatibility aliases.

```rust
use crate::{frame, log_parse};
use chrono::Datelike as _;
use chrono::Timelike as _;
use std::{fs, io, path, time};
```

## 3. Segments, Chunks, and Output Layout

A **segment** is a continuous DAQ clock region. A **chunk** is one size-bounded signal file in the segment's `signals/` subfolder. Frames and events each have one segment-level file, written and read incrementally using Parquet row groups. Source-file boundaries do not automatically start a segment or chunk.

```text
recording/
  manifest.json
  segment-000/
    signals/
      signals-000.parquet
      signals-001.parquet
    frames.parquet
    events.parquet
  segment-001/
    signals/
      signals-000.parquet
    frames.parquet
    events.parquet
```

Each signal chunk contains all signal rows for its messages; never split the signals of one message across chunks. The segment-level frames table links to those rows through `message_seq`. Events can span any number of signal chunks without special file-ownership rules.

The manifest lists chunks in capture order. A small segment normally contains only one chunk, similar to one existing CSV output. There is no dataset-wide `events.parquet` and no SuperDBC copy in the output.

## 4. Signal and Frame Tables

### 4.1 Signals

One row represents one decoded signal update. Types below use Arrow notation; `?` means nullable.

| Column | Type | Meaning |
|---|---|---|
| `message_seq` | `UInt64` | Dataset-local message ordinal shared by all signals from one captured frame. |
| `daq_ticks_ms` | `UInt32` | Original DAQ reception ticks. |
| `timestamp_utc_ms` | `Int64?` | Corrected Unix milliseconds in UTC; null when unavailable. |
| `bus_id` | `UInt8` | Bus identifier. |
| `bus_name` | `Utf8` | Readable bus name, also used by Plot Factory. |
| `can_id` | `UInt32` | CAN identifier. |
| `is_extended` | `Boolean` | Standard/extended identifier distinction. |
| `node_name` | `Utf8` | Transmitting node from the definition. |
| `message_name` | `Utf8` | Message name. |
| `message_desc` | `Utf8` | Message description. |
| `signal_name` | `Utf8` | Signal name. |
| `signal_desc` | `Utf8` | Signal description. |
| `signal_unit` | `Utf8` | Unit, or an empty string if unspecified. |
| `value` | `Float64` | Decoded physical value. |

Descriptions and names are deliberately repeated. Parquet compression and dictionary encoding handle repeated metadata, and a signal file remains usable for plotting without loading SuperDBC or joining definition tables.

`message_seq` is assigned in capture order before decoding and is unique across all source files in one export. It is not a backend database ID. It distinguishes messages with identical ticks and groups their signal rows without relying on backend IDs. Signal rows follow message order and then definition order.

Store physical values consistently, including enumerated signals. An enum label column can be added later if consumers need it. Exact raw integer values and float bit patterns remain recoverable from the companion frame and the matching external definition artifact; do not claim that `Float64` preserves every raw integer exactly.

### 4.2 Frames

The companion table keeps debugging information out of the main analysis table while preserving messages that have no decoded signals.

| Column | Type | Meaning |
|---|---|---|
| `message_seq` | `UInt64` | Joins a captured frame to its signal rows. |
| `source_file` | `Utf8` | Original source path listed in the manifest. |
| `source_record` | `UInt64` | Zero-based 16-byte record index in that source. |
| `daq_ticks_ms` | `UInt32` | Original DAQ ticks. |
| `raw_identity` | `UInt32` | Original packed identity, including transport flags. |
| `payload` | `Binary` | Original eight payload bytes. |
| `decode_status` | `Utf8` | `ok`, `invalid_identity`, `unknown_bus`, `unknown_message`, or `decode_error`. |

Every complete input record receives a frame row. Unknown or invalid messages produce no signal rows. A decoded message with no signals still has a frame row.

The raw source format does not contain actual DLC, RTR status, or CAN FD payloads. Do not invent those values. The planned Parquet reader will warn and skip a trailing partial record. The current parser's existing padding-and-skip implementation remains unchanged in this PR. Keep original logs for inspection; the manifest does not store partial bytes.

Use original source paths as labels so files sharing a basename remain distinguishable. List those paths in the manifest for traceability. Byte offsets are `source_record * 16` for the current format.

## 5. Events per Segment

Each segment has one flat event table:

| Column | Type | Meaning |
|---|---|---|
| `name` | `Utf8` | Event name, such as an SDC trip or completed lap. |
| `color` | `Utf8` | Display color in `#RRGGBB` form. |
| `start_time_ms` | `Int64` | Event start in the segment's declared event time basis. |
| `end_time_ms` | `Int64?` | End time for an interval; null for an instantaneous event. |

For segments with accepted UTC correction, event times are Unix milliseconds in UTC. Otherwise, they are raw DAQ milliseconds within that segment. Record `time_basis` as `utc_ms` or `daq_ticks_ms` in the segment metadata. Never infer the basis from a timestamp's magnitude or mix bases within a segment.

Event detection remains independent of Parquet. `log_parse::events` receives shared decoded-message values, retains detector state, and emits ordinary Rust event values. A writer streams those values into the current segment's event file. The same detector logic can later receive adapted live messages.

### 5.1 Intervals across boundaries

- An instantaneous event is appended to the segment event file when detected.
- An interval is appended once when it ends or is finalized. Its start and end can fall in different signal chunks.
- Detector state continues across source-file and chunk boundaries. Chunk rotation does not split an event or reset detection.
- Handling unfinished intervals at a segment boundary or end of input is deferred to the detector stage. The four-column event schema has no incomplete marker. Do not assert continuity across a DAQ clock reset.
- Write an empty typed event file when a segment has no emitted events.

Readers scan the selected segment's event file in batches and select intervals overlapping the requested time window. Intervals are stored in completion order, so readers must not assume the file is sorted by start time. No event-to-chunk ownership or event-indexing system is needed.

Actual SDC, watchdog, and lap rules are follow-up work. Existing fault-event and fault-sync messages provide inputs for fault detectors. Watchdog evidence and lap geometry/debounce rules still need specification; a gap alone does not prove a watchdog reset.

## 6. Small Manifest and External SuperDBC Reference

Keep one JSON manifest with only the core inventory:

| Field | Contents |
|---|---|
| `format_version` | Format version, initially `1.0`. |
| `parser_version` | Parser version string. |
| `superdbc` | Existing schema version, content hash, and git hash metadata identifying the external definition artifact. |
| `source_files` | Original source paths in processing order, also used as unique frame source labels. |
| `segments` | Each segment's directory, optional slope/intercept correction, `time_basis`, inclusive time bounds, and signal chunks. Each chunk lists its path and inclusive time bounds in the segment's time basis. |

All output paths are dataset-relative. The segment directory implies `frames.parquet` and `events.parquet`; there is no need to list them separately. Signal chunks are ordered path/start/end records. All time bounds are inclusive minimum/maximum timestamps; chunk bounds inherit the segment's time basis. Use UTC when a correction exists and DAQ ticks otherwise.

Do not copy or move SuperDBC into the export or compute an additional SHA-256 identity. Its existing hash metadata is a reference, not an independently verified checksum. Omit diagnostic summaries, source sizes/checksums, row counts, additional time summaries, detector-version lists, and processing-settings structures. The fixed segmentation rule and millisecond units are defined by the code and schema. More metadata can be added when a concrete consumer needs it.

Normal plotting uses the names, descriptions, and units already in the signal table and does not require access to the referenced file. Re-decoding raw frames or inspecting complete bit layouts requires supplying a matching external SuperDBC artifact. The existing SuperDBC hashes identify the definition artifact; its original path is not required in the manifest.

Use one compatible SuperDBC artifact per export initially. Multiple definition versions and independent recorder clocks are deferred.

Reject unsupported format major versions and incompatible column types. Additive fields can use a minor version when existing semantics stay unchanged. Further compatibility machinery is unnecessary for the initial implementation.

## 7. Parsing, Correction, and Bounded Writing

### 7.1 Segmentation

Use the current directory selection and lexicographic source-file ordering; record that order in the manifest. Do not introduce natural filename sorting as part of this change. Continue a segment across normal log rotation as the existing parser does.

Use the existing condition from `chunk_parsed`: start a new segment when ticks move backwards or the forward difference is strictly greater than `MAX_JUMP_MS = 300_000`. A gap of exactly 300 seconds stays in the same segment. Equal ticks remain valid; rollover follows the existing backwards-tick behavior. Do not add a configurable threshold or clock-unwrapping logic. The first decoded message after a boundary belongs to the new segment.

Match the existing decoded-message boundary decisions, which are made after successful decoding. Retaining an unknown or invalid raw frame must not introduce an extra segment. Locate each boundary at the corresponding decoded record; attach raw-only records to the resulting source-record ranges, using one uncorrelated segment if the entire input has no decoded messages. Preserve capture provenance through `message_seq`. Parquet does not resample signals. GPS-step splitting and other changes to segmentation semantics are outside scope.

### 7.2 Two passes for timestamp correction (later PR)

Keep the existing affine correction model:

```text
UTC milliseconds ~= slope * DAQ ticks milliseconds + intercept
```

Future Parquet orchestration will use the shared stages in two passes; this PR retains the existing whole-file/whole-segment processing unchanged. Pass 1 reads and decodes records to identify segment boundaries and accumulate the GPS information needed for each segment's correction. Pass 2 rereads records, applies finalized corrections, detects events, and writes chunks. This allows later GPS anchors to correct earlier chunks without retaining all messages in RAM. CSV can continue collecting decoded messages and passing correlated segments to its existing table writer, using the same common decoding, segmentation, and correction helpers.

Keep regression fitting in the shared correlation module. Support bounded accumulation for Parquet without introducing a second fitting policy or silently changing the CSV calculation; verify agreement with the existing fit within a documented floating-point tolerance. Preserve the existing GPS extraction, date validation, single-anchor slope of 1.0, and failed-fit behavior. For an unavailable correction, write null UTC. Use the segment's correction to choose its event time basis. Do not add a new fit acceptance policy in this work.

The execution-year-dependent GPS date check remains as implemented. Improvements to historical-date validation or regression policy are separate work and must not be bundled into Parquet export. Record only the slope/intercept correction without changing how CSV computes timestamps.

In the later streaming PR, read 16-byte records through `BufReader` with explicit byte order. Reject an export if source contents change between passes. Segment summaries are small; if their count becomes large, spool completed summaries to disk instead of accumulating an unbounded collection.

### 7.3 Writer limits and completion

Keep matching Apache Arrow Rust `arrow-array`, `arrow-schema`, and `parquet` versions for `log_parse::parquet_export`, with existing serde/serde_json abstractions. The writer only needs flat Arrow batches; no nested schemas or Rust Polars dependency are required.

Start with batches bounded by both row count and bytes, approximately 4 MiB, and row groups around 16 MiB. Start a new signal chunk around 64 MiB of signal output or at a configured message-count cap. These are tuning targets, not exact file-size guarantees. Rotate signal files between complete messages. Keep the segment's frame and event writers open across signal chunk rotations, flushing bounded row groups incrementally and closing both files at the segment boundary.

Keep one current signal chunk, the segment's frame/event writers, bounded buffers, and detector state. All three writers' buffered row groups count toward the memory budget. Use their memory estimates to flush early; a small input batch alone does not bound Parquet buffering. Segment-level frame/event files can be large on disk without loading their contents into RAM. Monitor footer metadata as row-group counts grow; exceptionally long segments may eventually justify optional frame/event sharding, which is deferred from the initial layout.

Write files under temporary names and close their Parquet footers before renaming. Publish the final manifest last. An interrupted export must not appear complete. Local restart can rerun the export; resumable parsing is not part of v1.

## 8. Export Entry Points: daqcli and daqapp

Expose `log_parse::csv_export` and `log_parse::parquet_export` entry points and add explicit format selection in daqcli and the app's existing background parsing worker. Preserve current CSV defaults. All CSV callers use `log_parse::csv_export::parse_logs_to_tables` directly.

Both entry points use the shared `log_parse` stages; neither exporter calls the other. Moving `table.rs` into `csv_export` must preserve its current headers, binning, filenames, and values. Backend and Plot Factory integrations remain out of scope.

## 9. Implementation Steps and Validation

See the [ordered implementation roadmap](daq-parquet-implementation.md) for the updated sequence and completion checks.

1. Retain the flat schemas and minimal manifest under `log_parse::parquet_export`.
2. Reorganize existing parser, segmentation, correlation, and CSV modules under `log_parse`, updating all import paths and preserving function bodies. This is the scope of the current PR.
3. In a later PR, introduce shared streaming reading/raw provenance and bounded first-pass analysis. Preserve existing source selection, segmentation, decoding, and timestamp policies; validate parser changes in that PR.
4. Build the Parquet writer with signal chunks and segment-level frames/events files, using the shared pipeline.
5. Add the shared event lifecycle and connect it to the Parquet event writer; production detector rules remain deferred.
6. Expose explicit format selection in CLI/app while preserving CSV defaults.
7. Verify bounded memory and output finalization, and document usage.

For this PR, verify that moved parser, segmentation, correlation, and CSV bodies are unchanged apart from module references, and compile all callers against the new module paths. Later streaming work needs focused validation of ordering, identity handling, raw provenance, partial tails, and 300-second boundaries. Do not redesign CSV formatting or expand into unrelated logging changes.

Tests remain deferred at the user's request. Build, formatting, and clippy checks remain appropriate for each implementation stage. The desktop logger and CAN driver only need module-reference updates. Plot Factory and daqcloud need no changes.

## 10. Remaining Decisions

- Choose the final batch/chunk limits using representative logs; repeated descriptions trade some file size for simpler consumers.
- Define SDC transition semantics, watchdog evidence, and lap geometry before implementing those detectors.

The initial format uses chunked flat signal files, one frames file and one events file per segment, and one dataset manifest, with referenced external definitions and ordinary Rust detector logic.
