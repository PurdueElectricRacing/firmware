use std::collections::HashMap;
use std::ffi::CStr;
use std::os::raw::c_char;
use std::path::Path;

mod util {
    pub mod can {
        pub const STANDARD_ID_MASK: u32 = 0x7ff;
    }
}

#[allow(dead_code, clippy::derivable_impls)]
mod connection {
    include!(concat!(
        env!("CARGO_MANIFEST_DIR"),
        "/../../../../../daqapp/src/connection.rs"
    ));
}

#[allow(dead_code, unused_imports)]
mod driver {
    include!(concat!(
        env!("CARGO_MANIFEST_DIR"),
        "/../../../../../daqapp/src/can/driver.rs"
    ));
}

#[repr(C)]
pub struct PerCanFrame {
    id: u32,
    is_extended: u8,
    length: u8,
    data: [u8; 8],
}

fn write_frame(frame: &slcan::CanFrame, output: &mut PerCanFrame) -> bool {
    let slcan::CanFrame::Can2(frame) = frame else {
        return false;
    };
    let (id, is_extended) = match frame.id() {
        slcan::Id::Standard(id) => (id.as_raw() as u32, 0),
        slcan::Id::Extended(id) => (id.as_raw(), 1),
    };
    let Some(data) = frame.data() else {
        return false;
    };
    output.id = id;
    output.is_extended = is_extended;
    output.length = data.len() as u8;
    output.data = [0; 8];
    output.data[..data.len()].copy_from_slice(data);
    true
}

fn parser_from_c_path(path: *const c_char) -> Option<can_decode::Parser> {
    if path.is_null() {
        return None;
    }
    let path = unsafe { CStr::from_ptr(path) }.to_str().ok()?;
    can_decode::Parser::from_dbc_file(Path::new(path)).ok()
}

fn message_id(parser: &can_decode::Parser, name: &str) -> Option<(u32, slcan::Id)> {
    let message = parser
        .msg_defs()
        .into_iter()
        .find(|message| message.name == name)?;
    match message.id {
        can_dbc::MessageId::Standard(id) => {
            let id = slcan::StandardId::new(id)?;
            Some((id.as_raw() as u32, slcan::Id::Standard(id)))
        }
        can_dbc::MessageId::Extended(id) => {
            let id = slcan::ExtendedId::new(id)?;
            Some((id.as_raw() | 0x8000_0000, slcan::Id::Extended(id)))
        }
    }
}

/// Parses one DAQApp UDP frame into the C-compatible test representation.
///
/// # Safety
/// `bytes` must reference `length` readable bytes and `output` must reference writable,
/// properly aligned storage for one `PerCanFrame`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn per_daq_parse_udp_frame(
    bytes: *const u8,
    length: usize,
    output: *mut PerCanFrame,
) -> bool {
    if bytes.is_null() || output.is_null() || length > 2048 {
        return false;
    }
    let input = unsafe { std::slice::from_raw_parts(bytes, length) };
    let mut buffer = [0u8; 2048];
    buffer[..length].copy_from_slice(input);
    let Ok(frames) = driver::parse_udp_buffer(&buffer, length) else {
        return false;
    };
    if frames.len() != 1 {
        return false;
    }
    write_frame(&frames[0], unsafe { &mut *output })
}

/// Decodes a `main_hb` frame using the DBC at `dbc_path`.
///
/// # Safety
/// `dbc_path` must be a valid NUL-terminated string, `frame` must reference a valid
/// `PerCanFrame`, and `car_state` must reference writable, properly aligned `f64` storage.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn per_daq_decode_main_hb(
    dbc_path: *const c_char,
    frame: *const PerCanFrame,
    car_state: *mut f64,
) -> bool {
    if frame.is_null() || car_state.is_null() {
        return false;
    }
    let Some(parser) = parser_from_c_path(dbc_path) else {
        return false;
    };
    let frame = unsafe { &*frame };
    if frame.length > 8 {
        return false;
    }
    let decode_id = frame.id
        | if frame.is_extended != 0 {
            0x8000_0000
        } else {
            0
        };
    let Some(decoded) = parser.decode_msg(decode_id, &frame.data[..frame.length as usize]) else {
        return false;
    };
    if decoded.name != "main_hb" {
        return false;
    }
    let Some(signal) = decoded.signals.get("car_state") else {
        return false;
    };
    unsafe { *car_state = signal.value.physical };
    true
}

/// Encodes and loopbacks a `start_button` frame using the DBC at `dbc_path`.
///
/// # Safety
/// `dbc_path` must be a valid NUL-terminated string and `output` must reference writable,
/// properly aligned storage for one `PerCanFrame`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn per_daq_encode_start_button(
    dbc_path: *const c_char,
    pressed: bool,
    output: *mut PerCanFrame,
) -> bool {
    if output.is_null() {
        return false;
    }
    let Some(parser) = parser_from_c_path(dbc_path) else {
        return false;
    };
    let Some((decode_id, slcan_id)) = message_id(&parser, "start_button") else {
        return false;
    };
    let values = HashMap::from([("is_pressed".to_string(), if pressed { 1.0 } else { 0.0 })]);
    let Some(data) = parser.encode_msg(decode_id, &values) else {
        return false;
    };
    let Some(frame) = slcan::Can2Frame::new_data(slcan_id, &data) else {
        return false;
    };

    let Ok(mut loopback) = driver::create_driver(&connection::ConnectionSource::Loopback) else {
        return false;
    };
    if loopback.write_frame(frame.into()).is_err() {
        return false;
    }
    let Ok(frames) = loopback.read_frames() else {
        return false;
    };
    frames.len() == 1 && write_frame(&frames[0], unsafe { &mut *output })
}
