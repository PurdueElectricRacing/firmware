//! Caller-managed worker. Only commands/events cross threads; telemetry stays with callers.

mod connection;
mod decode;
mod events;
mod firmware_session;
mod run;
pub mod tx;

use crate::{can_thread, firmware, frame, hil, superdbc, time};

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct DatabaseSource {
    pub path: std::path::PathBuf,
    pub bus_id: u8,
}

/// A file replacement loads on the worker.
/// A bus change reuses its active database.
#[derive(Debug, Clone)]
pub enum DatabaseRequest {
    Load {
        path: std::path::PathBuf,
        preferred_bus: Option<u8>,
    },
    Bus {
        bus_id: u8,
    },
}

/// Successful selection, sent only after the worker applies the new context.
#[derive(Debug)]
pub enum DatabaseUpdate {
    Loaded {
        database: Box<superdbc::database::Database>,
        bus_id: u8,
    },
    Bus {
        bus_id: u8,
    },
}

#[derive(Default)]
pub struct CanThreadConfig {
    pub log_folder: Option<std::path::PathBuf>,
    pub hil_dir: std::path::PathBuf,
}

pub enum CanThreadCommand {
    Connect(Option<crate::connection::ConnectionSource>),
    SelectDatabase {
        request_id: u64,
        selection: DatabaseRequest,
    },
    AddSendMessage(can_thread::tx::AddSendMessage),
    DeleteSendMessage {
        identity: frame::CanIdentity,
    },
    UpdateLogFolder(std::path::PathBuf),
    Stop,
    Hil(hil::engine::HilCommand),
    StartFirmwareUpdate(firmware::protocol::FirmwarePackage),
    ArmFirmwareUpdate(firmware::protocol::FirmwarePackage),
    CancelFirmwareUpdate,
}

pub enum CanThreadEvent {
    DatabaseSelected {
        request_id: u64,
        result: Result<DatabaseUpdate, String>,
    },
    Frame(frame::ParsedFrame),
    SourceSelected(Option<crate::connection::ConnectionSource>),
    Disconnection,
    ConnectionSuccessful,
    ConnectionFailed(String),
    MessageSent {
        identity: frame::CanIdentity,
        timestamp: time::Time,
        amount_left: Option<can_thread::tx::SendAmount>,
    },
    SendFailed {
        identity: frame::CanIdentity,
        error: String,
        retrying: bool,
    },
    Diagnostic(String),
    BusLoad {
        timestamp: time::Time,
        load_1s: f32,
        load_5s: f32,
        load_10s: f32,
        load_30s: f32,
    },
    Hil(hil::engine::HilSnapshot),
    FirmwareProgress(firmware::FirmwareProgress),
}

/// Single caller owns shutdown; cloned senders submit commands but do not own the worker.
pub struct CanThreadHandle {
    tx: std::sync::mpsc::Sender<CanThreadCommand>,
    join: Option<std::thread::JoinHandle<()>>,
}

impl CanThreadHandle {
    pub fn command(
        &self,
        command: CanThreadCommand,
    ) -> Result<(), std::sync::mpsc::SendError<CanThreadCommand>> {
        self.tx.send(command)
    }

    pub fn sender(&self) -> std::sync::mpsc::Sender<CanThreadCommand> {
        self.tx.clone()
    }

    pub fn stop(&mut self) -> std::thread::Result<()> {
        let _ = self.tx.send(CanThreadCommand::Stop);
        match self.join.take() {
            Some(thread) => thread.join(),
            None => Ok(()),
        }
    }
}

impl Drop for CanThreadHandle {
    fn drop(&mut self) {
        let _ = self.stop();
    }
}

pub fn spawn_can_thread(
    config: CanThreadConfig,
    out: std::sync::mpsc::Sender<CanThreadEvent>,
) -> std::io::Result<CanThreadHandle> {
    let (tx, rx) = std::sync::mpsc::channel();
    let join = std::thread::Builder::new()
        .name("daq-can".into())
        .spawn(move || run::run(config, out, rx))?;
    Ok(CanThreadHandle {
        tx,
        join: Some(join),
    })
}
