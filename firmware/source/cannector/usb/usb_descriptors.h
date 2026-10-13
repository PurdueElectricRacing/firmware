/**
 * @file usb_descriptors.h
 * @brief CANnector's USB identity, describe the device to the host.
 *
 * Descriptors are byte arrays in the USB wire format.
 * Multibyte values are little endian.
 */
#ifndef CANNECTOR_USB_DESCRIPTORS_H
#define CANNECTOR_USB_DESCRIPTORS_H

#include "common/phal_G4/usb/usb.h"

// IN is named from the host's perspective: board -> laptop, endpoint 1
static constexpr uint8_t BULK_IN_ADDRESS = PHAL_USB_ENDPOINT_ADDRESS_IN | PHAL_USB_DATA_ENDPOINT;

/**
 * Device identity:
 * 1. USB version: 2.0.
 * 2. Device class: vendor-specific (the daqcore driver knows our format)
 * 3. Control packet size: 64 bytes.
 * 4. Vendor/product IDs: 1209:0003
 * 5. Device version: 0.01.
 * 6. No manufacturer, product, or serial-number strings.
 * 7. One available configuration.
 */
static const uint8_t DEVICE_DESCRIPTOR[] = {
    PHAL_USB_DEVICE_DESCRIPTOR_LENGTH,
    PHAL_USB_DESCRIPTOR_DEVICE,
    0x00U, 0x02U,                         // bcdUSB: USB 2.0
    PHAL_USB_DEVICE_CLASS_VENDOR_SPECIFIC,
    0U,                                  // Device subclass
    0U,                                  // Device protocol
    PHAL_USB_PACKET_SIZE_BYTES,          // EP0 maximum packet size
    0x09U, 0x12U,                        // Vendor ID: 0x1209
    0x03U, 0x00U,                        // Product ID: 0x0003
    0x01U, 0x00U,                        // bcdDevice: 0.01
    0U,                                  // No manufacturer string
    0U,                                  // No product string
    0U,                                  // No serial-number string
    1U,                                  // Number of configurations
};

/**
 * Configuration 1 is the CAN-to-USB bridge.
 * Configuration 0 disables it.
 * One interface (0), with one alternate setting (0), advertises one bulk IN endpoint (0x81).
 */
static const uint8_t CONFIGURATION_DESCRIPTOR[] = {
    // Configuration descriptor: 9 bytes
    PHAL_USB_CONFIGURATION_DESCRIPTOR_LENGTH,
    PHAL_USB_DESCRIPTOR_CONFIGURATION,
    25U, 0U,                             // Total length: 9 + 9 + 7 bytes
    1U,                                  // Number of interfaces
    1U,                                  // Configuration value selected by host
    0U,                                  // No configuration string
    PHAL_USB_CONFIGURATION_ATTRIBUTES_BUS_POWERED,
    50U,                                 // Maximum power: 50 * 2 mA = 100 mA

    // Interface descriptor: 9 bytes
    // One function with no alternate modes
    PHAL_USB_INTERFACE_DESCRIPTOR_LENGTH,
    PHAL_USB_DESCRIPTOR_INTERFACE,
    0U,                                  // Interface number
    0U,                                  // Alternate setting
    1U,                                  // Number of endpoints (excluding EP0)
    PHAL_USB_DEVICE_CLASS_VENDOR_SPECIFIC,
    0U,                                  // Interface subclass
    0U,                                  // Interface protocol
    0U,                                  // No interface string

    // Endpoint descriptor: 7 bytes
    PHAL_USB_ENDPOINT_DESCRIPTOR_LENGTH,
    PHAL_USB_DESCRIPTOR_ENDPOINT,
    BULK_IN_ADDRESS,
    PHAL_USB_ENDPOINT_TRANSFER_TYPE_BULK,
    PHAL_USB_PACKET_SIZE_BYTES, 0U,      // Maximum packet size: 64 bytes
    0U,                                  // Polling interval unused for bulk IN
};

static_assert(sizeof(DEVICE_DESCRIPTOR) == PHAL_USB_DEVICE_DESCRIPTOR_LENGTH);
static_assert(sizeof(CONFIGURATION_DESCRIPTOR) == 25U);

#endif
