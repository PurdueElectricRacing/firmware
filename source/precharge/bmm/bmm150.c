/**
 * @file bmi.c
 * @author Adam Busch (busch8@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2022-02-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "bmm150.h"
#include "bsxlite_interface.h"
#include "common/phal_L4/spi/spi.h"
#include "common_defs.h"

static inline void BMI088_selectGyro(BMI088_Handle_t *bmi);
static inline void BMI088_selectAccel(BMI088_Handle_t *bmi);

bool BMM150_init(BMM150_Handle_t *bmm)
{
    bmm->mag_ready = false;

    /* Mag initilization  */
    BMM150_selectMag(bmm);
    if (PHAL_SPI_readByte(bmm->spi, BMM150_MAG_CHIP_ID_ADDR, true) != BMM150_MAG_CHIP_ID)
        return false;

    PHAL_SPI_writeByte(bmm->spi, BMM150_MAG_RANGE_ADDR, bmm->mag_datarate);

    // Perform self tests for sensor
    BMM150_magSelfTestStart(bmi);
    while (!BMM150_magSelfTestComplete(bmi))
        ;

    if (!BMI088_magSelfTestPass(bmi))
        return false;

    return true;
}

// unsure which one to use
int8_t bmm150_init(struct bmm150_dev *dev)
{
    int8_t rslt;
    uint8_t chip_id = 0;

    /* Power up the sensor from suspend to sleep mode */
    rslt = set_power_control_bit(BMM150_POWER_CNTRL_ENABLE, dev);

    if (rslt == BMM150_OK)
    {
        /* Start-up time delay of 3ms */
        dev->delay_us(BMM150_START_UP_TIME, dev->intf_ptr);

        /* Chip ID of the sensor is read */
        rslt = bmm150_get_regs(BMM150_REG_CHIP_ID, &chip_id, 1, dev);

        /* Proceed if everything is fine until now */
        if (rslt == BMM150_OK)
        {
            /* Check for chip id validity */
            if (chip_id == BMM150_CHIP_ID)
            {
                dev->chip_id = chip_id;

                /* Function to update trim values */
                rslt = read_trim_registers(dev);
            }
        }
    }

    return rslt;
}

static inline void BMI088_selectMag(BMM150_Handle_t *bmi)
{
    bmm->spi->nss_gpio_port = bmm->mag_csb_gpio_port;
    bmi->spi->nss_gpio_pin = bmm->mag_csb_pin;
}