#include "afe.h"

// Static function prototypes
static void packBalance(uint8_t* data);
static void afeStartComm(void);
static void afeEndComm(void);
static uint16_t calculatePEC(uint8_t* data, uint8_t len);
int verifyPEC(uint8_t* data, uint16_t len, uint16_t PEC);

// @funcname: afeInit
//
// @brief: Sets up AFE for use
void checkConn(void)
{
    uint16_t voltage_low = VUV(1);
	uint16_t voltage_high = VOV(4.2);
	uint8_t cmd[LTC6811_REG_SIZE] = {0b00000101, voltage_low & 0xFF,
                                     (((voltage_high & 0xF) << 4) & 0xF0) |
                                     (((voltage_low >> 8) & 0xF) & 0x0F),
                                     (voltage_high >> 4) & 0xFF, 0, 0};
	uint8_t register_check[LTC6811_REG_SIZE] = {0};

	afeWakeup();
	broadcastWrite(WRCFGA, LTC6811_REG_SIZE, cmd);
	broadcastRead(RDCFGA, LTC6811_REG_SIZE, register_check);

	// Ensure that configuration register is as expected
	for (int i = 0; i < LTC6811_REG_SIZE; i++)
    {
		if (register_check[i] != cmd[i])
        {
            bms.afe_con = 0;
            bms.error |= 1U << E_AFE_CONN;

            return;
        }
    }

    bms.afe_con = 1;
    bms.error &= ~(1U << E_AFE_CONN);

	broadcastPoll(DIAGN);
}

// @funcname: setBalance
//
// @brief: Determines which cells need to balance
//         and marks them as such. Also marks cells
//         with too high/low a voltage with an error
void setBalance(void)
{
    uint8_t  i, ov, uv, req_b, req_m;
    uint16_t min_volts;
    uint16_t balance_set;
    float    avg_SOC = 0;

    ov = uv = 0;
    min_volts = 0xffff;

    for (i = 0; i < bms.cell_count; i++)
    {
        avg_SOC += bms.cells.est_SOC[i];
        min_volts = (bms.cells.chan_volts_raw[i] < min_volts) ? bms.cells.chan_volts_raw[i] : min_volts;
    }

    avg_SOC /= bms.cell_count;

    for (i = 0; i < bms.cell_count; i++)
    {
        req_b = req_m = 0;

        if (bms.op_mode == MODE_CHARGE) {
            if ((bms.cells.chan_volts_raw[i] - MAX_DELTA) > min_volts) {
                req_b = 1;
            }
        }

        if (bms.cells.chan_volts_raw[i] > (uint32_t) (CELL_MAX_V * 10000)) {
            req_b = 1;
            ++ov;
            bms.uv |= 1U << i;
        } else if (bms.cells.chan_volts_raw[i] < (uint32_t) (CELL_MIN_V * 10000)) {
            req_m = 1;
            ++uv;
            bms.ov |= 1U << i;
        } else {
            bms.uv &= ~(1U << i);
            bms.ov &= ~(1U << i);
        }

        if (req_b) {
            bms.cells.balance_flags |= 1U << i;
        } else {
            bms.cells.balance_flags &= ~(1U << i);
        }

        if (req_m) {
            bms.cells.balance_mask |= 1U << i;
        } else {
            bms.cells.balance_mask &= ~(1U << i);
        }
    }

    balance_set = bms.cells.balance_flags & ~(bms.cells.balance_mask);

    for (i = 0; i < bms.cell_count; i++) {
        if (balance_set & (1U << i)) {
            bms.cells.balance_current[i] = (((float) bms.cells.chan_volts_raw[i]) / BAL_RES / 10000) * BAL_DUTY;
        } else {
            bms.cells.balance_current[i] = 0;
        }
    }

    if (ov) {
        bms.error |= 1U << E_OV;
    } else {
        bms.error &= ~(1U << E_OV);
    }

    if (uv) {
        bms.error |= 1U << E_UV;
    } else {
        bms.error &= ~(1U << E_UV);
    }
}

