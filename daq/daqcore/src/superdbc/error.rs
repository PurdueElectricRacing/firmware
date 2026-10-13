//! Operation-specific errors: loading/parsing, decoding, and encoding.

use crate::superdbc;

/// Reading a file or parsing its contents failed.
#[derive(Debug)]
pub enum LoadError {
    Io(std::io::Error),
    Parse(ParseError),
}

/// The document could not be deserialized or compiled into valid definitions.
#[derive(Debug)]
pub enum ParseError {
    InvalidJson(serde_json::Error),
    UnsupportedSchemaVersion { found: String },
    InvalidDefinition { context: String, reason: String },
}

impl ParseError {
    pub fn definition(context: impl Into<String>, reason: impl Into<String>) -> Self {
        Self::InvalidDefinition {
            context: context.into(),
            reason: reason.into(),
        }
    }
}

/// Runtime decoding only fails at lookup or payload length validation.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DecodeError {
    UnknownBus {
        bus_id: u8,
    },
    UnknownMessage {
        key: superdbc::message::MessageKey,
    },
    InvalidPayloadLength {
        key: superdbc::message::MessageKey,
        expected_min: u8,
        actual: usize,
    },
}

/// Invalid encoder inputs; values are never silently clamped or defaulted.
#[derive(Debug)]
pub enum EncodeError {
    UnknownBus {
        bus_id: u8,
    },
    UnknownMessage {
        key: superdbc::message::MessageKey,
    },
    MissingSignal {
        key: superdbc::message::MessageKey,
        signal: String,
    },
    UnknownSignal {
        key: superdbc::message::MessageKey,
        signal: String,
    },
    DuplicateSignal {
        key: superdbc::message::MessageKey,
        signal: String,
    },
    TypeMismatch {
        key: superdbc::message::MessageKey,
        signal: String,
        expected: superdbc::message::RawType,
    },
    NonFiniteValue {
        key: superdbc::message::MessageKey,
        signal: String,
    },
    Float32Overflow {
        key: superdbc::message::MessageKey,
        signal: String,
    },
    PhysicalOutOfRange {
        key: superdbc::message::MessageKey,
        signal: String,
        physical: f64,
    },
    RawOutOfRange {
        key: superdbc::message::MessageKey,
        signal: String,
        value: i128,
        min: i128,
        max: i128,
    },
}

impl std::fmt::Display for superdbc::message::MessageKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "bus {} / {}", self.bus_id, self.identity)
    }
}

impl std::fmt::Display for LoadError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Io(error) => write!(f, "cannot read SuperDBC: {error}"),
            Self::Parse(error) => error.fmt(f),
        }
    }
}

impl std::fmt::Display for ParseError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::InvalidJson(error) => write!(f, "invalid SuperDBC JSON: {error}"),
            Self::UnsupportedSchemaVersion { found } => write!(
                f,
                "unsupported SuperDBC schema version {found:?}; expected 1.1"
            ),
            Self::InvalidDefinition { context, reason } => {
                write!(f, "invalid SuperDBC definition at {context}: {reason}")
            }
        }
    }
}

impl std::fmt::Display for DecodeError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::UnknownBus { bus_id } => write!(f, "unknown CAN bus {bus_id}"),
            Self::UnknownMessage { key } => write!(f, "unknown CAN message {key}"),
            Self::InvalidPayloadLength {
                key,
                expected_min,
                actual,
            } => write!(
                f,
                "invalid payload length {actual} for {key}; expected {expected_min}..=8 bytes"
            ),
        }
    }
}

impl std::fmt::Display for EncodeError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::UnknownBus { bus_id } => write!(f, "unknown CAN bus {bus_id}"),
            Self::UnknownMessage { key } => write!(f, "unknown CAN message {key}"),
            Self::MissingSignal { key, signal } => write!(f, "missing signal {signal:?} for {key}"),
            Self::UnknownSignal { key, signal } => write!(f, "unknown signal {signal:?} for {key}"),
            Self::DuplicateSignal { key, signal } => {
                write!(f, "duplicate signal {signal:?} for {key}")
            }
            Self::TypeMismatch {
                key,
                signal,
                expected,
            } => write!(f, "signal {signal:?} for {key} requires raw {expected:?}"),
            Self::NonFiniteValue { key, signal } => {
                write!(f, "non-finite value for signal {signal:?} of {key}")
            }
            Self::Float32Overflow { key, signal } => {
                write!(f, "Float32 overflow for signal {signal:?} of {key}")
            }
            Self::PhysicalOutOfRange {
                key,
                signal,
                physical,
            } => write!(
                f,
                "physical value {physical} is outside the wire range for signal {signal:?} of {key}"
            ),
            Self::RawOutOfRange {
                key,
                signal,
                value,
                min,
                max,
            } => write!(
                f,
                "raw value {value} is outside {min}..={max} for signal {signal:?} of {key}"
            ),
        }
    }
}

impl std::error::Error for LoadError {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        Some(match self {
            Self::Io(error) => error,
            Self::Parse(error) => error,
        })
    }
}

impl std::error::Error for ParseError {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        match self {
            Self::InvalidJson(error) => Some(error),
            _ => None,
        }
    }
}

impl std::error::Error for DecodeError {}
impl std::error::Error for EncodeError {}

impl From<std::io::Error> for LoadError {
    fn from(error: std::io::Error) -> Self {
        Self::Io(error)
    }
}

impl From<ParseError> for LoadError {
    fn from(error: ParseError) -> Self {
        Self::Parse(error)
    }
}

impl From<serde_json::Error> for ParseError {
    fn from(error: serde_json::Error) -> Self {
        Self::InvalidJson(error)
    }
}
