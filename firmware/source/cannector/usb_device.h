/**
 * @file usb_device.h
 * @brief USB tx thread
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#include <stddef.h>
#include <stdint.h>

#include "common/phal_G4/usb/usb.h"
#include "common/timestamped_frame/timestamped_frame.h"


static constexpr size_t MAX_FRAMES_PER_PACKET = 4U;
static_assert(MAX_FRAMES_PER_PACKET * sizeof(timestamped_frame_t) == PHAL_USB_PACKET_SIZE_BYTES);


typedef struct {
    volatile bool configured;       // The host has selected configuration 1 and completed its acknowledgement
    volatile bool bulk_halted;      // The host has halted the CAN data endpoint
    uint8_t address;                // The USB address assigned by the host
    uint16_t pending_address;       // An address change waiting for acknowledgement
    uint16_t pending_configuration; // A configuration change waiting for acknowledgement
} usb_context_t;

void usb_tx_periodic(void);

#endif // USB_DEVICE_H