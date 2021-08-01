#include "pedals.h"
#include <string.h>

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

static pedals_positions_S pedal_positions;

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

// None

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

ADC_HandleTypeDef* pedal_adc_group;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

// None

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void pedals_Init(ADC_HandleTypeDef* hadc)
{
    pedal_positions.t1 = 0;
    pedal_positions.t2 = 0;
    pedal_positions.b1 = 0;
    pedal_positions.b2 = 0;

    pedal_adc_group = hadc;
}

void pedals_DoADC()
{
    pedal_positions.t1 = 0.5;
    pedal_positions.t2 = 0.4;
    pedal_positions.b1 = 0.3;
    pedal_positions.b2 = 0.2;

    HAL_ADC_PollForConversion(pedal_adc_group, HAL_MAX_DELAY);
}

void pedals_GetPositions(pedals_positions_S* positions)
{
    memcpy(&pedal_positions, positions, sizeof(pedals_positions_S));
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/