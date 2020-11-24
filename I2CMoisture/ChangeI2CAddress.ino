#include <Wire.h>

void writeI2CRegister8bit(int addr, int reg, int value) {
	Wire.beginTransmission(addr);
	Wire.write(reg);
	Wire.write(value);
	Wire.endTransmission();
}

void writeI2CRegister8bit(int addr, int value) {
	Wire.beginTransmission(addr);
	Wire.write(value);
	Wire.endTransmission();
}

void setup() {
	Wire.begin();
	Serial.begin(9600);
	//talking to the default address 0x20
	writeI2CRegister8bit(0x20, 1, 0x21); //change address to 0x21
	writeI2CRegister8bit(0x20, 6);       //reset
	delay(1000);                         //give it some time to boot
}

/*loop scans I2C bus and displays foud addresses*/
void loop()
{
	byte error, address;
	int nDevices;

	Serial.println("Scanning...");

	nDevices = 0;
	for (address = 1; address < 127; address++)
	{
		// The i2c_scanner uses the return value of
		// the Write.endTransmisstion to see if
		// a device did acknowledge to the address.
		Wire.beginTransmission(address);
		error = Wire.endTransmission();

		if (error == 0)
		{
			Serial.print("I2C device found at address 0x");
			if (address<16)
				Serial.print("0");
			Serial.print(address, HEX);
			Serial.println("  !");

			nDevices++;
		}
		else if (error == 4)
		{
			Serial.print("Unknow error at address 0x");
			if (address<16)
				Serial.print("0");
			Serial.println(address, HEX);
		}
	}
	if (nDevices == 0)
		Serial.println("No I2C devices found\n");
	else
		Serial.println("done\n");

	delay(5000);           // wait 5 seconds for next scan
}
