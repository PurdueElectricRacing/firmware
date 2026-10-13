use crate::{
    ParsedFrame, Time,
    can_thread::DatabaseSource,
    frame::{CanFrame, FrameKind},
    superdbc::database::{Bus, Database},
};

#[derive(Default)]
pub struct FrameDecoder {
    database: Option<Database>,
    bus_id: u8,
}

impl FrameDecoder {
    /// Validate a replacement completely before discarding the active database.
    pub fn reload(&mut self, source: &DatabaseSource) -> Result<(), String> {
        let database = Database::load(&source.path).map_err(|error| error.to_string())?;
        if database.bus_by_id(source.bus_id).is_none() {
            return Err(format!("SuperDBC has no bus {}", source.bus_id));
        }
        self.database = Some(database);
        self.bus_id = source.bus_id;
        Ok(())
    }

    pub fn bus(&self) -> Option<&Bus> {
        self.database.as_ref()?.bus_by_id(self.bus_id)
    }

    pub fn bus_id(&self) -> u8 {
        self.bus_id
    }

    pub fn decode(&self, frame: CanFrame, timestamp: Time) -> ParsedFrame {
        let decoded = if frame.kind == FrameKind::Data {
            self.bus()
                .and_then(|bus| bus.decode(frame.identity, &frame.data).ok())
        } else {
            None
        };
        ParsedFrame {
            kind: frame.kind,
            dlc: frame.dlc,
            timestamp,
            identity: frame.identity,
            raw_bytes: frame.data,
            decoded,
        }
    }
}
