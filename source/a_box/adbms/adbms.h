/**
 * @file adbms.h
 * @brief Primary logic and interface for ADBMS battery management system driver.
 *
 * @author Millan Kumar (kumar798@purdue.edu)
 */

#ifndef _BMS_H_
#define _BMS_H_

#include <stddef.h>
#include <stdint.h>

#include "adbms6380.h"
#include "common/phal/spi.h"
#include "common/strbuf/strbuf.h"

#define ADBMS_MODULE_COUNT (8)

// Max SPI TX is a command + all the data packets for all the modules
#define ADBMS_SPI_TX_BUFFER_SIZE \
    (ADBMS6380_COMMAND_PKT_SIZE + (ADBMS_MODULE_COUNT * ADBMS6380_SINGLE_DATA_PKT_SIZE))
// MAX SPI RX is bounded by reading all cell voltages from all modules
#define ADBMS_SPI_RX_BUFFER_SIZE (ADBMS6380_RDCVALL_DATA_PKT_SIZE * ADBMS_MODULE_COUNT)

#define ADBMS_REFON        (true)  // REGA
#define ADBMS_CTH          (0b110) // REGA - 25.05 mV
#define ADBMS_OV_THRESHOLD (4.2f)  // REGB - in volts
#define ADBMS_UV_THRESHOLD (3.0f)  // REGB - in volts
#define ADBMS_RD           (false) // ADCV
#define ADBMS_CONT         (true)  // ADCV/ADSV
#define ADBMS_DCP          (false) // ADCV/ADSV
#define ADBMS_RSTF         (true)  // ADCV
#define ADBMS_OW           (0b00)  // ADCV/ADSV

/**
 * @brief ADBMS driver connection/operation state.
 */
typedef enum {
    /** Driver is idle; not connected or awaiting re-connection. */
    ADBMS_STATE_IDLE = 0,
    /** Driver is connected; periodic reads/balancing are active. */
    ADBMS_STATE_CONNECTED,
} ADBMS_state_t;

/**
 * @brief Per-ADBMS module measurements, config, and error flags.
 */
typedef struct {
    /** Latest cell voltage readings for this module (volts). */
    float cell_voltages[ADBMS6380_CELL_COUNT];
    /** Latest raw cell voltage readings for this module. */
    int16_t cell_voltages_raw[ADBMS6380_CELL_COUNT];
    /** Minimum cell voltage within this module (volts). */
    float min_voltage;
    /** Maximum cell voltage within this module (volts). */
    float max_voltage;
    /** Average cell voltage within this module (volts). */
    float avg_voltage;
    /** Sum of cell voltages within this module (volts). */
    float sum_voltage;
    /** Latest thermistor readings mapped to GPIOs (degrees Celsius). */
    float thermistors[ADBMS6380_GPIO_COUNT];
    //! TODO: do we also want min/max/avg for thermistors?
    /** Per-cell discharge enable flags used for balancing. Set by BMS, not higher level logic. */
    bool is_discharging[ADBMS6380_CELL_COUNT];

    /** Cached REGA bytes written to the device. Used to compare against read-back data. */
    uint8_t rega[ADBMS6380_SINGLE_DATA_RAW_SIZE];
    /** Cached REGB bytes written to the device. Used to compare against read-back data. */
    uint8_t regb[ADBMS6380_SINGLE_DATA_RAW_SIZE];

    /** Set if a read-back REGA does not match cached REGA. */
    bool err_rega_mismatch;
    /** Set if a read-back REGB does not match cached REGB. */
    bool err_regb_mismatch;
} ADBMS_module_t;

/**
 * @brief Top-level ADBMS driver state, I/O buffers, and aggregated measurements.
 */
typedef struct {
    /** Current driver state. */
    ADBMS_state_t state;

    /** Per-module state and measurements. */
    ADBMS_module_t modules[ADBMS_MODULE_COUNT];

    /** Minimum cell voltage across all modules (volts). */
    float min_voltage;
    /** Maximum cell voltage across all modules (volts). */
    float max_voltage;
    /** Average cell voltage across all modules (volts). */
    float avg_voltage;
    /** Sum of cell voltages across all modules (volts). */
    float sum_voltage;

    /** True if cell discharge balancing is permitted by higher-level logic. */
    bool is_discharge_enabled;

    /** SPI instance used for ADBMS communication (CS control is manual by ADBMS6380 driver). */
    SPI_InitConfig_t *spi;
    /** Scratch TX buffer wrapper for command/data packets. */
    strbuf_t tx_strbuf;
    /** RX buffer for multi-module readouts. */
    uint8_t rx_buf[ADBMS_SPI_RX_BUFFER_SIZE];

    /** Set on any SPI transfer/read failure. Should be a terminal error. */
    bool err_spi;
    /** Set when connect/initialization sequence fails. Retry logic up to higher-level logic. */
    bool err_connect;
    /** Aggregated REGA mismatch flag across modules. */
    bool err_rega_mismatch;
    /** Aggregated REGB mismatch flag across modules. */
    bool err_regb_mismatch;
} ADBMS_bms_t;

