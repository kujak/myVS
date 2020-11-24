#include <Wire.h>

#define sensor01 0x20;

void setup()
{

  /* add setup code here */
	Wire.begin();
	Serial.begin(9600);
	writeI2CRegister8bit(sensor01, 6); //reset
}

void loop()
{

  /* add main program code here */
	Serial.print("Moist: ");
	Serial.print(readI2CRegister16bit(sensor01, 0)); //read capacitance register
	Serial.print(", Temp: ");

	Serial.print(readI2CRegister16bit(sensor01, 5)); //temperature register
	Serial.print(", Light: ");

	writeI2CRegister8bit(0x20, 3); //request light measurement 
	Serial.println(readI2CRegister16bit(sensor01, 4)); //read light register
}
void writeI2CRegister8bit(int addr, int value) {
	Wire.beginTransmission(addr);
	Wire.write(value);
	Wire.endTransmission();
}

unsigned int readI2CRegister16bit(int addr, int reg) {
	Wire.beginTransmission(addr);
	Wire.write(reg);
	Wire.endTransmission();
	delay(20);

	Wire.requestFrom(addr, 2);

	unsigned int t = Wire.read() << 8;

	t = t | Wire.read();

	return t;
}