// @funcname: afeTask
//
// @brief: Main AFE task
void afeTask(void)
{
    uint8_t        i, x;
    uint8_t        cmd[LTC6811_REG_SIZE];
    uint8_t        data[8];
    uint8_t        data_ow[8];
    uint16_t       valid_PEC;
    uint16_t       balance_control;
    uint32_t       mod_volts_conv = 0;
    static int8_t  time;
    static uint8_t loop_count;

    afe_state_t        next_state;
    static afe_state_t current_state;

    next_state = current_state;

    switch (current_state)
    {
        // Turn off balancing so we can get a good ADC conversion
        case BAL_OFF:
        {
            bms.no_sleep |= 1U;
            broadcastRead(RDCFGA, LTC6811_REG_SIZE, cmd);
            cmd[4] = 0x00;
            cmd[5] = cmd[4] & ~0xf;
            broadcastWrite(WRCFGA, LTC6811_REG_SIZE, cmd);
            next_state = SETTLE;

            break;
        }

        // Let the voltage settle after disabling balancing (tested and required!)
        case SETTLE:
        {
            if (time == 5) {
                // Skip if OW just ran
                if (loop_count != 1) {
                    next_state = MEAS;
                } else {
                    next_state = BAL;
                }
            }

            break;
        }

        // Take all cell measurements
        case MEAS:
        {
            broadcastPoll(ADCVSC(2, DISCHARGE_NOT_PERMITTED));

            next_state = WAIT;

            break;
        }

        // Wait for ADCVSC to finish (tested and required!)
        case WAIT:
        {
            if (time == 8) {
                next_state = READ;
            }

            break;
        }

        case READ:
        {
            for (i = 0; i < 4; i++)
            {
                valid_PEC = broadcastRead(readCmd[i], LTC6811_REG_SIZE, data);

                x = i * 3;
                bms.cells.chan_volts_raw[x++] = byte_combine(data[1], data[0]);
                bms.cells.chan_volts_raw[x++] = byte_combine(data[3], data[2]);
                bms.cells.chan_volts_raw[x] = byte_combine(data[5], data[4]);
            }

            next_state = BAL;

            break;
        }

        // Enable balancing for cells that need it
        case BAL:
        {
            setBalance();
            balance_control = bms.cells.balance_flags & ~bms.cells.balance_mask;
            #ifdef NO_BALANCE
            balance_control = 0x0000;
            #endif
            broadcastRead(RDCFGA, LTC6811_REG_SIZE, cmd);

            cmd[4] = balance_control & 0xff;
            cmd[5] &= ~0xf;
            cmd[5] = (balance_control >> 8) & 0xf;

            broadcastWrite(WRCFGA, LTC6811_REG_SIZE, cmd);
            next_state = (loop_count == 0) ? OW_PU0 : DIAG;
            loop_count = (loop_count == 9) ? 0 : loop_count + 1;

            break;
        }

        // Run ADOW with pull-ups
        case OW_PU0:
        {
            broadcastPoll(ADOW(3, 1, DISCHARGE_NOT_PERMITTED, ALL_CELLS));
            next_state = OW_PU1;

            break;
        }

        // Run ADOW with pull-ups (again), and read cell voltages
        case OW_PU1:
        {
            broadcastPoll(ADOW(3, 1, DISCHARGE_NOT_PERMITTED, ALL_CELLS));

            for (i = 0; i < 4; i++)
            {
                valid_PEC = broadcastRead(readCmd[i], LTC6811_REG_SIZE, data);

                x = i * 3;
                bms.cells.pu[x++] = byte_combine(data[1], data[0]);
                bms.cells.pu[x++] = byte_combine(data[3], data[2]);
                bms.cells.pu[x] = byte_combine(data[5], data[4]);
            }

            next_state = OW_PD0;

            break;
        }

        // Run ADOWN with pull-downs
        case OW_PD0:
        {
            broadcastPoll(ADOW(3, 0, DISCHARGE_NOT_PERMITTED, ALL_CELLS));
            next_state = OW_PD1;

            break;
        }

        // Run ADOW with pull-downs (again), and read cell voltages
        case OW_PD1:
        {
            broadcastPoll(ADOW(3, 0, DISCHARGE_NOT_PERMITTED, ALL_CELLS));

            for (i = 0; i < 4; i++)
            {
                valid_PEC = broadcastRead(readCmd[i], LTC6811_REG_SIZE, data);

                x = i * 3;
                bms.cells.pd[x++] = byte_combine(data[1], data[0]);
                bms.cells.pd[x++] = byte_combine(data[3], data[2]);
                bms.cells.pd[x] = byte_combine(data[5], data[4]);
            }

            next_state = OW_CALC;

            break;
        }

        // Determine if we have an open wire
        case OW_CALC:
        {
            x = 0;
            broadcastPoll(CLRCELL);

            if (bms.cells.pd[0] == 0) {
                x++;
                bms.ow |= 1U;
            } else {
                bms.ow &= ~1U;
            }

            if (bms.cells.pu[11] == 0 && bms.cell_count == 12) {
                x++;
                bms.ow |= 1U << 12;
            } else {
                bms.ow &= ~(1U << 12);
            }

            for (i = 1; i < bms.cell_count; i++)
            {
                // Note: This is backwards compared to datasheet due to unsigned
                if (bms.cells.pd[i] - bms.cells.pu[i] > OW_THRESH) {
                    x++;
                    bms.ow |= 1U << (i - 1);
                } else {
                    bms.ow &= ~(1U << (i - 1));
                }
            }

            if (x) {
                bms.error |= 1U << E_OW;
            } else {
                bms.error &= ~(1U << E_OW);
            }

            next_state = DIAG;

            break;
        }

        // Run diagnostics to check for AFE errors
        case DIAG:
        {
            broadcastPoll(ADSTAT(2, DISCHARGE_NOT_PERMITTED));
            broadcastPoll(ADAX(2, DISCHARGE_NOT_PERMITTED));
            broadcastRead(RDSTATA, LTC6811_REG_SIZE, data);

            bms.cells.mod_volts_raw = ((float) (byte_combine(data[1], data[0]) + CELL_0_OFFSET)) * 20 / 10000;
            bms.die_temp = (float) byte_combine(data[3], data[2]) * TEMP_CONV - KELVIN_2_CELSIUS;
            bms.afe_vdd = (float) byte_combine(data[5], data[4]) / 10000;

            broadcastRead(RDAUXB, LTC6811_REG_SIZE, data);

            bms.afe_ref = (float) byte_combine(data[5], data[4]) / 10000;
            next_state = HALT;

            break;
        }

        // Halt this loop until the 100ms mark (or pause for sleep)
        case HALT:
        {
            bms.no_sleep &= ~(1U);

            if (time == 99)
            {
                if (bms.sleep_req)
                {
                    broadcastWrite(CLRSCTRL, 0, NULL);

                    return;
                }

                for (i = 0; i < bms.cell_count; i++)
                {
                    mod_volts_conv += bms.cells.chan_volts_raw[i];
                }

                bms.cells.mod_volts_raw = (float) mod_volts_conv / 10000;
                time = -1;
                next_state = BAL_OFF;
            }

            break;
        }

        // Somehow, some way, we got lost. Rectify this
        default:
        {
            next_state = BAL_OFF;
            time = -1;
        }
    }

    current_state = next_state;
    ++time;
}