/**
 * @brief Initialize the ADBMS driver instance and TX buffer.
 *
 * Sets the driver state to idle, clears discharge flags, and sets the
 * provided SPI config and TX buffer into the internal strbuf.
 *
 * @param bms Pointer to driver state to initialize.
 * @param spi SPI configuration used for ADBMS transactions.
 * @param tx_buf Backing buffer for TX command/data packets.
 */
void adbms_init(ADBMS_bms_t *bms, SPI_InitConfig_t *spi, uint8_t *tx_buf);

/**
 * @brief Calculate and write REGA configuration to all modules.
 *
 * @param bms Pointer to driver state.
 * @return True on successful SPI transfer, false on failure.
 */
bool adbms_write_rega(ADBMS_bms_t *bms);
/**
 * @brief Calculate and write REGB configuration to all modules.
 *
 * Computes REGB with OV/UV thresholds and discharge flags, then writes
 * the configuration to all modules.
 *
 * @param bms Pointer to driver state.
 * @return True on successful SPI transfer, false on failure.
 */
bool adbms_write_regb(ADBMS_bms_t *bms);
/**
 * @brief Read back REGA and compare with cached configuration.
 *
 * Sets per-module and aggregated mismatch flags when the read-back does
 * not match the cached REGA data.
 *
 * @param bms Pointer to driver state.
 * @return False on SPI failure and when there is a mismatch; true otherwise.
 */
bool adbms_read_and_check_rega(ADBMS_bms_t *bms);
/**
 * @brief Read back REGB and compare with cached configuration.
 *
 * Sets per-module and aggregated mismatch flags when the read-back does
 * not match the cached REGB data.
 *
 * @param bms Pointer to driver state.
 * @return False on SPI failure and when there is a mismatch; true otherwise.
 */
bool adbms_read_and_check_regb(ADBMS_bms_t *bms);

/**
 * @brief Perform the connect/bring-up sequence for all modules.
 *
 * Writes REGA/REGB, verifies read-back, and starts ADCV
 * and ADSV conversions.
 *
 * @param bms Pointer to driver state.
 */
void adbms_connect(ADBMS_bms_t *bms);

/**
 * @brief Read all cell voltages and update module/pack statistics.
 *
 * Updates per-module min/max/avg/sum and aggregated min/max/avg/sum for
 * the full pack.
 *
 * @param bms Pointer to driver state.
 */
void adbms_read_cells(ADBMS_bms_t *bms);
/**
 * @brief Read all GPIO/thermistor voltages.
 *
 * @param bms Pointer to driver state.
 */
void adbms_read_therms(ADBMS_bms_t *bms);

/**
 * @brief Compute per-cell discharge flags based on pack voltage spread.
 *
 * If balancing is disabled, clears all discharge flags. If the pack
 * minimum voltage is below @p min_voltage, no balancing occurs. Otherwise,
 * discharges cells above (min + delta).
 *
 * @param bms Pointer to driver state.
 * @param min_voltage Minimum pack voltage required to allow balancing.
 * @param min_delta Voltage delta above minimum to start discharging.
 */
void adbms_calculate_balance_cells(ADBMS_bms_t *bms, float min_voltage, float min_delta);
/**
 * @brief Compute discharge flags and write REGB to apply balancing.
 *
 * Calls adbms_calculate_balance_cells(), writes REGB, and verifies it.
 *
 * @param bms Pointer to driver state.
 * @param min_voltage Minimum pack voltage required to allow balancing.
 * @param min_delta Voltage delta above minimum to start discharging.
 */
void adbms_balance_and_update_regb(ADBMS_bms_t *bms, float min_voltage, float min_delta);

/**
 * @brief Periodic service routine for connection, measurements, and balancing.
 *
 * If idle, attempts to connect. When connected, reads cells and thermistors,
 * then updates balancing + REGB.
 *
 * @param bms Pointer to driver state.
 * @param min_voltage_for_balance Minimum pack voltage required to allow balancing.
 * @param min_delta_for_balance Voltage delta above minimum to start discharging.
 */
void adbms_periodic(ADBMS_bms_t *bms, float min_voltage_for_balance, float min_delta_for_balance);

#endif // _BMS_H_