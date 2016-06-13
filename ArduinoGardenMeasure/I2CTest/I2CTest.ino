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
#define seconds 2

const int chipSelect = 4;

volatile boolean startI2C;

int I2Cmoist;
int I2Ctemp;
int I2Clight;
int	I2CSensor = 1;

String dataString = "";

int moist[4];
int temp[4];
int light[4];

boolean first = true;

int humiSensor[4] = { sensor1, sensor2, sensor3, sensor4 };

void setup()
{
	Timer1.initialize(seconds*1000000);
	Timer1.attachInterrupt(doI2C);	

	Wire.begin();

	Serial.begin(9600);
	Serial.println("setup");
	if (!SD.begin(chipSelect)) 
	{
		Serial.println("Card failed, or not present");
		// don't do anything more:
		return;
	}
	Serial.println("card initialized.");
}
void loop()
{	
	if (first)
	{
		for (int i = 1; i <= I2CSensor; i++)
		{
			Wire.beginTransmission(humiSensor[i]);
			if (Wire.endTransmission()) 
			{ 
				I2CwriteRegister8bit(humiSensor[i], 6); 
			}
		}

		first = false;
	}

	if (startI2C)
	{
		getI2C(humiSensor[I2CSensor]);
		startI2C = false;
		I2CSensor++;
		if (I2CSensor > 2)	
		{ 
			I2CSensor = 0;	
		}
	}

	noInterrupts();
	moist[I2CSensor] = I2Cmoist;
	temp[I2CSensor]  = I2Ctemp;
	light[I2CSensor] = I2Clight;
	interrupts();

	delay(1000);

	output(moist[I2CSensor],temp[I2CSensor],light[I2CSensor]);
}
void output(int _moist,int _temp, int _light)
{
	Serial.print("Sensor : "); Serial.print(I2CSensor);
	Serial.print(" --> Moist "); Serial.print(_moist);
	Serial.print(", Temp "); Serial.print(_temp);
	Serial.print(", Light "); Serial.println(_light);
}
void write2SD()
{
	File dataFile = SD.open("datalog.txt", FILE_WRITE);

	// if the file is available, write to it:
	if (dataFile) 
	{
		dataFile.println(dataString);
		dataFile.close();
		// print to the serial port too:
		Serial.println(dataString);
	}
	// if the file isn't open, pop up an error:
	else 
	{
		Serial.println("error opening datalog.txt");
	}
}
void doI2C()
{
	startI2C = true;
}
void getI2C(uint8_t _addr)
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