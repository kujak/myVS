/*
 Name:		I2CScanWriteSDWeb.ino
 Created:	6/16/2016 10:45:36 AM
 Author:	Kujak
*/

#include "I2CSensorCap.h"
#include <SD.h>
#include <TimerOne.h>
#include <Wire.h>

#define sensor1 0x20
#define seconds 10

const int chipSelect = 4;

volatile boolean I2Cstart;

int I2Cmoist;
int I2Ctemp;
int I2Clight;
int	I2CSensor = 1;

boolean SDOK;
boolean firstRun = true;

I2CSensorCap I2Csensor1 = I2CSensorCap(sensor1);

void setup()
{
	Timer1.initialize(seconds * 1000000);
	Timer1.attachInterrupt(I2CdoScan);
	
	Wire.begin();

	Serial.begin(9600);
	Serial.println("setup");

	I2Csensor1.begin();

	if (!SD.begin(chipSelect))
	{
		Serial.println("Card failed, or not present");
		// don't do anything more:
		return;
	}
	else
	{
		SDOK = true;
		Serial.println("card initialized.");
	}
}

void loop()
{
	if (I2Cstart)
	{
		SDcreateOutput(I2Csensor1.getValues());
	}

	Serial.println("Loop");
	delay(1000);	
}
void SDcreateOutput(String _dataString)
{
	if (SDOK)
	{
		SDwriteTo(_dataString);
	}

	Serial.println(_dataString);
}
void SDwriteTo(String _dataString)
{
	File dataFile = SD.open("datalog.txt", FILE_WRITE);

	// if the file is available, write to it:
	if (dataFile)
	{
		dataFile.println(_dataString);
		dataFile.close();
	}
	// if the file isn't open, pop up an error:
	else
	{
		Serial.println("error opening datalog.txt");
	}
}
void I2CdoScan()
{
	I2Cstart = true;
}