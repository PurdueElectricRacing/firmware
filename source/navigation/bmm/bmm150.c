/**
 * @file bmm150.c
 * @author Christopher McGalliard (cmcgalli@purdue.edu)
 * @brief
 * @version 0.3
 * @date 2023-02-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "bmm150.h"
#include "main.h"
#include "bmi088.h"
#include "common/phal_L4/spi/spi.h"
#include "common_defs.h"
#include "stdbool.h"
#include "can_parse.h"
#include "SFS_pp.h"

static inline void BMM150_selectMag(BMM150_Handle_t *bmm);
bool BMM150_readID(BMM150_Handle_t *bmm);
void BMM150_powerOnMag(BMM150_Handle_t *bmm);
bool BMM150_init(BMM150_Handle_t *bmm);
bool BMM150_readMag(BMM150_Handle_t *bmm, ExtU *rtU);
void BMM150_setActive(BMM150_Handle_t *bmm);
bool BMM150_selfTest(BMM150_Handle_t *bmm);
bool BMM150_selfTestAdvanced(BMM150_Handle_t *bmm);
int16_t raw_x, raw_y, raw_z;
int16_t new_xa;
int16_t new_xb;
int16_t new_ya;
int16_t new_yb;
int16_t new_za;
int16_t new_zb;
int16_t result_value_x;
int16_t result_value_y;
int16_t result_value_z;
int16_t main_result;
int16_t diff;
extern q_handle_t q_tx_can;

bool BMM150_readMag(BMM150_Handle_t *bmm, ExtU *rtU)
{
    // SPI rx and tx buffers
    static uint8_t spi_rx_buff[16] = {0};
    static uint8_t spi_tx_buff[16] = {0};

    // Prepare tx buffer with x axis LSB address
    spi_tx_buff[0] = (1 << 7) | BMM150_MAG_X_LSB_ADDR;

    // SPI nss for BMM
    BMM150_selectMag(bmm);

    // Wait until spi is available
    while (PHAL_SPI_busy(bmm->spi))
        ;

    // Complete transfer
    PHAL_SPI_transfer_noDMA(bmm->spi, spi_tx_buff, 1, 6, spi_rx_buff);
    // while (PHAL_SPI_busy(bmm->spi))
    //     ;

    // Parse x axis mag
    new_xa = (spi_rx_buff[1] >> 3) & 0x001f;
    new_xb = (spi_rx_buff[2] << 8) & 0xff00;
    result_value_x = new_xa | (new_xb >> 3);

    // Parse y axis mag
    new_ya = (spi_rx_buff[3] >> 3) & 0x001f;
    new_yb = (spi_rx_buff[4] << 8) & 0xff00;
    result_value_y = new_ya | (new_yb >> 3);

    // Parse z axis mag
    new_za = (spi_rx_buff[5] >> 1) & 0x7f;
    new_zb = (spi_rx_buff[6] << 8) & 0xff00;
    result_value_z = new_za | new_zb;

    // Main result (needs to be sqrt)
    main_result = (result_value_x * result_value_x) + (result_value_y * result_value_y) + (result_value_y * result_value_y);

    rtU->mag[0] = CLAMP(((double)result_value_x) * MAG_CALIBRATION, MIN_MAG, MAX_MAG);
    rtU->mag[1] = CLAMP(((double)result_value_y) * MAG_CALIBRATION, MIN_MAG, MAX_MAG);
    rtU->mag[2] = CLAMP(((double)result_value_z) * MAG_CALIBRATION, MIN_MAG, MAX_MAG);

    SEND_BMM_MAG(q_tx_can, result_value_x, result_value_y, result_value_z);
    return true;
}

bool BMM150_selfTest(BMM150_Handle_t *bmm)
{
    BMM150_selectMag(bmm);
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b00000110);
    if (!(PHAL_SPI_readByte(bmm->spi, 0x42, true) && 0b00000001))
    {
        return false;
    }
    if (!(PHAL_SPI_readByte(bmm->spi, 0x44, true) && 0b00000001))
    {
        return false;
    }
    if (!(PHAL_SPI_readByte(bmm->spi, 0x46, true) && 0b00000001))
    {
        return false;
    }
    BMM150_setActive(bmm);
    return true;
}

bool BMM150_selfTestAdvanced(BMM150_Handle_t *bmm)
{
    BMM150_selectMag(bmm);
    // 1. Set sleep mode
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b00000110);
    // 2. Disable X, Y axis
    PHAL_SPI_writeByte(bmm->spi, 0x4E, 0b00011111);
    // 3. Set Z repetitions to desired level
    PHAL_SPI_writeByte(bmm->spi, 0x52, 0x04);
    // 4. Enable positive advanced self test current
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b11000110);
    // 5. Set forced mode, readout Z and R channel after measurement is finished
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b11000010);
    uint8_t z_byte_1 = PHAL_SPI_readByte(bmm->spi, 0x46, true);
    uint8_t z_byte_2 = PHAL_SPI_readByte(bmm->spi, 0x47, true);
    int16_t pos_z_res = ((z_byte_1 >> 1) & 0x7f) | ((z_byte_2 << 8) & 0xff00);
    // enable no current
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b00000010);
    // 6. Enable negative advanced self test current
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b10000010);
    // 7. Set forced mode, readout Z and R channel after measurement is finished
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b10000010);
    uint8_t z_byte_3 = PHAL_SPI_readByte(bmm->spi, 0x46, true);
    uint8_t z_byte_4 = PHAL_SPI_readByte(bmm->spi, 0x47, true);
    int16_t neg_z_res = ((z_byte_3 >> 1) & 0x7f) | ((z_byte_4 << 8) & 0xff00);
    // 8. Disable advanced self test current(this must be done manually)
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b00000010);
    // 9. Calculate difference between the two compensated field values.This difference should be around 200 μT with some margins.10. Perform a soft reset of manually restore desired settings
    diff = pos_z_res - neg_z_res;
    BMM150_setActive(bmm);
    return true;
}

void BMM150_powerOnMag(BMM150_Handle_t *bmm)
{
    BMM150_selectMag(bmm);
    PHAL_SPI_writeByte(bmm->spi, 0x4b, 0b00000001);
    BMM150_setActive(bmm);
    return;
}

void BMM150_setActive(BMM150_Handle_t *bmm)
{
    BMM150_selectMag(bmm);
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b00000000);
    return;
}
uint8_t testresult = -2;
bool BMM150_readID(BMM150_Handle_t *bmm)
{
    BMM150_selectMag(bmm);
    PHAL_SPI_writeByte(bmm->spi, 0x4b, 0b00000001);
    PHAL_SPI_writeByte(bmm->spi, BMM150_OP_MODE_ADDR, 0b00111000);
    if (PHAL_SPI_readByte(bmm->spi, BMM150_CHIP_ID_ADDR, true) != BMM150_CHIP_ID)
    {
        // PHAL_writeGPIO(SPI_CS_MAG_GPIO_Port, SPI_CS_MAG_Pin, 1);
        return false;
    }
    // PHAL_writeGPIO(SPI_CS_MAG_GPIO_Port, SPI_CS_MAG_Pin, 1);
    return true;
}

static inline void BMM150_selectMag(BMM150_Handle_t *bmm)
{
    // PHAL_writeGPIO(SPI_CS_ACEL_GPIO_Port, SPI_CS_ACEL_Pin, 1);
    // PHAL_writeGPIO(SPI_CS_GYRO_GPIO_Port, SPI_CS_GYRO_Pin, 1);
    bmm->spi->nss_gpio_port = bmm->mag_csb_gpio_port;
    bmm->spi->nss_gpio_pin = bmm->mag_csb_pin;
}