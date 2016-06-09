/*
 Name:		I2CTest.ino
 Created:	6/10/2016 1:13:03 AM
 Author:	Kujak
*/

#include <Wire.h>

#define sensor01 0x20

volatile int moist;
volatile int temp;
volatile int light;
int I2Cmoist;
int I2Ctemp;
int I2Clight;

void setup(void)
{
	Wire.begin();
	I2CwriteRegister8bit(sensor01, 6); //reset	
	Serial.begin(9600);
	Serial.println("setup");
}

void loop(void)
{
	Serial.println("loop");
	I2Cmoist = moist;
	I2Ctemp = temp;
	I2Clight = light;

	Serial.print("moist = ");
	Serial.println(I2Cmoist);
	delay(1000);
	getI2C();
}
void getI2C(void)
{
	Serial.println("getI2C");
	int moist = I2CreadRegister16bit(sensor01, 0);
	//	int temp = I2CreadRegister16bit(sensor01, 5);
	//	I2CwriteRegister8bit(sensor01, 3);
	//	int light = I2CreadRegister16bit(sensor01, 4);
}

void I2CwriteRegister8bit(int addr, int value)
{
	Wire.beginTransmission(addr);
	Wire.write(value);
	Wire.endTransmission();
}

unsigned int I2CreadRegister16bit(int addr, int reg)
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