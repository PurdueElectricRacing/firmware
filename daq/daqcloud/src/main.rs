mod db;
mod schema;

use axum::{Router, routing::get};

#[tokio::main]
async fn main() -> Result<(), db::Error> {
    let database_url = std::env::var("DATABASE_URL").unwrap_or_else(|_| "daqcloud.sqlite3".into());
    let database = tokio::task::spawn_blocking(move || db::connect(&database_url)).await??;

    let app = Router::new()
        .route("/", get(|| async { "daqcloud\n" }))
        .with_state(database);

    let address = std::env::var("BIND_ADDR").unwrap_or_else(|_| "127.0.0.1:3000".into());
    let listener = tokio::net::TcpListener::bind(&address).await?;
    println!("daqcloud listening on http://{}", listener.local_addr()?);

    axum::serve(listener, app)
        .with_graceful_shutdown(async {
            if let Err(error) = tokio::signal::ctrl_c().await {
                eprintln!("Could not listen for shutdown: {error}");
            }
        })
        .await?;
    Ok(())
}
