mod db;

use std::env;

#[tokio::main]
async fn main() -> Result<(), db::Error> {
    let database_url = env::var("DATABASE_URL").unwrap_or_else(|_| "daqcloud.sqlite3".into());
    let database = tokio::task::spawn_blocking(move || db::init_pool(&database_url)).await??;

    let app = axum::Router::new()
        .route("/", axum::routing::get(|| async { "daqcloud\n" }))
        .with_state(database);

    let address = env::var("BIND_ADDR").unwrap_or_else(|_| "127.0.0.1:3000".into());
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
