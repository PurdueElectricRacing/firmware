#include <CAN.h>
#include <math.h>

// ADC MUX Pins
#define ADC_MUX_ADDR_0_PIN (23)
#define ADC_MUX_ADDR_1_PIN (25)
#define ADC_MUX_ADDR_2_PIN (27)
#define ADC_MUX_ADDR_3_PIN (29)

#define ADC_MUX_SIG_0_PIN (A12)
#define ADC_MUX_SIG_1_PIN (A10)
#define ADC_MUX_SIG_2_PIN (A11)
#define ADC_MUX_SIG_3_PIN (A9)

#define CAN_MESSAGE_ID_BASE (0xBE0)
#define MCP2515_CS_PIN (9)

// Hardware constants

// Pull-up resistor values
#define R_REF_0   9600.
#define R_REF_1   10000.
#define R_REF_2   10000.
#define R_REF_3   10000.

#define ADC_REF 5.
#define VCC     5.
#define ADC_RES 1024.
#define R25     10000.

// Parameters of the thermocouple:
// R_rel = R_t/R_25
// 1/T = A + B * ln(R_rel) + C * ln(R_rel) ^ 2 + D * ln(R_rel) ^ 3
// Low temperatures:
#define A_LOW   0.003354016
#define B_LOW   0.000256173
#define C_LOW   2.13941e-06
#define D_LOW   -7.25325e-08
// Medium temperatures
#define A_MEDIUM   0.003353045
#define B_MEDIUM   0.000254200
#define C_MEDIUM   1.14261e-06
#define D_MEDIUM   -6.93803e-08
// High temperatures
#define A_HIGH   0.003353609
#define B_HIGH   0.000253768
#define C_HIGH   8.53411e-07
#define D_HIGH   -8.79629e-08

void(* reboot) (void) = 0;


float temp_calc (float a, float b, float c, float d, float r_rel) {

    float sum = a +
                b * log(r_rel) +
                c * log(r_rel) * log(r_rel) +
                d * log(r_rel) * log(r_rel) * log(r_rel);
    
    return 1 / sum;

}

float temp (int adc_meas, float r_ref) {

    float v_out = (adc_meas + 0.5) * ADC_REF / ADC_RES;

    float r_rel = v_out * r_ref / (VCC - v_out) / R25;

    if (r_rel > 0.3599 && r_rel <= 3.277) {
        return temp_calc(A_LOW, B_LOW, B_LOW, B_LOW, r_rel) - 273.15;
    } else if (r_rel > 0.06816 && r_rel <= 0.3599) {
        return temp_calc(A_MEDIUM, B_MEDIUM, B_MEDIUM, B_MEDIUM, r_rel) - 273.15;
    } else if (r_rel <= 0.06816) {
        return temp_calc(A_HIGH, B_HIGH, B_HIGH, B_HIGH, r_rel) - 273.15;
    } else {
        return -1; // We're not happy (in a good sense)
    }
}

void setup()
{
	pinMode(ADC_MUX_ADDR_0_PIN, OUTPUT);
	pinMode(ADC_MUX_ADDR_1_PIN, OUTPUT);
	pinMode(ADC_MUX_ADDR_2_PIN, OUTPUT);
	pinMode(ADC_MUX_ADDR_3_PIN, OUTPUT);
	
	Serial.begin(9600);
	while (!Serial);

	Serial.println("PER Thermistors... sadly written for Arduino\n");

	CAN.setPins(MCP2515_CS_PIN, MCP2515_DEFAULT_INT_PIN);
	if (!CAN.begin(500E3)) {
		Serial.println("Error with CAN initilization!");
		reboot();
	}
}

void loop()
{
	static uint8_t mux_index = 0;

	// MUX decoding
	digitalWrite(ADC_MUX_ADDR_0_PIN, (mux_index & 0b0001) == 0b0001);
	digitalWrite(ADC_MUX_ADDR_1_PIN, (mux_index & 0b0010) == 0b0010);
	digitalWrite(ADC_MUX_ADDR_2_PIN, (mux_index & 0b0100) == 0b0100);
	digitalWrite(ADC_MUX_ADDR_3_PIN, (mux_index & 0b1000) == 0b1000);
	
	// Allow for analog mux to settle, 
	delay(250);

	// Analog sample
	uint16_t temp0_raw = analogRead(ADC_MUX_SIG_0_PIN);
 delay(10);
	uint16_t temp1_raw = analogRead(ADC_MUX_SIG_1_PIN);
  delay(10);
	uint16_t temp2_raw = analogRead(ADC_MUX_SIG_2_PIN);
  delay(10);
	uint16_t temp3_raw = analogRead(ADC_MUX_SIG_3_PIN);

 float temp0_real = temp(temp0_raw, R_REF_0);
 float temp1_real = temp(temp1_raw, R_REF_1);
 float temp2_real = temp(temp2_raw, R_REF_2);
 float temp3_real = temp(temp3_raw, R_REF_3);

	// CAN message construction
	uint16_t msg_id = CAN_MESSAGE_ID_BASE + mux_index;

  if(temp0_raw < 0 || temp0_raw > 1000){
      Serial.print("--- mux_index = ");
      Serial.print(mux_index);
      Serial.print(", temp0 (raw) = ");
      Serial.print(temp0_raw);
      Serial.print("--- \n");
    } else {
      Serial.print("mux_index = ");
      Serial.print(mux_index);
      Serial.print(", temp3z_raw = ");
      Serial.print(temp3_raw);
	  Serial.print(" real temp = ");
	  Serial.print(temp3_real);
	  Serial.print("\n");    
    }

  uint16_t temp0 = (uint16_t) (temp0_real * 10);
  uint16_t temp1 = (uint16_t) (temp1_real * 10);
  uint16_t temp2 = (uint16_t) (temp2_real * 10);
  uint16_t temp3 = (uint16_t) (temp3_real * 10);
  
	CAN.beginExtendedPacket(msg_id);
  CAN.write(temp0 & 0xFF);
  CAN.write((temp0 >> 8) & 0xFF);
  CAN.write(temp1 & 0xFF);
  CAN.write((temp1 >> 8) & 0xFF);
  CAN.write(temp2 & 0xFF);
  CAN.write((temp2 >> 8) & 0xFF);
  CAN.write(temp3 & 0xFF);
  CAN.write((temp3 >> 8) & 0xFF);
	CAN.endPacket();

  if(mux_index == 15){
    mux_index = 0;
  } else {
    mux_index++;
  }

}
