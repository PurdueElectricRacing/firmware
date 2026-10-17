# DAQ Parquet Export: Ordered Implementation Roadmap

**Status:** Planned; no implementation started.  
**Design:** [Simplified Parquet design](daq-parquet-design.md)

## Scope

Implement offline Parquet export in daqcore and expose it in daqcli and daqapp. Leave the desktop logger, raw logging format, and existing CSV pipeline unchanged. Preserve the fixed segmentation rule: backwards ticks or a forward jump strictly greater than 300 seconds.

Plot Factory integration, daqcloud ingestion, and production event detectors are outside this roadmap. SuperDBC remains external to the output directory.

## Sequence and Dependencies

Complete the stages in order. Each stage should be a small, reviewable change with its own relevant checks. This is a dependency timeline rather than a calendar estimate.

| Order | Stage | Result |
|---|---|---|
| 1 | Define output types and schemas | Flat tables and a manifest can be written and read. |
| 2 | Add the Parquet raw-record reader | Incremental records retain source positions and raw bytes. |
| 3 | Implement segment and clock analysis | First-pass output supplies existing segment boundaries and correction models. |
| 4 | Build the streaming exporter | A complete dataset can be exported through a daqcore API. |
| 5 | Add the event lifecycle | Segment event files support storage-independent detector outputs. |
| 6 | Expose CLI and app selection | Users can explicitly choose Parquet while CSV remains the default. |
| 7 | Verify resource limits and document usage | The export is ready for use on representative recordings. |

## 1. Define Output Types and Schemas

- Add a dedicated Parquet export module in daqcore, separate from CSV table generation.
- Add matching Arrow/Parquet dependencies and small Rust types for the manifest and export options.
- Define the flat signal, frame, and event schemas from the design document.
- Establish the directory layout: `segment-NNN/signals/signals-NNN.parquet`, with `frames.parquet` and `events.parquet` directly under each segment.
- Keep schema version, units, nullable UTC, source labels, and `message_seq` semantics explicit.

**Completion check:** Write and read a tiny fixture using the Rust Parquet reader. Verify column types, values, and a typed empty event file. Do not add consumer integrations to validate the format.

## 2. Add an Incremental Raw-Record Reader

- Read the existing 16-byte format through `BufReader` with explicit byte order.
- Preserve the current `.log` selection and lexicographic file ordering.
- Attach source label, zero-based source-record index, and dataset-wide `message_seq` to every complete record.
- Reuse SuperDBC decoding; retain undecodable records for the frames table.
- Capture trailing partial-record bytes in source metadata.
- Leave the existing CSV reader and desktop logger untouched.

**Completion check:** Fixtures cover multiple files, identity flags, duplicate ticks, unknown messages, invalid records, and partial tails. Verify source offsets and stable message ordinals.

## 3. Implement First-Pass Segment and Clock Analysis

- Match the current segmentation decisions on successfully decoded messages: backwards ticks or `delta > 300_000 ms`.
- Map those boundaries back to raw source-record ranges without allowing unknown frames to introduce extra segments.
- Preserve file-rotation continuity and existing rollover behavior.
- Accumulate GPS fitting information with bounded memory; use the existing extraction, date-validation, single-anchor, and failed-fit semantics.
- Reuse existing correction helpers where practical. Restrict any shared-code extraction to small, behavior-preserving changes.
- Produce segment descriptors, correction metadata, source checksums, and the external SuperDBC reference/checksum for pass two.

**Completion check:** Compare boundaries and corrected timestamps against the existing pipeline on small fixtures. Cover gaps below, exactly at, and above 300 seconds; resets; unknown frames; and zero, one, and multiple GPS anchors. Document any floating-point comparison tolerance. Do not change CSV validation policy.

## 4. Build the Streaming Parquet Exporter

- Add a daqcore export entry point that runs analysis, rereads records, and writes the output.
- Write one frame row per complete record and one signal row per decoded signal.
- Keep one segment-level frames writer open while rotating signal chunks between complete messages.
- Bound batches and row groups by bytes and rows. Start with the design's approximate 4 MiB batches, 16 MiB row groups, and 64 MiB signal chunks; treat these as initial tuning values.
- Initially write a typed empty segment event file; stage 5 supplies detected events.
- Build the small manifest with source provenance, correction metadata, file inventory, counts, and diagnostic totals.
- Verify source contents did not change between passes. Finalize Parquet files before publishing the manifest.

**Completion check:** Export a multi-file fixture with multiple segments and forced small signal chunks. Verify joins through `message_seq`, raw preservation, nullable UTC, complete-message chunk boundaries, and independence from batch/chunk size. An input change or interrupted export must not publish a completed dataset.

**Milestone:** A usable offline exporter exists through daqcore without modifying CSV or user-facing applications.

## 5. Add Segment Event Serialization and Detector Lifecycle

- Define ordinary Rust event values and a small detector lifecycle for observing messages, segment boundaries, and end of input.
- Connect detector outputs to the existing segment-level event writer.
- Keep detector state across source files and signal chunk rotations.
- Append an interval when completed; finalize active intervals at segment/end-of-input boundaries with an incomplete marker.
- Use the segment's declared UTC or DAQ-tick event time basis.
- Exercise the lifecycle with a synthetic test detector. Do not implement SDC, watchdog, or lap rules in this stage.

**Completion check:** Cover instant events, cross-chunk intervals, source-file rotation, truncated intervals, empty event files, and event row-group flushing. Detection logic must not depend on Arrow or Parquet types.

## 6. Expose Parquet in daqcli and daqapp

- Add explicit CSV/Parquet format selection while preserving CSV defaults and current CLI invocations.
- Route Parquet selection to the new exporter and CSV selection to the existing `parse_logs_to_tables` entry point.
- Use the app's current background parsing worker to invoke Parquet export and report its completion/errors.
- Document the output directory and the fact that raw re-decoding requires a matching external SuperDBC file.

**Completion check:** Verify explicit Parquet export through CLI and app, existing CSV dispatch, and useful errors for invalid inputs or unwritable output. If shared helpers changed, use a focused before/after CSV fixture comparison; do not launch a broader CSV revalidation effort.

**Milestone:** Users can generate the planned dataset without changing their existing CSV workflow.

## 7. Verify Resource Limits and Finish Documentation

- Run a generated recording larger than the available memory budget through export and batch reading.
- Measure all active writer buffers, detector state, segment summaries, and footer metadata; tune limits if needed.
- Confirm segment frames/events are streamed rather than accumulated and signal chunk rotation does not alter results.
- Run relevant Rust tests and workspace build, formatting, and clippy checks.
- Add concise CLI/app usage and an example of the generated directory layout.

**Completion check:** Demonstrate bounded message processing, valid output, and incomplete-output handling. Record measured tuning choices and any remaining limitations. No Plot Factory or daqcloud work is required for completion.

## Deferred Follow-ups

- Production SDC/fault, watchdog, and lap detectors once their rules are specified.
- Plot Factory loading and plotting integration.
- daqcloud ingestion and upload/retry design.
- Changes to GPS validation, segmentation policy, desktop logging, or CSV behavior, each as separate work if requested.
