#include <Wire.h>

#define ADDRESS 0x50

void setup()
{

  /* add setup code here */
	Serial.begin(9600);

	Wire.begin();
}

void loop()
{

  /* add main program code here */

}

void dataWrite()
{
	Wire.beginTransmission(ADDRESS);
	Wire.write(1);
	Wire.endTransmission();
}

void dataRead()
{
	Wire.beginTransmission(ADDRESS);
	Wire.endTransmission();

	Wire.requestFrom(ADDRESS, 1);

	while (!Wire.available())
	{

	}
}
