use serde::{Deserialize, Serialize};
use std::path::PathBuf;

use super::FORMAT_VERSION;

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Manifest {
    pub format_version: String,
    pub parser_version: String,
    pub superdbc: SuperDbcReference,
    pub source_files: Vec<PathBuf>,
    pub segments: Vec<Segment>,
}

impl Manifest {
    pub fn new(parser_version: String, superdbc: SuperDbcReference) -> Self {
        Self {
            format_version: FORMAT_VERSION.into(),
            parser_version,
            superdbc,
            source_files: Vec::new(),
            segments: Vec::new(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct SuperDbcReference {
    pub schema_version: String,
    pub content_hash: String,
    pub git_hash: String,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Segment {
    pub directory: PathBuf,
    pub correction: Option<ClockCorrection>,
    pub time_basis: TimeBasis,
    pub start_time_ms: i64, // depends on time_basis
    pub end_time_ms: i64,
    pub signal_chunks: Vec<SignalChunk>,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct SignalChunk {
    pub path: PathBuf,
    pub start_time_ms: i64, // depends on time_basis
    pub end_time_ms: i64,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
/// `unix_ms = slope * daq_ticks_ms + intercept_ms`, rounded to milliseconds.
pub struct ClockCorrection {
    pub slope: f64,
    pub intercept_ms: f64,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum TimeBasis {
    UtcMs,
    DaqTicksMs,
}
