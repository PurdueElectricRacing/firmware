use clap::Parser as _;

#[derive(clap::Parser)]
#[command(name = "daqcli", about = "Parse DAQ logs into CSV tables")]
struct Cli {
    #[arg(long)]
    logs_dir: std::path::PathBuf,
    #[arg(long)]
    output_dir: std::path::PathBuf,
    #[arg(long, default_value = "out")]
    prefix: String,
    #[arg(long)]
    superdbc: std::path::PathBuf,
}

fn main() -> std::process::ExitCode {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    let cli = Cli::parse();

    let database = match daqcore::superdbc::database::Database::load(&cli.superdbc) {
        Ok(database) => database,
        Err(error) => {
            log::error!("SuperDBC load failed ({}): {error}", cli.superdbc.display());
            return std::process::ExitCode::FAILURE;
        }
    };

    match daqcore::log_parse::parse_logs_to_tables(
        &cli.logs_dir,
        &cli.output_dir,
        &cli.prefix,
        &database,
    ) {
        Ok(()) => std::process::ExitCode::SUCCESS,
        Err(e) => {
            log::error!("log parsing failed: {e}");
            std::process::ExitCode::FAILURE
        }
    }
}
