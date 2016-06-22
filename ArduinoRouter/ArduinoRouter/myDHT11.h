// myDHT11.h

#ifndef MYDHT11_h
#define MYDHT11_h

#define DHTLIB_OK				0
#define DHTLIB_ERROR_CHECKSUM	-1
#define DHTLIB_ERROR_TIMEOUT	-2

#include <dht11.h>

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


class myDHT11
{
public:
	myDHT11();
	double Fahrenheit(double celsius);
	int Celcius2Fahrenheit(int celcius);
	double Kelvin(double celsius);
	double dewPoint(double celsius, double humidity,boolean fast);
private:
	int Pin;
	double dewPointNormal(double celsius, double humidity);
	double dewPointFast(double celsius, double humidity);
};
#endif

