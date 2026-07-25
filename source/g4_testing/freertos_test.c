#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_FREERTOS)

#include <stdint.h>

#include "common/freertos/freertos.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/utils/countof.h"
#include "main.h"


GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_OUTPUT_LOW_SPEED),
};

static constexpr uint32_t TargetCoreClockrateHz = 16'000'000;
ClockRateConfig_t clock_config = {
    .clock_source              = CLOCK_SOURCE_HSI,
    .use_pll                   = false,
    .vco_output_rate_target_hz = 16'000'000,
    .system_clock_target_hz    = TargetCoreClockrateHz,
    .ahb_clock_target_hz       = TargetCoreClockrateHz,
    .apb1_clock_target_hz      = TargetCoreClockrateHz,
    .apb2_clock_target_hz      = TargetCoreClockrateHz,
};

static volatile uint32_t produced       = 0;
static volatile uint32_t consumed       = 0;
static volatile uint32_t shared_counter = 0;
static volatile uint32_t monitor_count  = 0;

static void startup_task(void);
static void producer_task(void);
static void consumer_task(void);
static void monitor_task(void);

static void fail(void);
void HardFault_Handler(void);

DEFINE_TASK(startup_task, 1000, TASK_PRIORITY_HIGH,   STACK_512);
DEFINE_TASK(producer_task, 100, TASK_PRIORITY_NORMAL, STACK_512);
DEFINE_TASK(consumer_task, 100, TASK_PRIORITY_NORMAL, STACK_512);
DEFINE_TASK(monitor_task,  500, TASK_PRIORITY_LOW,    STACK_512);

DEFINE_QUEUE(message_queue, uint32_t, 16);

DEFINE_MUTEX(counter_mutex);
DEFINE_BINARY_SEMAPHORE(start_sem);
DEFINE_COUNTING_SEMAPHORE(work_sem);


int main() {
    if (PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }

    if (!PHAL_initGPIO(gpio_config, countof(gpio_config))) {
        HardFault_Handler();
    }

    INIT_QUEUE(message_queue);

    INIT_MUTEX(counter_mutex);
    INIT_BINARY_SEMAPHORE(start_sem);
    INIT_COUNTING_SEMAPHORE(work_sem, 16);

    START_TASK(startup_task);
    START_TASK(producer_task);
    START_TASK(consumer_task);
    START_TASK(monitor_task);

    vTaskStartScheduler();

    // Unreachable
    HardFault_Handler();

    return 0;
}


static void startup_task(void) {
    freertos_delay_ms(1000);

    xSemaphoreGive(start_sem);

    vTaskDelete(NULL);
}


static void producer_task(void) {
    static bool started = false;

    if (!started) {
        xSemaphoreTake(start_sem, portMAX_DELAY);
        started = true;
    }

    produced++;

    uint32_t value = produced;

    if (xQueueSend(message_queue, &value, 0) != pdPASS) {
        fail();
    }

    xSemaphoreGive(work_sem);

    PHAL_toggleGPIO(LED_GREEN_PORT, LED_GREEN_PIN);
}


static void consumer_task(void) {
    uint32_t value;

    if (xSemaphoreTake(work_sem, 0) != pdPASS) {
        return;
    }

    if (xQueueReceive(message_queue, &value, 0) != pdPASS) {
        fail();
    }

    xSemaphoreTake(counter_mutex, portMAX_DELAY);

    shared_counter = value;
    consumed++;

    xSemaphoreGive(counter_mutex);

    PHAL_toggleGPIO(LED_GREEN_PORT, LED_GREEN_PIN);
}


static void monitor_task(void) {
    xSemaphoreTake(counter_mutex, portMAX_DELAY);

    monitor_count++;

    bool ok =
        (produced >= consumed) &&
        (shared_counter == consumed);

    xSemaphoreGive(counter_mutex);

    if (!ok) {
        fail();
    }

    PHAL_toggleGPIO(LED_GREEN_PORT, LED_GREEN_PIN);
}


static void fail(void) {
    while (true) {
        PHAL_writeGPIO(LED_GREEN_PORT, LED_GREEN_PIN, 1);
        freertos_delay_ms(100);
    }
}

void HardFault_Handler(void) {
    while (true) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_FREERTOS