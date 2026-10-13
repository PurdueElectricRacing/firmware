use crate::{
    ParsedFrame, Time,
    can_thread::{DatabaseRequest, DatabaseUpdate},
    frame::{CanFrame, FrameKind},
    superdbc::database::{Bus, Database},
};

#[derive(Default)]
pub struct FrameDecoder {
    database: Option<Database>,
    bus_id: u8,
}

impl FrameDecoder {
    /// Load and validate file replacements on the worker
    /// Failures leave both the definitions and bus unchanged.
    pub fn select(&mut self, selection: &DatabaseRequest) -> Result<DatabaseUpdate, String> {
        match selection {
            DatabaseRequest::Load {
                path,
                preferred_bus,
            } => {
                let database = Database::load(path).map_err(|error| error.to_string())?;
                let bus_id = preferred_bus
                    .filter(|id| database.bus_by_id(*id).is_some())
                    .or_else(|| database.bus_by_id(0).map(|bus| bus.bus_id()))
                    .or_else(|| database.buses().first().map(|bus| bus.bus_id()))
                    .ok_or_else(|| "SuperDBC contains no selectable buses".to_owned())?;
                let update = DatabaseUpdate::Loaded {
                    database: Box::new(database.clone()),
                    bus_id,
                };
                self.database = Some(database);
                self.bus_id = bus_id;
                Ok(update)
            }
            DatabaseRequest::Bus { bus_id } => {
                let database = self
                    .database
                    .as_ref()
                    .ok_or_else(|| "No SuperDBC database is loaded".to_owned())?;
                if database.bus_by_id(*bus_id).is_none() {
                    return Err(format!("SuperDBC has no bus {bus_id}"));
                }
                self.bus_id = *bus_id;
                Ok(DatabaseUpdate::Bus { bus_id: *bus_id })
            }
        }
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
