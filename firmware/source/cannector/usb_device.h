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

void usb_tx_periodic(void);

#endif // USB_DEVICE_H
