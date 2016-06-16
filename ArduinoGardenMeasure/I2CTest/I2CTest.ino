/*
 Name:		I2CTest.ino
 Created:	6/10/2016 1:13:03 AM
 Author:	Kujak
*/

#include <SD.h>
#include <TimerOne.h>
#include <Wire.h>

#define sensor1 0x20
#define sensor2 0x21
#define sensor3 0x22
#define sensor4 0x23
#define seconds 5

const int chipSelect = 4;

volatile boolean I2Cstart;

int I2Cmoist;
int I2Ctemp;
int I2Clight;
int	I2CSensor = 1;

String dataString = "";

int ArrayMoist[4];
int ArrayTemp[4];
int ArrayLight[4];

boolean SDOK;
boolean firstRun = true;

int I2ChumiSensor[4] = { sensor1, sensor2, sensor3, sensor4 };

void setup()
{
	Timer1.initialize(seconds*1000000);
	Timer1.attachInterrupt(I2CdoScan);	

	Wire.begin();

	Serial.begin(9600);
	Serial.println("setup");

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
	if (firstRun)
	{
		for (int i = 1; i <= I2CSensor; i++)
		{
			Wire.beginTransmission(I2ChumiSensor[i]);
			if (Wire.endTransmission()) 
			{ 
				I2CwriteRegister8bit(I2ChumiSensor[i], 6); 
			}
		}

		firstRun = false;
	}

	if (I2Cstart)
	{
		I2CgetValues(I2ChumiSensor[I2CSensor]);
		I2Cstart = false;
		I2CSensor++;

		if (I2CSensor > 2)	
		{ 
			I2CSensor = 0;	
		}
	}

	noInterrupts();
	ArrayMoist[I2CSensor] = I2Cmoist;
	ArrayTemp[I2CSensor]  = I2Ctemp;
	ArrayLight[I2CSensor] = I2Clight;
	interrupts();

	delay(1000);

	SDcreateOutput(ArrayMoist[I2CSensor],ArrayTemp[I2CSensor],ArrayLight[I2CSensor]);
}
void SDcreateOutput(int _moist,int _temp, int _light)
{
	//dataString = "";
	//dataString = "Sensor : ";    dataString += I2CSensor;
	//dataString += " --> Moist "; dataString += _moist;
	//dataString += ", Temp ";     dataString += _temp;
	//dataString += ", Light ";    dataString += _light;

	dataString = "";
	dataString = "Sensor : ";    dataString += I2CSensor;
	dataString += " --> Moist "; dataString += _moist;
	dataString += ", Temp ";     dataString += _temp;
	dataString += ", Light ";    dataString += _light;

	if (SDOK)
	{
		SDwriteTo();
	}
	
	Serial.println(dataString);
}
void SDwriteTo()
{
	File dataFile = SD.open("datalog.txt", FILE_WRITE);

	// if the file is available, write to it:
	if (dataFile) 
	{
		dataFile.println(dataString);
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
void I2CgetValues(uint8_t _addr)
{
	I2Cmoist = I2CreadRegister16bit(_addr, 0);
	I2Ctemp  = I2CreadRegister16bit(_addr, 5);
	I2CwriteRegister8bit(_addr, 3);
	I2Clight = I2CreadRegister16bit(_addr, 4);
}
void I2CwriteRegister8bit(int _addr, int value)
{
	Wire.beginTransmission(_addr);
	Wire.write(value);
	Wire.endTransmission();
}
unsigned int I2CreadRegister16bit(int _addr, int reg)
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