#include <CAN.h>

// ADC MUX Pins
#define ADC_MUX_ADDR_0_PIN (0)
#define ADC_MUX_ADDR_1_PIN (1)
#define ADC_MUX_ADDR_2_PIN (2)
#define ADC_MUX_ADDR_3_PIN (3)

#define ADC_MUX_SIG_0_PIN (A0)
#define ADC_MUX_SIG_1_PIN (A1)
#define ADC_MUX_SIG_2_PIN (A2)
#define ADC_MUX_SIG_3_PIN (A3)

#define CAN_MESSAGE_ID_BASE (0xBE0)


void setup()
{
  //analogReference(EXTERNAL);
  
	pinMode(ADC_MUX_ADDR_0_PIN, OUTPUT);
	pinMode(ADC_MUX_ADDR_1_PIN, OUTPUT);
	pinMode(ADC_MUX_ADDR_2_PIN, OUTPUT);
	pinMode(ADC_MUX_ADDR_3_PIN, OUTPUT);
	
//	pinMode(ADC_MUX_SIG_0_PIN);
//	pinMode(ADC_MUX_SIG_1_PIN);
//	pinMode(ADC_MUX_SIG_2_PIN);
//	pinMode(ADC_MUX_SIG_3_PIN);

	Serial.begin(9600);
	while (!Serial);

	Serial.println("PER Thermistors... sadly written for Arduino\n");

	if (!CAN.begin(500E3)) {
		Serial.println("Error with CAN initilization!");
		//while (1);
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
	uint16_t temp0 = analogRead(ADC_MUX_SIG_0_PIN);
	uint16_t temp1 = analogRead(ADC_MUX_SIG_1_PIN);
	uint16_t temp2 = analogRead(ADC_MUX_SIG_2_PIN);
	uint16_t temp3 = analogRead(ADC_MUX_SIG_3_PIN);

	// CAN message construction
	uint16_t msg_id = CAN_MESSAGE_ID_BASE + mux_index;

  if(temp0 < 0 || temp0 > 1000){
      Serial.print("--- mux_index = ");
      Serial.print(mux_index);
      Serial.print(", temp0 (raw) = ");
      Serial.print(temp0);
      Serial.print("--- \n");
    } else {
      Serial.print("mux_index = ");
      Serial.print(mux_index);
      Serial.print(", temp0 (raw) = ");
      Serial.print(temp0);
      Serial.print("\n");     
    }
   
	CAN.beginExtendedPacket(msg_id);
	CAN.write((temp0 >> 8) & 0xFF);
	CAN.write((temp0) & 0xFF);
	CAN.write((temp1 >> 8) & 0xFF);
	CAN.write((temp1) & 0xFF);
	CAN.write((temp2 >> 8) & 0xFF);
	CAN.write((temp2) & 0xFF);
	CAN.write((temp3 >> 8) & 0xFF);
	CAN.write((temp3) & 0xFF);
	CAN.endPacket();

  if(mux_index == 15){
    mux_index = 0;
  } else {
    mux_index++;
  }

}
