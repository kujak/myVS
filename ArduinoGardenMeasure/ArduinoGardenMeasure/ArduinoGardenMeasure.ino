/*
 Name:		ArduinoGardenMeasure.ino
 Created:	6/7/2016 4:24:28 PM
 Author:	Kujak
*/
#include <Wire.h>

#define sensor01 0x20
// the setup function runs once when you press reset or power the board
void setup() 
{
	Wire.begin();
	Serial.begin(9600);
	writeI2CRegister8bit(sensor01, 6); //reset
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	Serial.print("Moist: ");
	Serial.print(readI2CRegister16bit(sensor01, 0)); //read capacitance register
	Serial.print(", Temp: ");

	Serial.print(readI2CRegister16bit(sensor01, 5)); //temperature register
	Serial.print(", Light: ");

	writeI2CRegister8bit(0x20, 3); //request light measurement 
	Serial.println(readI2CRegister16bit(sensor01, 4)); //read light register
}
void writeI2CRegister8bit(int addr, int value) 
{
	Wire.beginTransmission(addr);
	Wire.write(value);
	Wire.endTransmission();
}

unsigned int readI2CRegister16bit(int addr, int reg) 
{
	Wire.beginTransmission(addr);
	Wire.write(reg);
	Wire.endTransmission();
	delay(20);

	Wire.requestFrom(addr, 2);

	unsigned int t = Wire.read() << 8;

	t = t | Wire.read();

	return t;
}