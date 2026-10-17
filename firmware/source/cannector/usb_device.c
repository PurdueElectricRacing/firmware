/**
 * @file usb_device.c
 * @brief Periodic CAN-to-USB forwarding; enumeration lives in usb/usb_control.c.
 */
#include "usb_device.h"
#include "activity_leds.h"

#include "common/rtos/rtos.h"
#include "main.h"
#include "usb/usb_control.h"

void usb_tx_periodic(void) {
    timestamped_frame_t packet[MAX_FRAMES_PER_PACKET];
    size_t count = 0U;
    while (count < MAX_FRAMES_PER_PACKET && xQueueReceive(can_queue, &packet[count], 0U) == pdTRUE) {
        count++;
    }
    if (count == 0U) {
        // no frames to send
        return;
    }

    // USB IRQ runs above the RTOS syscall priority
    // PRIMASK excludes it across the configuration check and PMA write
    uint32_t saved_primask = __get_PRIMASK();
    __disable_irq();
    if (usb_data_enabled()) {
        // Best effort: discard this batch if IN is busy
        uint16_t packet_length = (uint16_t)(count * sizeof(packet[0]));
        if (PHAL_USB_write(PHAL_USB_DATA_ENDPOINT, packet, packet_length)) {
            activity_leds_record(ACTIVITY_USB, xTaskGetTickCount());
        }
    }
    __set_PRIMASK(saved_primask);

    // Unconfigured/halted batches are discarded
}
