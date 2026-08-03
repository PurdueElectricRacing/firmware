/**
 * @file usb.h
 * @brief STM32G4 USB full-speed device API.
 * @author Ronak Jain (jain717@purdue.edu)
 *
 * The HAL provides one control endpoint and one bulk data endpoint. Applications
 * supply USB descriptors, control-request handling, and data queues through the
 * callback and packet-transfer functions below.
 */

#ifndef PHAL_G4_USB_H
#define PHAL_G4_USB_H

#include <stdint.h>

/** Endpoint zero for enumeration and control transfers. */
static constexpr uint8_t PHAL_USB_CONTROL_ENDPOINT = 0U;

/** Endpoint one for aggregate bulk application data. */
static constexpr uint8_t PHAL_USB_DATA_ENDPOINT = 1U;

/** Maximum packet size for both endpoints. */
static constexpr uint16_t PHAL_USB_PACKET_SIZE_BYTES = 64U;

/** USB-standard transfer direction, named from the host perspective. */
typedef enum : uint8_t {
    PHAL_USB_ENDPOINT_DIRECTION_OUT = 0U, /**< Host to device. */
    PHAL_USB_ENDPOINT_DIRECTION_IN = 1U, /**< Device to host. */
} PHAL_USB_EndpointDirection_t;

/** Details present when a USB event represents an endpoint transfer. */
typedef struct {
    uint8_t endpoint; /**< Endpoint associated with the transfer. */
    PHAL_USB_EndpointDirection_t direction; /**< Direction of the transfer. */
    bool setup; /**< The OUT transfer contains an endpoint-zero SETUP packet. */
} PHAL_USB_TransferEvent_t;

/** USB bus reset or endpoint-transfer event delivered in interrupt context. */
typedef struct {
    bool transfer; /**< True for a transfer; false for a bus reset. */
    union {
        PHAL_USB_TransferEvent_t transfer_data; /**< Valid when transfer is true. */
    };
} PHAL_USB_Event_t;

/**
 * @brief Handle USB events.
 *
 * Applications define this callback to process bus resets, SETUP packets, OUT
 * packets, and IN completions. Check event->transfer before accessing
 * transfer_data. The HAL provides a weak empty implementation. OUT data remains
 * available through PHAL_USB_read(). An IN completion marks the endpoint ready
 * for PHAL_USB_write().
 *
 * @param event USB event for the current interrupt.
 */
void PHAL_USB_callback(const PHAL_USB_Event_t *event);

/**
 * @brief Initialize the USB device hardware and endpoints.
 *
 * Initialization enables HSI48, USB, UCPD1, PA8/PA9 USB-C sink signaling,
 * PA11/PA12 USB data signaling, USB packet memory, endpoint zero as control,
 * endpoint one as bulk, and USB interrupts. The device remains detached until
 * PHAL_USB_connect() enables the D+ pull-up.
 *
 * @return true when initialization completes; false when USB is active or the
 *         48 MHz clock startup times out.
 */
bool PHAL_USB_init(void);

/**
 * @brief Detach and disable the USB peripheral.
 *
 * @return true when deinitialization completes; false when USB is inactive.
 */
bool PHAL_USB_deinit(void);

/**
 * @brief Attach the initialized USB device to the host.
 *
 * @return true when the D+ pull-up is enabled; false when USB is inactive.
 */
bool PHAL_USB_connect(void);

/**
 * @brief Apply the address assigned by a SET_ADDRESS request.
 *
 * Apply the address after the control endpoint reports completion of the IN
 * status packet.
 *
 * @param address Device address from 0 through 127.
 * @return true when the address is applied; false for an inactive peripheral
 *         or an address above 127.
 */
bool PHAL_USB_setAddress(uint8_t address);

/**
 * @brief Stall one endpoint direction.
 *
 * @param endpoint PHAL_USB_CONTROL_ENDPOINT or PHAL_USB_DATA_ENDPOINT.
 * @param direction Endpoint direction.
 * @return true when the endpoint is stalled; false when a precondition fails.
 */
bool PHAL_USB_stall(uint8_t endpoint, PHAL_USB_EndpointDirection_t direction);

/**
 * @brief Queue one device-to-host packet.
 *
 * The HAL copies exactly length_bytes into USB packet memory, where
 * length_bytes may be zero through PHAL_USB_PACKET_SIZE_BYTES. A zero-length
 * packet accepts a null source. The endpoint is ready for its next packet when
 * the IN completion callback runs.
 *
 * @param endpoint PHAL_USB_CONTROL_ENDPOINT or PHAL_USB_DATA_ENDPOINT.
 * @param source Packet bytes, or null for a zero-length packet.
 * @param length_bytes Packet length from 0 through 64.
 * @return true when the packet is queued; false when a precondition fails or
 *         the endpoint is busy.
 */
bool PHAL_USB_write(uint8_t endpoint, const void *source, uint16_t length_bytes);

/**
 * @brief Consume one host-to-device packet.
 *
 * A successful read copies the packet and rearms reception. An undersized
 * destination leaves the packet pending and reports its length through
 * received_length. A zero-length packet accepts a null destination.
 *
 * @param endpoint PHAL_USB_CONTROL_ENDPOINT or PHAL_USB_DATA_ENDPOINT.
 * @param destination Packet destination, or null for zero capacity.
 * @param destination_capacity Destination capacity in bytes.
 * @param received_length Receives the packet length.
 * @return true when the packet is consumed; false when a precondition fails,
 *         reception is pending, or the destination capacity is insufficient.
 */
bool PHAL_USB_read(uint8_t endpoint,
                   void *destination,
                   uint16_t destination_capacity,
                   uint16_t *received_length);

/**
 * @brief Read the USB peripheral's current 11-bit frame number.
 *
 * The value is maintained by the peripheral from Start-of-Frame packets and
 * wraps after 2047.
 *
 * @return The current frame number.
 */
uint16_t PHAL_USB_getFrameNumber(void);

#endif // PHAL_G4_USB_H
