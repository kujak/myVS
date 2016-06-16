// 
// 
// 

#include "I2CSensorCap.h"
#include "Wire.h"

I2CSensorCap::I2CSensorCap(int addr)
{
	_addr = addr;	
}

void I2CSensorCap::I2CwriteRegister8bit( int value)
{
	Wire.beginTransmission(_addr);
	Wire.write(value);
	Wire.endTransmission();
}

unsigned int I2CSensorCap::I2CreadRegister16bit(int reg)
{
	Wire.beginTransmission(_addr);
	Wire.write(reg);
	Wire.endTransmission();
	delay(20);

	Wire.requestFrom(_addr, 2);

	unsigned int t = Wire.read() << 8;

	t = t | Wire.read();

	return t;
}

unsigned int I2CSensorCap::getValue(int _selection)
{
	unsigned int ret;

	switch (_selection)
	{
		case 1: ret = I2CreadRegister16bit(0); break;
		case 2: ret = I2CreadRegister16bit(5); break;
		case 3:	I2CwriteRegister8bit(3); 
			    ret = I2CreadRegister16bit(4); break;
	}

	return ret;
}

String I2CSensorCap::getValues()
{
	String ret;

	ret = String(_addr)+ ','+ String(getValue(1)) + ',' + String(getValue(2)) + ',' + String(getValue(3));

	return ret;
}

void I2CSensorCap::begin()
{
	I2CwriteRegister8bit(6); // Reset
}
