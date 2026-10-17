# DAQ Parquet Format — Design Goals

## Objective

Design a canonical, portable Parquet-based intermediate format for our DAQ system. This format will sit between raw binary `.log` parsing and downstream consumers, including local analysis and the cloud backend.

**First inspect the existing repository, parsing pipeline, timestamp correction, logging structures, and export functionality. Then propose a concrete implementation plan. Do not implement yet.**

## Requirements

### 1. Canonical intermediate format
- Convert raw binary `.log` files into a well-defined, versioned Parquet dataset.
- Prefer a **long or message-oriented format**, rather than a wide format with one column per signal. CAN messages arrive asynchronously and have different signal sets.
- Preserve raw DAQ ticks, globally corrected timestamps, message identity, decoded signal values, and enough metadata to interpret the data.
- Preserve sufficient information to support future reprocessing and debugging.
- Avoid dependence on backend-generated database primary keys.

### 2. Segmentation and chunking
- Support combining multiple `.log` files into one logical dataset.
- Split data into segments at significant timestamp discontinuities (approximately 5+ seconds, configurable).
- Support further chunking large segments into multiple Parquet files to bound memory and file sizes.
- Ensure processing and reading can happen incrementally without loading an entire dataset into RAM.

### 3. Events
- Support automatic detection of meaningful events during parsing, such as:
  - SDC trips and fault conditions.
  - Board watchdog resets.
  - Completed laps and lap times.
  - These will be later implemented in a module in daqcore parsing.
- Store events separately from the main message/signal data, preferably in `events.parquet`.
- Support instantaneous events and time intervals, with event type, timestamps, source, severity, and optional structured attributes.
- Keep event detection independent of the storage format so it can be reused for live and offline analysis.
- Events should be associated with the logical dataset, not inherently restricted to individual file chunks.

### 4. Offline-first and backend ingestion
- All parsing, timestamp correction, and event detection must work locally without internet.
- The resulting dataset should support local analysis and visualization.
- Parquet is an **intermediate/transport format**, not the intended permanent cloud storage.
- Eventually, the backend will receive Parquet files, read them incrementally, validate their contents, and ingest them into a single relational database.
- The backend will own persistent database IDs, upload metadata, and ingestion state.
- Avoid requiring expensive parsing or timestamp correction on the backend, which is expected to run on a 1 shared CPU, 1 GB RAM VPS.
- Support reliable, resumable uploads and idempotent ingestion in the future.

### 5. Metadata and compatibility
- Include a dataset manifest describing schema version, parser version, source files, segments, chunks, timestamp units, and relevant processing metadata.
- Preserve message/signal definitions or references to the definitions needed to interpret the dataset.
- Ensure the format can evolve without silently breaking compatibility.
- Prefer existing Rust ecosystem libraries and project abstractions.

## Deliverable

After reviewing the repository, produce an implementation plan covering:

1. Proposed Parquet schemas and directory structure.
2. Integration with the existing `.log` parser and timestamp correction pipeline.
3. Streaming/chunking strategy and memory considerations.
4. Event detection and serialization architecture.
5. Metadata, schema versioning, and compatibility.
6. Incremental implementation steps and testing strategy.

Identify any important architectural tradeoffs or open questions. Favor simple, maintainable solutions over unnecessary abstraction or infrastructure.

(Remember to look at the existing codebase. This parquet system will be the future primary format that daqcloud ingests as well as plot factory.)