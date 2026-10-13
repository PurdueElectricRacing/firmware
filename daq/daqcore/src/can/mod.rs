pub const EXTENDED_ID_FLAG: u32 = 0x80000000;
pub const STANDARD_ID_MASK: u32 = 0x7FF;
pub const EXTENDED_ID_MASK: u32 = 0x1FFFFFFF;

pub mod bus_load;
pub mod driver;
pub mod logger;
