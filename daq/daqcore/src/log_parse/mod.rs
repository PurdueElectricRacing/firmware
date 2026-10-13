use std::path;

pub mod consts;
pub mod correlate;
pub mod parse;
pub mod table;
use crate::superdbc;

pub fn parse_logs_to_tables(
    logs_dir: &path::Path,
    output_dir: &path::Path,
    output_prefix: &str,
    database: &superdbc::database::Database,
) -> Result<(), csv::Error> {
    let parsed = parse::parse_log_files(logs_dir, database)?;
    let chunked = parse::chunk_parsed(parsed);
    let correlated = correlate::time_correlate_chunks(chunked);

    let mut table_builder = table::TableBuilder::default();
    for bus in [0, 1].into_iter().filter_map(|id| database.bus_by_id(id)) {
        table_builder.create_header(bus);
    }
    table_builder.create_and_write_tables(output_dir, output_prefix, correlated)
}
