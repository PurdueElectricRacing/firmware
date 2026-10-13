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

// USB 2.0, vendor-specific device, no strings, one configuration.
static const uint8_t DEVICE_DESCRIPTOR[] = {
    PHAL_USB_DEVICE_DESCRIPTOR_LENGTH, PHAL_USB_DESCRIPTOR_DEVICE, 0x00U, 0x02U, PHAL_USB_DEVICE_CLASS_VENDOR_SPECIFIC, 0U, 0U, PHAL_USB_PACKET_SIZE_BYTES,
    0x09U, 0x12U, 0x03U, 0x00U, 0x01U, 0x00U, 0U, 0U, 0U, 1U,
};
static const uint8_t CONFIGURATION_DESCRIPTOR[] = {
    PHAL_USB_CONFIGURATION_DESCRIPTOR_LENGTH, PHAL_USB_DESCRIPTOR_CONFIGURATION, 25U, 0U, 1U, 1U, 0U, PHAL_USB_CONFIGURATION_ATTRIBUTES_BUS_POWERED, 50U,
    PHAL_USB_INTERFACE_DESCRIPTOR_LENGTH, PHAL_USB_DESCRIPTOR_INTERFACE, 0U, 0U, 1U, PHAL_USB_DEVICE_CLASS_VENDOR_SPECIFIC, 0U, 0U, 0U,
    PHAL_USB_ENDPOINT_DESCRIPTOR_LENGTH, PHAL_USB_DESCRIPTOR_ENDPOINT, BULK_IN_ADDRESS, PHAL_USB_ENDPOINT_TRANSFER_TYPE_BULK, PHAL_USB_PACKET_SIZE_BYTES, 0U, 0U,
};
static_assert(sizeof(DEVICE_DESCRIPTOR) == PHAL_USB_DEVICE_DESCRIPTOR_LENGTH);
static_assert(sizeof(CONFIGURATION_DESCRIPTOR) == 25U);

static usb_context_t g_usb_context = {
    .configured = false,
    .bulk_halted = false,
    .address = 0U,
    .pending_address = NO_PENDING_VALUE,
    .pending_configuration = NO_PENDING_VALUE,
};

static void stall_control(void) {
    g_usb_context.pending_address = NO_PENDING_VALUE;
    g_usb_context.pending_configuration = NO_PENDING_VALUE;
    PHAL_USB_stall(PHAL_USB_CONTROL_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_IN);
    PHAL_USB_stall(PHAL_USB_CONTROL_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_OUT);
}

static void respond(const void *data, uint16_t size, uint16_t requested) {
    if (!PHAL_USB_write(PHAL_USB_CONTROL_ENDPOINT, data, size < requested ? size : requested)) {
        stall_control();
    }
}

static bool valid_endpoint(uint16_t index) {
    return g_usb_context.address != 0U && (index == 0U || index == PHAL_USB_ENDPOINT_ADDRESS_IN
        || (g_usb_context.configured && index == BULK_IN_ADDRESS));
}

