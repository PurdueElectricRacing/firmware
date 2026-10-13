/**
 * @file usb_device.c
 * @brief USB device setup/callbacks + USB tx thread
 */
#include "usb_device.h"

#include <string.h>

#include "common/phal_G4/usb/usb.h"
#include "common/rtos/rtos.h"
#include "common/timestamped_frame/timestamped_frame.h"
#include "main.h"

static constexpr uint8_t BULK_IN_ADDRESS = PHAL_USB_ENDPOINT_ADDRESS_IN | PHAL_USB_DATA_ENDPOINT;
static constexpr uint16_t NO_PENDING_VALUE = UINT16_MAX;
static constexpr size_t MAX_FRAMES_PER_PACKET = 4U;

// USB 2.0, vendor-specific device, no strings, one configuration.
static const uint8_t device_descriptor[] = {
    PHAL_USB_DEVICE_DESCRIPTOR_LENGTH, PHAL_USB_DESCRIPTOR_DEVICE, 0x00U, 0x02U, PHAL_USB_DEVICE_CLASS_VENDOR_SPECIFIC, 0U, 0U, PHAL_USB_PACKET_SIZE_BYTES,
    0x09U, 0x12U, 0x03U, 0x00U, 0x01U, 0x00U, 0U, 0U, 0U, 1U,
};
static const uint8_t configuration_descriptor[] = {
    PHAL_USB_CONFIGURATION_DESCRIPTOR_LENGTH, PHAL_USB_DESCRIPTOR_CONFIGURATION, 25U, 0U, 1U, 1U, 0U, PHAL_USB_CONFIGURATION_ATTRIBUTES_BUS_POWERED, 50U,
    PHAL_USB_INTERFACE_DESCRIPTOR_LENGTH, PHAL_USB_DESCRIPTOR_INTERFACE, 0U, 0U, 1U, PHAL_USB_DEVICE_CLASS_VENDOR_SPECIFIC, 0U, 0U, 0U,
    PHAL_USB_ENDPOINT_DESCRIPTOR_LENGTH, PHAL_USB_DESCRIPTOR_ENDPOINT, BULK_IN_ADDRESS, PHAL_USB_ENDPOINT_TRANSFER_TYPE_BULK, PHAL_USB_PACKET_SIZE_BYTES, 0U, 0U,
};
static_assert(sizeof(device_descriptor) == PHAL_USB_DEVICE_DESCRIPTOR_LENGTH);
static_assert(sizeof(configuration_descriptor) == 25U);
static_assert(MAX_FRAMES_PER_PACKET * sizeof(timestamped_frame_t) == PHAL_USB_PACKET_SIZE_BYTES);

static volatile bool configured = false;
static volatile bool bulk_halted = false;
static uint8_t address = 0U;
static uint16_t pending_address = NO_PENDING_VALUE;
static uint16_t pending_configuration = NO_PENDING_VALUE;

static void stall_control(void) {
    pending_address = NO_PENDING_VALUE;
    pending_configuration = NO_PENDING_VALUE;
    PHAL_USB_stall(PHAL_USB_CONTROL_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_IN);
    PHAL_USB_stall(PHAL_USB_CONTROL_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_OUT);
}

static void respond(const void *data, uint16_t size, uint16_t requested) {
    if (!PHAL_USB_write(PHAL_USB_CONTROL_ENDPOINT, data, size < requested ? size : requested)) {
        stall_control();
    }
}

static bool valid_endpoint(uint16_t index) {
    return address != 0U && (index == 0U || index == PHAL_USB_ENDPOINT_ADDRESS_IN
        || (configured && index == BULK_IN_ADDRESS));
}

