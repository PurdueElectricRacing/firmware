use std::path::PathBuf;
use std::process::ExitCode;

use clap::Parser as ClapParser;

#[derive(ClapParser)]
#[command(name = "daqcli", about = "Parse DAQ logs into CSV tables")]
struct Cli {
    #[arg(long)]
    logs_dir: PathBuf,
    #[arg(long)]
    output_dir: PathBuf,
    #[arg(long, default_value = "out")]
    prefix: String,
    #[arg(long)]
    superdbc: PathBuf,
}

fn main() -> ExitCode {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    let cli = Cli::parse();

    let database = match daqcore::superdbc::database::Database::load(&cli.superdbc) {
        Ok(database) => database,
        Err(error) => {
            log::error!("SuperDBC load failed ({}): {error}", cli.superdbc.display());
            return ExitCode::FAILURE;
        }
    };

    match daqcore::log_parse::parse_logs_to_tables(
        &cli.logs_dir,
        &cli.output_dir,
        &cli.prefix,
        &database,
    ) {
        Ok(()) => ExitCode::SUCCESS,
        Err(e) => {
            log::error!("log parsing failed: {e}");
            ExitCode::FAILURE
        }
    }
}
