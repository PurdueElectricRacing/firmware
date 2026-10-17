pub mod manifest;
pub mod schema;

use std::path::PathBuf;

// parquet export version
pub const FORMAT_VERSION: &str = "1.0";

pub const MANIFEST_FILE: &str = "manifest.json";

/// Approximate decoded batch size before passing data to the writer
pub const BATCH_TARGET_BYTES: usize = 4 * 1024 * 1024;
/// Approximate buffered row-group size before flushing
pub const ROW_GROUP_TARGET_BYTES: usize = 16 * 1024 * 1024;
/// Approximate signal file size before rotating chunks
pub const SIGNAL_CHUNK_TARGET_BYTES: u64 = 64 * 1024 * 1024;

/// Dataset-relative directory for one continuous DAQ clock region.
pub fn segment_path(segment: u32) -> PathBuf {
    PathBuf::from(format!("segment-{segment:03}"))
}

/// All signals of one message stay together in one signal chunk.
pub fn signal_chunk_path(segment: u32, chunk: u32) -> PathBuf {
    segment_path(segment)
        .join("signals")
        .join(format!("signals-{chunk:03}.parquet"))
}

pub fn frames_path(segment: u32) -> PathBuf {
    segment_path(segment).join("frames.parquet")
}

pub fn events_path(segment: u32) -> PathBuf {
    segment_path(segment).join("events.parquet")
}
