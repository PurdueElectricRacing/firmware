use crate::db;

#[derive(Clone)]
pub struct AppState {
    pub db: db::DbPool,
}
