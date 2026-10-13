pub mod protocol;
pub mod updater;

#[derive(Clone, Debug)]
pub struct FirmwareProgress {
    pub board: String,
    pub board_index: usize,
    pub board_count: usize,
    pub phase: String,
    pub sent_bytes: usize,
    pub total_bytes: usize,
    pub error: Option<String>,
}
