# DAQ Parquet Intermediate Format: Simplified Design Plan

**Status:** Draft proposal; implementation has not started.  
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
- Preserve the existing CSV export entry point and behavior, including file ordering, segmentation, timestamp correction, headers, binning, filenames, and enum handling. Add Parquet as a separate export path; do not route CSV through it or migrate CSV consumers as a prerequisite.
- Limit shared-code edits to small, behavior-preserving helpers where necessary. A streaming Parquet implementation is not a reason to refactor the working CSV pipeline.
- Exclude Plot Factory integration, plotting/derived-signal changes, daqcloud ingestion, database migrations, upload protocols, and retry/checkpoint design.

These constraints supersede the broader proposals in the original requirements where they conflict. The flat schemas and segment-level frame/event layout remain the intended Parquet design.

## 2. Existing Code and Integration Points

| Area | Current behavior | Planned change |
|---|---|---|
| [Pipeline](../daq/daqcore/src/log_parse/mod.rs) | Parse logs, split chunks, correlate time, export CSV. | Add a Parquet export path using the same decoder and correction model. |
| [Binary parser](../daq/daqcore/src/log_parse/parse.rs) | Reads complete files and accumulates decoded messages; drops undecodable frames. | Add a streaming reader for the Parquet path; leave the CSV parser behavior unchanged. |
| [Raw frame](../firmware/common/timestamped_frame/timestamped_frame.h) | 16 bytes: ticks, packed identity, eight payload bytes. | Preserve those original values in the frames table. |
| [Desktop logger](../daq/daqcore/src/can/logger.rs) | Rotates `.log` files approximately every 60 seconds. | No changes. Consume its existing output. |
| [Segmentation](../daq/daqcore/src/log_parse/parse.rs) | Splits on backwards ticks or forward jumps greater than 300 seconds. | Preserve the same fixed rule and boundary decisions; split only Parquet signal storage further into size-bounded chunks. |
| [Timestamp correction](../daq/daqcore/src/log_parse/correlate.rs) | Fits an affine mapping using `gps_time`. | Reuse the existing correction semantics in the Parquet path; do not change CSV timestamp results or validation. |
| [CSV export](../daq/daqcore/src/log_parse/table.rs) | Wide table on a 1 ms grid with seven metadata rows. | Leave unchanged. Parquet writes long rows through a separate writer. |
| [Plot Factory](../plot_factory/factory/loading.py) | Eagerly loads and concatenates wide CSVs. | Out of scope; leave its loaders, plots, and tests unchanged. |
| [daqcloud](../daq/daqcloud/src/main.rs) | Separate backend application. | Out of scope; no changes or ingestion planning. |

Reuse SuperDBC message identity and decoding. The canonical message definition key is `(bus_id, can_id, is_extended)`; signal name identifies a signal within that message.

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
| `source_file` | `Utf8` | Unique source label listed in the manifest. |
| `source_record` | `UInt64` | Zero-based 16-byte record index in that source. |
| `daq_ticks_ms` | `UInt32` | Original DAQ ticks. |
| `raw_identity` | `UInt32` | Original packed identity, including transport flags. |
| `payload` | `Binary` | Original eight payload bytes. |
| `decode_status` | `Utf8` | `ok`, `invalid_identity`, `unknown_bus`, `unknown_message`, or `decode_error`. |

Every complete input record receives a frame row. Unknown or invalid messages produce no signal rows. A decoded message with no signals still has a frame row.

The raw source format does not contain actual DLC, RTR status, or CAN FD payloads. Do not invent those values. Preserve the offset and bytes of a trailing partial record in source metadata rather than padding it into a synthetic frame.

Source labels must be unique even when input files share a basename. Include their original paths/names in the manifest for traceability. Byte offsets are `source_record * 16` for the current format.

## 5. Events per Segment

Each segment has one flat event table:

| Column | Type | Meaning |
|---|---|---|
| `name` | `Utf8` | Event name, such as an SDC trip or completed lap. |
| `color` | `Utf8` | Display color in `#RRGGBB` form. |
| `start_time_ms` | `Int64` | Event start in the segment's declared event time basis. |
| `end_time_ms` | `Int64?` | End time for an interval; null for an instantaneous event. |
| `attributes_json` | `Utf8?` | Optional JSON object for details such as lap number or an incomplete interval. |

For segments with accepted UTC correction, event times are Unix milliseconds in UTC. Otherwise, they are raw DAQ milliseconds within that segment. Record `event_time_basis` as `utc_ms` or `daq_ticks_ms` in the segment metadata. Never infer the basis from a timestamp's magnitude or mix bases within a segment.

