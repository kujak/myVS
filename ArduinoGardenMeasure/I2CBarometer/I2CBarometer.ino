/*
 Name:		I2CBarometer.ino
 Created:	6/17/2016 3:06:56 PM
 Author:	Kujak
*/

#include <Wire.h>
#include <Adafruit_MPL115A2.h>

Adafruit_MPL115A2 mpl115a2;

int sensorPin = A0;    // select the input pin for the potentiometer

float rawRange = 1024; // 3.3v
float logRange = 5.0; // 3.3v = 10^5 lux

void setup(void)
{
	Serial.begin(9600);
	Serial.println("Hello!");

	Serial.println("Getting barometric pressure ...");
	mpl115a2.begin();
}

void loop(void)
{
	float pressureKPA = 0, temperatureC = 0;

	mpl115a2.getPT(&pressureKPA, &temperatureC);
	Serial.print("Pressure (kPa): "); Serial.print(pressureKPA, 4); Serial.print(" kPa  ");
	Serial.print("Temp (*C): "); Serial.print(temperatureC, 1); Serial.println(" *C both measured together");

	pressureKPA = mpl115a2.getPressure();
	Serial.print("Pressure (kPa): "); Serial.print(pressureKPA, 4); Serial.println(" kPa");

	temperatureC = mpl115a2.getTemperature();
	Serial.print("Temp (*C): "); Serial.print(temperatureC, 1); Serial.println(" *C");
	
	int rawValue = analogRead(sensorPin);

	Serial.print("Raw = ");
	Serial.print(rawValue);
	Serial.print(" - Lux = ");
	Serial.println(RawToLux(rawValue));

	delay(1000);
}

float RawToLux(int raw)
{
	float logLux = raw * logRange / rawRange;
	return pow(10, logLux);
}