use clap::Parser as _;
use std::{path, process};

#[derive(clap::Parser)]
#[command(name = "daqcli", about = "Parse DAQ logs into CSV tables")]
struct Cli {
    #[arg(long)]
    logs_dir: path::PathBuf,
    #[arg(long)]
    output_dir: path::PathBuf,
    #[arg(long, default_value = "out")]
    prefix: String,
    #[arg(long)]
    superdbc: path::PathBuf,
}

fn main() -> process::ExitCode {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    let cli = Cli::parse();

    let database = match daqcore::superdbc::database::Database::load(&cli.superdbc) {
        Ok(database) => database,
        Err(error) => {
            log::error!("SuperDBC load failed ({}): {error}", cli.superdbc.display());
            return process::ExitCode::FAILURE;
        }
    };

    match daqcore::log_parse::parse_logs_to_tables(
        &cli.logs_dir,
        &cli.output_dir,
        &cli.prefix,
        &database,
    ) {
        Ok(()) => process::ExitCode::SUCCESS,
        Err(e) => {
            log::error!("log parsing failed: {e}");
            process::ExitCode::FAILURE
        }
    }
}
