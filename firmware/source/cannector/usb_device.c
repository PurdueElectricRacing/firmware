/**
 * @file usb_device.c
 * @brief Periodic CAN-to-USB forwarding; enumeration lives in usb/usb_control.c.
 */
#include "usb_device.h"

#include "common/rtos/rtos.h"
#include "main.h"
#include "usb/usb_control.h"
#include "common/phal_G4/gpio/gpio.h"

static uint32_t g_last_usb_tx_time_ms = 0U;

void usb_tx_periodic(void) {
    timestamped_frame_t packet[MAX_FRAMES_PER_PACKET];
    size_t count = 0U;
    while (count < MAX_FRAMES_PER_PACKET && xQueueReceive(can_queue, &packet[count], 0U) == pdTRUE) {
        count++;
    }
    if (count == 0U) {
        goto exit;
    }
    

    // USB IRQ runs above the RTOS syscall priority
    // PRIMASK excludes it across the configuration check and PMA write
    uint32_t saved_primask = __get_PRIMASK();
    __disable_irq();
    if (usb_data_enabled()) {
        // Best effort: discard this batch if IN is busy
        uint16_t packet_length = (uint16_t)(count * sizeof(packet[0]));
        (void)PHAL_USB_write(PHAL_USB_DATA_ENDPOINT, packet, packet_length);
        g_last_usb_tx_time_ms = xTaskGetTickCount();
    }
    __set_PRIMASK(saved_primask);

exit:
    // Blink the USB LED after TX
    uint32_t usb_tx_age_ms = xTaskGetTickCount() - g_last_usb_tx_time_ms;
    bool usb_tx_recent = usb_tx_age_ms < USB_LED_BLINK_PERIOD_MS;
    PHAL_GPIO_write(USB_LED_PORT, USB_LED_PIN, usb_tx_recent);    

    // Unconfigured/halted batches are discarded
}
