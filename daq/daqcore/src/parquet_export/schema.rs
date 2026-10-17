use arrow_schema::{DataType, Field, Schema};
use std::{collections::HashMap, sync::Arc};

use super::FORMAT_VERSION;

fn table_schema(table: &str, fields: Vec<Field>) -> Arc<Schema> {
    Arc::new(Schema::new_with_metadata(
        fields,
        HashMap::from([
            ("daq.format_version".into(), FORMAT_VERSION.into()),
            ("daq.table".into(), table.into()),
        ]),
    ))
}

/// One decoded physical signal update per row, in message/definition order.
pub fn signals_schema() -> Arc<Schema> {
    table_schema(
        "signals",
        vec![
            // `message_seq` is a dataset-wide capture ordinal assigned before decoding.
            Field::new("message_seq", DataType::UInt64, false),
            Field::new("daq_ticks_ms", DataType::UInt32, false),
            Field::new("timestamp_utc_ms", DataType::Int64, true),
            Field::new("bus_id", DataType::UInt8, false),
            Field::new("bus_name", DataType::Utf8, false),
            Field::new("can_id", DataType::UInt32, false),
            Field::new("is_extended", DataType::Boolean, false),
            Field::new("node_name", DataType::Utf8, false),
            Field::new("message_name", DataType::Utf8, false),
            Field::new("message_desc", DataType::Utf8, false),
            Field::new("signal_name", DataType::Utf8, false),
            Field::new("signal_desc", DataType::Utf8, false),
            Field::new("signal_unit", DataType::Utf8, false),
            Field::new("value", DataType::Float64, false),
        ],
    )
}

/// One complete raw record per row, including records without decoded signals.
/// Payloads are exactly eight bytes.
/// `decode_status` is one of: ok, invalid_identity, unknown_bus, unknown_message, decode_error.
pub fn frames_schema() -> Arc<Schema> {
    table_schema(
        "frames",
        vec![
            // `message_seq` is a dataset-wide capture ordinal assigned before decoding.
            Field::new("message_seq", DataType::UInt64, false),
            Field::new("source_file", DataType::Utf8, false),
            // `source_record` is zero-based within its source file
            Field::new("source_record", DataType::UInt64, false),
            Field::new("daq_ticks_ms", DataType::UInt32, false),
            Field::new("raw_identity", DataType::UInt32, false),
            Field::new("payload", DataType::Binary, false),
            Field::new("decode_status", DataType::Utf8, false),
        ],
    )
}

/// Events are segment-local and may span signal chunks.
/// A null end is an instantaneous event.
/// Time basis is declared by the segment manifest, not inferred from the timestamp values.
pub fn events_schema() -> Arc<Schema> {
    table_schema(
        "events",
        vec![
            Field::new("name", DataType::Utf8, false),
            Field::new("color", DataType::Utf8, false),
            Field::new("start_time_ms", DataType::Int64, false),
            Field::new("end_time_ms", DataType::Int64, true),
        ],
    )
}
