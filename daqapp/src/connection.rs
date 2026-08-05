#[derive(serde::Serialize, serde::Deserialize, Clone, Debug, PartialEq)]
pub enum ConnectionSource {
    Serial(String, CanBusSpeed),
    Udp(u16),
    Simulated(bool, Option<std::path::PathBuf>), // true for connected, false for disconnected, path to dbc file for sim
    Loopback,
}

#[derive(serde::Serialize, serde::Deserialize, Copy, Clone, PartialEq, Debug)]

pub enum CanBusSpeed {
    Kbps250,
    Kbps500,
}

impl ConnectionSource {
    pub fn display_name(&self) -> String {
        match self {
            ConnectionSource::Serial(path, speed) => {
                format!("Serial: {} ({})", path, speed.display_name())
            }
            ConnectionSource::Udp(port) => format!("UDP: {}", port),
            ConnectionSource::Simulated(connected, _) => {
                if *connected {
                    "Simulated (connected)".into()
                } else {
                    "Simulated (disconnected)".into()
                }
            }
            ConnectionSource::Loopback => "Loopback".into(),
        }
    }
}

impl CanBusSpeed {
    pub fn display_name(&self) -> String {
        match self {
            CanBusSpeed::Kbps250 => "250k".into(),
            CanBusSpeed::Kbps500 => "500k".into(),
        }
    }

    pub fn to_slcan_bitrate(self) -> slcan::NominalBitRate {
        match self {
            CanBusSpeed::Kbps250 => slcan::NominalBitRate::Rate250Kbit,
            CanBusSpeed::Kbps500 => slcan::NominalBitRate::Rate500Kbit,
        }
    }

    pub fn to_bps(self) -> u32 {
        match self {
            CanBusSpeed::Kbps250 => 250_000,
            CanBusSpeed::Kbps500 => 500_000,
        }
    }

    pub fn options() -> Vec<CanBusSpeed> {
        vec![CanBusSpeed::Kbps250, CanBusSpeed::Kbps500]
    }
}

impl Default for CanBusSpeed {
    fn default() -> Self {
        CanBusSpeed::Kbps500
    }
}
