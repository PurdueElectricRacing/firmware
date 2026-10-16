mod consts;
mod db;
mod state;

#[tokio::main]
async fn main() -> Result<(), db::Error> {
    let database =
        tokio::task::spawn_blocking(move || db::init_pool(consts::DATABASE_URL)).await??;
    let state = state::AppState { db: database };

    let app = axum::Router::new()
        .route("/", axum::routing::get(|| async { "daqcloud\n" }))
        .with_state(state);

    let address: std::net::SocketAddr = consts::BIND_ADDR.parse().expect("Invalid bind address");
    let listener = tokio::net::TcpListener::bind(address).await?;
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
