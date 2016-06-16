// I2CSensorCap.h

#ifndef _I2CSENSORCAP_h
#define _I2CSENSORCAP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class I2CSensorCap
{
public :
	I2CSensorCap(int addr);	
	unsigned int getValue(int _selection);
	String getValues();
	void begin();
private:
	int _addr;
	void I2CwriteRegister8bit(int value);
	unsigned int I2CreadRegister16bit(int reg);
};

#endif