static void handle_setup(const PHAL_USB_SetupPacket_t *setup) {
    switch (setup->request) {
        case PHAL_USB_REQUEST_GET_DESCRIPTOR: // GET_DESCRIPTOR: only device and configuration index zero.
            if (setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_IN && setup->index == 0U && setup->length != 0U) {
                if (setup->value == ((uint16_t)PHAL_USB_DESCRIPTOR_DEVICE << 8U)) {
                    respond(device_descriptor, sizeof(device_descriptor), setup->length);
                    return;
                }
                if (setup->value == ((uint16_t)PHAL_USB_DESCRIPTOR_CONFIGURATION << 8U)) {
                    respond(configuration_descriptor, sizeof(configuration_descriptor), setup->length);
                    return;
                }
            }
            break;
        case PHAL_USB_REQUEST_SET_ADDRESS: // SET_ADDRESS: apply only after the IN status completion.
            if (setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_OUT && setup->index == 0U && setup->length == 0U
                && setup->value <= 127U && !configured) {
                pending_address = setup->value;
                respond(nullptr, 0U, 0U);
                return;
            }
            break;
        case PHAL_USB_REQUEST_GET_CONFIGURATION: // GET_CONFIGURATION
            if (setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_IN && setup->value == 0U && setup->index == 0U
                && setup->length == 1U && address != 0U) {
                uint8_t value = configured ? 1U : 0U;
                respond(&value, sizeof(value), setup->length);
                return;
            }
            break;
        case PHAL_USB_REQUEST_SET_CONFIGURATION: // SET_CONFIGURATION
            if (setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_OUT && setup->index == 0U && setup->length == 0U
                && setup->value <= 1U && address != 0U) {
                // Stop submissions until the configuration status stage completes.
                configured = false;
                pending_configuration = setup->value;
                respond(nullptr, 0U, 0U);
                return;
            }
            break;
        case PHAL_USB_REQUEST_GET_STATUS: { // GET_STATUS: no remote wakeup or self-powered capability.
            if (setup->value != 0U || setup->length != 2U) {
                break;
            }
            uint8_t status[2] = {0U, 0U};
            if ((setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_IN && setup->index == 0U && address != 0U)
                || (setup->type == PHAL_USB_REQUEST_TYPE_INTERFACE_IN && configured && setup->index == 0U)
                || (setup->type == PHAL_USB_REQUEST_TYPE_ENDPOINT_IN && valid_endpoint(setup->index))) {
                if (setup->type == PHAL_USB_REQUEST_TYPE_ENDPOINT_IN && setup->index == BULK_IN_ADDRESS) {
                    status[0] = bulk_halted ? 1U : 0U;
                }
                respond(status, sizeof(status), setup->length);
                return;
            }
            break;
        }
        case PHAL_USB_REQUEST_GET_INTERFACE: // GET_INTERFACE: interface zero, alternate setting zero.
            if (setup->type == PHAL_USB_REQUEST_TYPE_INTERFACE_IN && configured && setup->index == 0U
                && setup->value == 0U && setup->length == 1U) {
                uint8_t alternate = 0U;
                respond(&alternate, sizeof(alternate), setup->length);
                return;
            }
            break;
        case PHAL_USB_REQUEST_SET_INTERFACE: // SET_INTERFACE resets the bulk pipe to DATA0.
            if (setup->type == PHAL_USB_REQUEST_TYPE_INTERFACE_OUT && configured && setup->index == 0U
                && setup->value == 0U && setup->length == 0U) {
                PHAL_USB_clearHalt(PHAL_USB_DATA_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_IN);
                bulk_halted = false;
                respond(nullptr, 0U, 0U);
                return;
            }
            break;
        case PHAL_USB_REQUEST_CLEAR_FEATURE: // CLEAR_FEATURE(ENDPOINT_HALT)
        case PHAL_USB_REQUEST_SET_FEATURE: // SET_FEATURE(ENDPOINT_HALT)
            if (setup->type == PHAL_USB_REQUEST_TYPE_ENDPOINT_OUT && configured && setup->index == BULK_IN_ADDRESS
                && setup->value == PHAL_USB_FEATURE_ENDPOINT_HALT && setup->length == 0U) {
                bulk_halted = setup->request == PHAL_USB_REQUEST_SET_FEATURE;
                if (bulk_halted) {
                    PHAL_USB_stall(PHAL_USB_DATA_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_IN);
                } else {
                    PHAL_USB_clearHalt(PHAL_USB_DATA_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_IN);
                }
                respond(nullptr, 0U, 0U);
                return;
            }
            break;
        default:
            break;
    }
    stall_control();
}

void PHAL_USB_callback(const PHAL_USB_Event_t *event) {
    if (event == nullptr) {
        return;
    }
    if (!event->transfer) {
        configured = false;
        bulk_halted = false;
        address = 0U;
        pending_address = NO_PENDING_VALUE;
        pending_configuration = NO_PENDING_VALUE;
        return;
    }
    if (event->transfer_data.endpoint != PHAL_USB_CONTROL_ENDPOINT) {
        return; // Bulk IN completions are retired by the HAL.
    }
    if (event->transfer_data.direction == PHAL_USB_ENDPOINT_DIRECTION_OUT) {
        uint8_t packet[PHAL_USB_PACKET_SIZE_BYTES];
        uint16_t length = 0U;
        if (event->transfer_data.setup) {
            // A new SETUP aborts the preceding control request.
            pending_address = NO_PENDING_VALUE;
            pending_configuration = NO_PENDING_VALUE;
        }
        if (!PHAL_USB_read(PHAL_USB_CONTROL_ENDPOINT, packet, sizeof(packet), &length)) {
            stall_control();
        } else if (event->transfer_data.setup) {
            if (length == sizeof(PHAL_USB_SetupPacket_t)) {
                PHAL_USB_SetupPacket_t setup;
                memcpy(&setup, packet, sizeof(setup));
                handle_setup(&setup);
            } else {
                stall_control();
            }
        } else if (length != 0U) {
            stall_control(); // Control-IN status must be an empty OUT packet.
        }
    } else {
        if (pending_address != NO_PENDING_VALUE) {
            address = (uint8_t)pending_address;
            PHAL_USB_setAddress(address);
            pending_address = NO_PENDING_VALUE;
        }
        if (pending_configuration != NO_PENDING_VALUE) {
            PHAL_USB_clearHalt(PHAL_USB_DATA_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_IN);
            bulk_halted = false;
            configured = pending_configuration == 1U;
            pending_configuration = NO_PENDING_VALUE;
        }
    }
}

void usb_tx_periodic(void) {
    timestamped_frame_t packet[MAX_FRAMES_PER_PACKET];
    size_t count = 0U;
    while (count < MAX_FRAMES_PER_PACKET && xQueueReceive(can_queue, &packet[count], 0U) == pdTRUE) {
        count++;
    }
    if (count == 0U) {
        return;
    }

    // USB IRQ runs above the RTOS syscall priority. PRIMASK excludes it across
    // the configuration check and PMA write; RTOS critical sections would not.
    uint32_t saved_primask = __get_PRIMASK();
    __disable_irq();
    if (configured && !bulk_halted) {
        // Best effort: discard this batch if IN is busy. Success only means
        // copied to PMA, not delivered. No host/reader means no completion.
        (void)PHAL_USB_write(PHAL_USB_DATA_ENDPOINT, packet, (uint16_t)(count * sizeof(packet[0])));
    }
    __set_PRIMASK(saved_primask);
    // Unconfigured/halted batches are also discarded to avoid a stale backlog.
}