static void handle_setup(const PHAL_USB_SetupPacket_t *setup) {
    switch (setup->request) {
        case PHAL_USB_REQUEST_GET_DESCRIPTOR: // GET_DESCRIPTOR: only device and configuration index zero.
            if (setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_IN && setup->index == 0U && setup->length != 0U) {
                if (setup->value == ((uint16_t)PHAL_USB_DESCRIPTOR_DEVICE << 8U)) {
                    respond(DEVICE_DESCRIPTOR, sizeof(DEVICE_DESCRIPTOR), setup->length);
                    return;
                }
                if (setup->value == ((uint16_t)PHAL_USB_DESCRIPTOR_CONFIGURATION << 8U)) {
                    respond(CONFIGURATION_DESCRIPTOR, sizeof(CONFIGURATION_DESCRIPTOR), setup->length);
                    return;
                }
            }
            break;
        case PHAL_USB_REQUEST_SET_ADDRESS: // SET_ADDRESS: apply only after the IN status completion.
            if (setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_OUT && setup->index == 0U && setup->length == 0U
                && setup->value <= 127U && !g_usb_context.configured) {
                g_usb_context.pending_address = setup->value;
                respond(nullptr, 0U, 0U);
                return;
            }
            break;
        case PHAL_USB_REQUEST_GET_CONFIGURATION: // GET_CONFIGURATION
            if (setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_IN && setup->value == 0U && setup->index == 0U
                && setup->length == 1U && g_usb_context.address != 0U) {
                uint8_t value = g_usb_context.configured ? 1U : 0U;
                respond(&value, sizeof(value), setup->length);
                return;
            }
            break;
        case PHAL_USB_REQUEST_SET_CONFIGURATION: // SET_CONFIGURATION
            if (setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_OUT && setup->index == 0U && setup->length == 0U
                && setup->value <= 1U && g_usb_context.address != 0U) {
                // Stop submissions until the configuration status stage completes.
                g_usb_context.configured = false;
                g_usb_context.pending_configuration = setup->value;
                respond(nullptr, 0U, 0U);
                return;
            }
            break;
        case PHAL_USB_REQUEST_GET_STATUS: { // GET_STATUS: no remote wakeup or self-powered capability.
            if (setup->value != 0U || setup->length != 2U) {
                break;
            }
            uint8_t status[2] = {0U, 0U};
            if ((setup->type == PHAL_USB_REQUEST_TYPE_DEVICE_IN && setup->index == 0U && g_usb_context.address != 0U)
                || (setup->type == PHAL_USB_REQUEST_TYPE_INTERFACE_IN && g_usb_context.configured && setup->index == 0U)
                || (setup->type == PHAL_USB_REQUEST_TYPE_ENDPOINT_IN && valid_endpoint(setup->index))) {
                if (setup->type == PHAL_USB_REQUEST_TYPE_ENDPOINT_IN && setup->index == BULK_IN_ADDRESS) {
                    status[0] = g_usb_context.bulk_halted ? 1U : 0U;
                }
                respond(status, sizeof(status), setup->length);
                return;
            }
            break;
        }
        case PHAL_USB_REQUEST_GET_INTERFACE: // GET_INTERFACE: interface zero, alternate setting zero.
            if (setup->type == PHAL_USB_REQUEST_TYPE_INTERFACE_IN && g_usb_context.configured && setup->index == 0U
                && setup->value == 0U && setup->length == 1U) {
                uint8_t alternate = 0U;
                respond(&alternate, sizeof(alternate), setup->length);
                return;
            }
            break;
        case PHAL_USB_REQUEST_SET_INTERFACE: // SET_INTERFACE resets the bulk pipe to DATA0.
            if (setup->type == PHAL_USB_REQUEST_TYPE_INTERFACE_OUT && g_usb_context.configured && setup->index == 0U
                && setup->value == 0U && setup->length == 0U) {
                PHAL_USB_clearHalt(PHAL_USB_DATA_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_IN);
                g_usb_context.bulk_halted = false;
                respond(nullptr, 0U, 0U);
                return;
            }
            break;
        case PHAL_USB_REQUEST_CLEAR_FEATURE: // CLEAR_FEATURE(ENDPOINT_HALT)
        case PHAL_USB_REQUEST_SET_FEATURE: // SET_FEATURE(ENDPOINT_HALT)
            if (setup->type == PHAL_USB_REQUEST_TYPE_ENDPOINT_OUT && g_usb_context.configured && setup->index == BULK_IN_ADDRESS
                && setup->value == PHAL_USB_FEATURE_ENDPOINT_HALT && setup->length == 0U) {
                g_usb_context.bulk_halted = setup->request == PHAL_USB_REQUEST_SET_FEATURE;
                if (g_usb_context.bulk_halted) {
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
        g_usb_context.configured = false;
        g_usb_context.bulk_halted = false;
        g_usb_context.address = 0U;
        g_usb_context.pending_address = NO_PENDING_VALUE;
        g_usb_context.pending_configuration = NO_PENDING_VALUE;
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
            g_usb_context.pending_address = NO_PENDING_VALUE;
            g_usb_context.pending_configuration = NO_PENDING_VALUE;
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
        if (g_usb_context.pending_address != NO_PENDING_VALUE) {
            g_usb_context.address = (uint8_t)g_usb_context.pending_address;
            PHAL_USB_setAddress(g_usb_context.address);
            g_usb_context.pending_address = NO_PENDING_VALUE;
        }
        if (g_usb_context.pending_configuration != NO_PENDING_VALUE) {
            PHAL_USB_clearHalt(PHAL_USB_DATA_ENDPOINT, PHAL_USB_ENDPOINT_DIRECTION_IN);
            g_usb_context.bulk_halted = false;
            g_usb_context.configured = g_usb_context.pending_configuration == 1U;
            g_usb_context.pending_configuration = NO_PENDING_VALUE;
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
    if (g_usb_context.configured && !g_usb_context.bulk_halted) {
        // Best effort: discard this batch if IN is busy. Success only means
        // copied to PMA, not delivered. No host/reader means no completion.
        (void)PHAL_USB_write(PHAL_USB_DATA_ENDPOINT, packet, (uint16_t)(count * sizeof(packet[0])));
    }
    __set_PRIMASK(saved_primask);
    // Unconfigured/halted batches are also discarded to avoid a stale backlog.
}
