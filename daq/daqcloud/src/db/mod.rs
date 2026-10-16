use std::time::Duration;

use diesel::{
    connection::SimpleConnection,
    r2d2::{ConnectionManager, CustomizeConnection, Pool},
    sqlite::SqliteConnection,
};
use diesel_migrations::{EmbeddedMigrations, MigrationHarness, embed_migrations};

pub type Error = Box<dyn std::error::Error + Send + Sync>;
pub type DbPool = Pool<ConnectionManager<SqliteConnection>>;

const MIGRATIONS: EmbeddedMigrations = embed_migrations!("migrations");

#[derive(Debug)]
struct SqliteSettings;

impl CustomizeConnection<SqliteConnection, diesel::r2d2::Error> for SqliteSettings {
    fn on_acquire(&self, conn: &mut SqliteConnection) -> Result<(), diesel::r2d2::Error> {
        // These settings apply to each connection, including pool replacements.
        conn.batch_execute("PRAGMA busy_timeout = 5000; PRAGMA foreign_keys = ON;")
            .map_err(diesel::r2d2::Error::QueryError)
    }
}

/// Open a file-backed SQLite database and migrate it before serving requests.
/// Call from a blocking thread: Diesel's SQLite driver is synchronous.
pub fn connect(database_url: &str) -> Result<DbPool, Error> {
    let manager = ConnectionManager::<SqliteConnection>::new(database_url);
    let pool = Pool::builder()
        .max_size(4)
        .connection_timeout(Duration::from_secs(5))
        .connection_customizer(Box::new(SqliteSettings))
        .build(manager)?;

    {
        let mut conn = pool.get()?;
        // WAL is persistent for this database file and allows readers during writes.
        conn.batch_execute("PRAGMA journal_mode = WAL;")?;
        conn.run_pending_migrations(MIGRATIONS)?;
    }

    Ok(pool)
}