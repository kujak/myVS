/*
 Name:		Timer.ino
 Created:	6/9/2016 11:36:00 PM
 Author:	Kujak
*/

#include <TimerOne.h>
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
	Timer1.initialize(1000000);
	Timer1.attachInterrupt(getI2C);
	Serial.begin(9600);
	Serial.println("setup");
}

void loop(void)
{
	Serial.println("loop");
	noInterrupts();
	I2Cmoist = moist;
	I2Ctemp = temp;
	I2Clight = light;
	interrupts();

	Serial.print("blinkCount = ");
	Serial.println(moist);
	delay(100);
}
void getI2C(void)
{
	Serial.println("getI2C");
//	int moist = I2CreadRegister16bit(sensor01, 0);
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