Event detection remains independent of Parquet. A small daqcore parsing module receives messages, retains detector state, and emits ordinary Rust event values. A writer streams those values into the current segment's event file. The same detector logic can later receive adapted live messages.

### 5.1 Intervals across boundaries

- An instantaneous event is appended to the segment event file when detected.
- An interval is appended once when it ends or is finalized. Its start and end can fall in different signal chunks.
- Detector state continues across source-file and chunk boundaries. Chunk rotation does not split an event or reset detection.
- At a segment boundary or end of input, finalize an active interval at the last observed time and set `incomplete: true` in its attributes. Do not assert continuity across a DAQ clock reset.
- Write an empty typed event file when a segment has no emitted events.

Readers scan the selected segment's event file in batches and select intervals overlapping the requested time window. Intervals are stored in completion order, so readers must not assume the file is sorted by start time. No event-to-chunk ownership or event-indexing system is needed.

Actual SDC, watchdog, and lap rules are follow-up work. Existing fault-event and fault-sync messages provide inputs for fault detectors. Watchdog evidence and lap geometry/debounce rules still need specification; a gap alone does not prove a watchdog reset.

## 6. Small Manifest and External SuperDBC Reference

Keep one JSON manifest with the information needed to interpret and locate the tables:

| Item | Contents |
|---|---|
| Versions | Format version, initially `1.0`, and parser version/revision. |
| Definitions | SuperDBC filename/reference, schema/version metadata, and checksum of the actual artifact used. |
| Sources | Ordered source labels, original paths/names, sizes, checksums, record counts, byte order, and any partial trailing bytes. |
| Settings | Record the existing fixed `300_000 ms` jump threshold and relevant timestamp/detector settings. |
| Segments | Start/end ticks, corrected UTC bounds when available, boundary reason, correction coefficients, GPS anchor count/fit error, event time basis, and frame/event file paths and row counts. |
| Chunks | Signal file paths, message ranges, row counts, and available time bounds. |
| Events | Enabled detector names/versions, so empty results can be distinguished from detection being disabled. |
| Diagnostics | Counts of malformed and undecodable records. |

Do not copy or move SuperDBC into the export. Compute its checksum from the artifact actually used for decoding; the existing SuperDBC declared hashes are useful metadata but are not independent integrity checks.

Normal plotting uses the names, descriptions, and units already in the signal table and does not require access to the referenced file. Re-decoding raw frames or inspecting complete bit layouts requires supplying a matching external SuperDBC artifact. A recorded local path is a provenance hint, not a portable location guarantee.

Use one compatible SuperDBC artifact per export initially. Multiple definition versions and independent recorder clocks are deferred.

Reject unsupported format major versions and incompatible column types. Additive fields can use a minor version when existing semantics stay unchanged. Further compatibility machinery is unnecessary for the initial implementation.

## 7. Parsing, Correction, and Bounded Writing

### 7.1 Segmentation

Use the current directory selection and lexicographic source-file ordering; record that order in the manifest. Do not introduce natural filename sorting as part of this change. Continue a segment across normal log rotation as the existing parser does.

Use the existing condition from `chunk_parsed`: start a new segment when ticks move backwards or the forward difference is strictly greater than `MAX_JUMP_MS = 300_000`. A gap of exactly 300 seconds stays in the same segment. Equal ticks remain valid; rollover follows the existing backwards-tick behavior. Do not add a configurable threshold or clock-unwrapping logic. The first decoded message after a boundary belongs to the new segment.

Match the existing decoded-message boundary decisions, which are made after successful decoding. Retaining an unknown or invalid raw frame must not introduce an extra segment. Locate each boundary at the corresponding decoded record; attach raw-only records to the resulting source-record ranges, using one uncorrelated segment if the entire input has no decoded messages. Preserve capture provenance through `message_seq`. Parquet does not resample signals. GPS-step splitting and other changes to segmentation semantics are outside scope.

### 7.2 Two passes for timestamp correction

Keep the existing affine correction model:

```text
UTC milliseconds ~= slope * DAQ ticks milliseconds + intercept
```

The Parquet-only path uses two passes. Pass 1 streams records to identify the existing segment boundaries and accumulate the GPS information needed for each segment's correction. Pass 2 rereads the records, applies the finalized correction, decodes signals, detects events, and writes chunks. This allows later GPS anchors to correct earlier chunks without retaining all messages in RAM. The current CSV path continues to call its existing parse/chunk/correlate/table functions.

