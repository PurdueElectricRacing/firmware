//! Host-side validation for the STM32G4 vendor bulk-echo example.
//!
//! The test finds the example by VID/PID, claims its only interface, and sends
//! sequential packets through bulk OUT endpoint 0x01. Each packet must return
//! unchanged through bulk IN endpoint 0x81 before the next packet is sent.

use std::{error::Error, time::Duration};

use rusb::{Context, DeviceHandle, UsbContext};

const VENDOR_ID: u16 = 0x1209;
const PRODUCT_ID: u16 = 0x0003;
const INTERFACE: u8 = 0;
const BULK_OUT_ENDPOINT: u8 = 0x01;
const BULK_IN_ENDPOINT: u8 = 0x81;
const MAX_PACKET_LENGTH: usize = 64;
const TRANSFER_TIMEOUT: Duration = Duration::from_secs(1);
// Exercise odd/even copies and lengths immediately around common power-of-two
// boundaries, including the full-speed endpoint's maximum packet length.
const TEST_PACKET_LENGTHS: [usize; 10] = [1, 2, 7, 8, 15, 16, 31, 32, 63, 64];
const ITERATIONS_PER_LENGTH: u32 = 100;

/// Find the example, select its configuration, and claim its vendor interface.
fn open_device(context: &Context) -> Result<DeviceHandle<Context>, Box<dyn Error>> {
    for device in context.devices()?.iter() {
        let descriptor = device.device_descriptor()?;
        if descriptor.vendor_id() != VENDOR_ID || descriptor.product_id() != PRODUCT_ID {
            continue;
        }

        let handle = device.open()?;
        // Auto-detach matters only if the host associated a kernel driver with
        // this vendor interface. Unsupported platforms may safely ignore it.
        handle.set_auto_detach_kernel_driver(true).ok();
        handle.set_active_configuration(1)?;
        handle.claim_interface(INTERFACE)?;
        println!(
            "Connected on bus {} address {}",
            device.bus_number(),
            device.address()
        );
        return Ok(handle);
    }

    Err(format!("USB device {VENDOR_ID:04x}:{PRODUCT_ID:04x} was not found").into())
}

/// Generate deterministic data that changes across both bytes and iterations.
fn test_packet(sequence: u32, length: usize) -> Vec<u8> {
    (0..length)
        .map(|index| sequence.wrapping_mul(31).wrapping_add(index as u32) as u8)
        .collect()
}

/// Send one complete packet and verify its matching echo before returning.
fn echo_packet(handle: &DeviceHandle<Context>, packet: &[u8]) -> Result<(), Box<dyn Error>> {
    // Sequential write/read operation mirrors the firmware's intentionally
    // minimal one-packet application flow; this is not a throughput benchmark.
    let written = handle.write_bulk(BULK_OUT_ENDPOINT, packet, TRANSFER_TIMEOUT)?;
    if written != packet.len() {
        return Err(format!("short write: {written}/{} bytes", packet.len()).into());
    }

    // Give libusb the endpoint's full capacity, then validate both the returned
    // transfer length and every byte. This catches truncation as well as damage.
    let mut echoed = [0u8; MAX_PACKET_LENGTH];
    let read = handle.read_bulk(BULK_IN_ENDPOINT, &mut echoed, TRANSFER_TIMEOUT)?;
    if read != packet.len() {
        return Err(format!("short read: {read}/{} bytes", packet.len()).into());
    }
    if echoed[..read] != packet[..] {
        return Err("echoed packet did not match the transmitted packet".into());
    }

    Ok(())
}

fn main() -> Result<(), Box<dyn Error>> {
    let context = Context::new()?;
    let handle = open_device(&context)?;

    // Repeating every boundary length catches endpoint-rearm and stale-buffer
    // failures that a single successful transfer could miss.
    for length in TEST_PACKET_LENGTHS {
        for sequence in 0..ITERATIONS_PER_LENGTH {
            echo_packet(&handle, &test_packet(sequence, length))?;
        }
        println!("Passed {ITERATIONS_PER_LENGTH} {length}-byte packets");
    }

    println!("STM32G4 USB echo test passed");
    Ok(())
}