// @funcname: afeWakeup
//
// @brief: Wakes up AFE
void afeWakeup(void)
{
    afeStartComm();
    afeEndComm();
}

// @funcname: broadcastPoll
//
// @brief: Sends a broadcast message to AFE
//
// @param: command: 16 bit command to send to AFE
void broadcastPoll(uint16_t command)
{
    uint16_t PEC;
    uint8_t  message[4];
    uint8_t  spi_rx_buff[4] = {0};

	message[0] = command >> 8;
	message[1] = command;

	PEC = calculatePEC(message, 2);
	message[2] = PEC >> 8;
	message[3] = PEC;

	afeStartComm();

    PHAL_SPI_transfer(bms.spi, message, 4, spi_rx_buff);
    while (PHAL_SPI_busy(bms.spi));

	afeEndComm();
}

// @funcname: broadcastWrite
//
// @brief: Sends a broadcast message to AFE with data to write
//
// @param: command: 16 bit command to send to AFE
// @param: size: data length to write
// @param: data: pointer to data to write
void broadcastWrite(uint16_t command, uint16_t size, uint8_t* data)
{
    uint16_t PEC;
	uint8_t  message[4 + 6 + 2];
    uint8_t  spi_rx_buff[12] = {0};

	message[0] = command >> 8;
	message[1] = command;
	PEC = calculatePEC(message, 2);
	message[2] = PEC >> 8;
	message[3] = PEC;

	memcpy(&message[4], data, 6);

	PEC = calculatePEC(data, size);
	message[4 + 6 + 0] = PEC >> 8;
	message[4 + 6 + 1] = PEC;

	afeStartComm();

    PHAL_SPI_transfer(bms.spi, message, 12, spi_rx_buff);
    while (PHAL_SPI_busy(bms.spi));

	afeEndComm();

	return;
}