Use bounded regression statistics for Parquet and verify agreement with the existing fit within a documented floating-point tolerance. Preserve the existing GPS extraction, date validation, single-anchor slope of 1.0, and failed-fit behavior. For an unavailable correction, write null UTC. Use the segment's correction to choose its event time basis. Do not add a new fit acceptance policy in this work.

The execution-year-dependent GPS date check remains as implemented. Improvements to historical-date validation or regression policy are separate work and must not be bundled into Parquet export. Record available correction metadata without changing how CSV computes timestamps.

Read 16-byte records through `BufReader` with explicit byte order. Reject an export if source contents change between passes. Segment summaries are small; if their count becomes large, spool completed summaries to disk instead of accumulating an unbounded collection.

### 7.3 Writer limits and completion

Use matching Apache Arrow Rust `arrow-array`, `arrow-schema`, and `parquet` versions with existing serde/serde_json abstractions. The writer only needs flat Arrow batches; no nested schemas or Rust Polars dependency are required.

Start with batches bounded by both row count and bytes, approximately 4 MiB, and row groups around 16 MiB. Start a new signal chunk around 64 MiB of signal output or at a configured message-count cap. These are tuning targets, not exact file-size guarantees. Rotate signal files between complete messages. Keep the segment's frame and event writers open across signal chunk rotations, flushing bounded row groups incrementally and closing both files at the segment boundary.

Keep one current signal chunk, the segment's frame/event writers, bounded buffers, and detector state. All three writers' buffered row groups count toward the memory budget. Use their memory estimates to flush early; a small input batch alone does not bound Parquet buffering. Segment-level frame/event files can be large on disk without loading their contents into RAM. Monitor footer metadata as row-group counts grow; exceptionally long segments may eventually justify optional frame/event sharding, which is deferred from the initial layout.

Write files under temporary names and close their Parquet footers before renaming. Publish the final manifest last. An interrupted export must not appear complete. Local restart can rerun the export; resumable parsing is not part of v1.

## 8. Export Entry Points: daqcli and daqapp

Add a separate daqcore Parquet-export entry point and an explicit format selection in daqcli and the app's existing background parsing worker. Preserve current CSV defaults and calls to `parse_logs_to_tables`. Reuse SuperDBC decoding and existing correction helpers where practical; any shared helper extraction must preserve behavior and remain narrowly scoped. Leave `table.rs` and CSV output formatting unchanged.

## 9. Implementation Steps and Tests

1. Define the three flat schemas, segment-level frame/event layout, and small manifest. Write and read a tiny fixture using the Rust Parquet reader to verify field types, metadata, and empty event files. Consumer application integration is not required for format validation.
2. Introduce a streaming raw-record reader with source positions. Test byte order, packed identity, unknown messages, duplicate ticks, and trailing partial records.
3. Implement the Parquet-only two-pass path using the existing segmentation and correction semantics. Test gaps below, exactly at, and above 300 seconds; multiple files in one segment; backwards ticks/rollover; unknown frames between decoded messages; and zero/one/many GPS anchors. Compare segment boundaries and timestamps against the existing pipeline without changing it.
4. Add bounded Parquet writing and connect CLI/app export. Test that signal chunk sizes do not alter signal values, message grouping, frame rows, or events; all signals of a message stay together and segment frame/event writers remain open across rotations. Include large raw integers and unusual floats to verify raw preservation despite physical-value precision limits.
5. Add event types and serialization with a synthetic detector. Test instant events, intervals across signal chunks and source files, segment truncation, event time basis, empty segment event files, and overlap selection for intervals stored in completion order.
6. Benchmark generated recordings larger than RAM. Measure batch buffers, writer row groups, event state, and metadata; verify memory does not grow with message count. Test interrupted finalization and input changes between passes.

Use the existing Rust build/test/format/clippy checks for implementation changes. Check that CSV selection still reaches the existing entry point; if a small shared helper must change, use a focused before/after CSV fixture check. Do not expand this effort into a CSV rewrite or broad revalidation project. The desktop logger, Plot Factory, and daqcloud require no changes or additional validation work for this export-only scope. Production event detectors are separate follow-up changes.

## 10. Remaining Decisions

- Choose the final batch/chunk limits using representative logs; repeated descriptions trade some file size for simpler consumers.
- Determine the smallest helper reuse needed for the Parquet reader/correction path while leaving CSV behavior intact.
- Define SDC transition semantics, watchdog evidence, and lap geometry before implementing those detectors.

The initial format uses chunked flat signal files, one frames file and one events file per segment, and one dataset manifest, with referenced external definitions and ordinary Rust detector logic.
