#ifndef BMS_H_
#define BMS_H_

typedef enum
{
	BMS_OK,
	BMS_ERROR,
	BMS_VOLTAGE_ERROR,
	BMS_TEMP_ERROR,
	ADC_ERROR,
	CAN_ERROR,
	GENERIC_ERROR,
} BMS_Status_TypeDef;

#endif 