// @funcname: broadcastRead
//
// @brief: Sends a broadcast message to AFE and reads data
//
// @param: command: 16 bit command to send to AFE
// @param: size: data length to write
// @param: data: pointer to data to write
int broadcastRead(uint16_t command, uint16_t size, uint8_t* data)
{

	uint8_t command_message[4];
	uint8_t data_PEC[2];
	uint16_t PEC;
	uint16_t command_PEC;

    static uint8_t spi_rx_buff[16] = {0};
    static uint8_t spi_tx_buff[16] = {0};

	command_message[0] = command >> 8;
	command_message[1] = command;
	command_PEC = calculatePEC(command_message, 2);
	command_message[2] = command_PEC >> 8;
	command_message[3] = command_PEC;

	afeStartComm();

	// Send command
	PHAL_SPI_transfer(bms.spi, command_message, 4, spi_rx_buff);
    while (PHAL_SPI_busy(bms.spi));

    // Receive data
	PHAL_SPI_transfer(bms.spi, spi_tx_buff, size, data);
    while (PHAL_SPI_busy(bms.spi));

	// Receive PEC
	PHAL_SPI_transfer(bms.spi, spi_tx_buff, 4, spi_rx_buff);
    while (PHAL_SPI_busy(bms.spi));

	afeEndComm();

	// Verify PEC
	PEC = spi_rx_buff[0] << 8 | spi_rx_buff[1];
	if (verifyPEC(data, size, PEC) < 0)
    {
		return -1;
	}

	return 0;
}

// @funcname: packBalance
//
// @brief: Converts balance flags to S-pin pulsing
//         commands
//
// @param: data: address of packed data
static void packBalance(uint8_t* data)
{
    uint8_t i;

    for (i = 0; i < 12; i++)
    {
        // Only enable cell balancing if the balance flag is set and we haven't masked the cell
        if ((bms.cells.balance_flags & (1 << i)) && !(bms.cells.balance_mask & (1 << i)))
        {
            data[i / 2] |= S_MAX << ((i % 2) * 4);
        }
    }
}

// @funcname: afeStartComm
//
// @brief: Pulls CSB low to start comms
static void afeStartComm(void)
{
    PHAL_writeGPIO(CSB_AFE_GPIO_Port, CSB_AFE_Pin, 0);
}

// @funcname: afeEndComm
//
// @brief: Pulls CSB high to stop comms
static void afeEndComm(void)
{
    PHAL_writeGPIO(CSB_AFE_GPIO_Port, CSB_AFE_Pin, 1);
}

// @funcname: calculatePEC
//
// @brief: Calculates 16 bit packet error code (PEC)
//
// @param: data: pointer to data
// @param: len: length of data
static uint16_t calculatePEC(uint8_t* data, uint8_t len)
{
    uint8_t i;
    uint16_t address;
    uint16_t remainder = 16;

	for (i = 0; i < len; i++)
    {
		address = ((remainder >> 7) ^ data[i]) & 0xff;
		remainder = (remainder << 8) ^ crc15Table[address];
	}

	return ((remainder * 2) & 0xffff);
}

// @funcname: verifyPEC
//
// @brief: Verifies a PEC
//
// @param: data: pointer to data to check
// @param: len: data length
// @param: PEC: PEC to check against
int verifyPEC(uint8_t* data, uint16_t len, uint16_t PEC)
{

	if (calculatePEC(data, len) == PEC)
    {
		return 0;
	}

	return -1;
}
