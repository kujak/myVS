/*
 Name:		I2CScanWriteSDWeb.ino
 Created:	6/16/2016 10:45:36 AM
 Author:	Kujak
*/

#include "I2CSensorCap.h"
#include <Ethernet.h>
#include <SD.h>
#include <TimerOne.h>
#include <Wire.h>

#define sensor1 0x20
#define sensor2 0x21

#define seconds 60

const int chipSelect = 4;

volatile boolean I2Cstart;

int	I2CSensor = 1;
int I2CSensorcur = 1;

String latestData;

boolean SDOK;
boolean I2CsensorActive[2];

I2CSensorCap I2Csensor1 = I2CSensorCap(sensor1);
I2CSensorCap I2Csensor2 = I2CSensorCap(sensor2);

// Hier die MAC Adresse des Shields eingeben (Aufkleber auf Rückseite)
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xFB, 0x80 };
// Eine IP im lokalen Netzwerk angeben
IPAddress ip(192, 168, 69, 190);
// Ethernet Library als Server initialisieren verwendet die obige IP, Port ist per default 80
EthernetServer server(80);

void setup()
{
	Serial.println(".setup");
	Serial.begin(9600);	

	// init Interrupt Timer zum Auslesen der Werte
	Timer1.initialize(seconds * 1000000);
	Timer1.attachInterrupt(I2CdoScan);

	// I2C Kommunikation starten
	Wire.begin();

	setupI2C();
	setupEthernet();
	setupSDCard();	
}
void setupI2C()
{	
	Serial.println(".- setupI2C");
	// Sensor 1 starten
	I2Csensor1.begin();
}
void setupSDCard()
{
	Serial.println(".- setupSDCard");
	// SD Karte initialisieren bzw als inaktiv markieren
	if (!SD.begin(chipSelect))
	{
		Serial.println(".-- Card failed, or not present");
		// don't do anything more:
		return;
	}
	else
	{
		SDOK = true;
		Serial.println(".-- card initialized.");
	}
}
void setupEthernet()
{
	Serial.println(".- setupEthernet");
	// Ethernet Verbindung und Server starten
	Ethernet.begin(mac, ip);
	server.begin();
	Serial.print(".-- Server gestartet. IP: ");
	// IP des Arduino-Servers ausgeben
	Serial.println(Ethernet.localIP());
}
void loop()
{
	if (I2Cstart) // Interrupt holt die I2C Daten
	{
		//I2CstepClient();
		SDcreateOutput(latestData);
		I2Cstart = false;
	}

	EthernetClient client = server.available();

	if (client)
	{
		ClientOutput(client);
		client.stop();
		Serial.println("client disconnected");
	}
}
void I2CstepClient()
{
	latestData = I2Csensor1.getValues();
}
void ClientOutput(EthernetClient _client)
{
	Serial.println("client connected");
	// an http request ends with a blank line
	boolean currentLineIsBlank = true;

	while (_client.connected())
	{		
		if (_client.available())
		{
			char c = _client.read();
			// if you've gotten to the end of the line (received a newline
			// character) and the line is blank, the http request has ended,
			// so you can send a reply
			if (c == '\n' && currentLineIsBlank)
			{
				// send a standard http response header
				_client.println("HTTP/1.1 200 OK");
				_client.println("Content-Type: text/html");
				_client.println("Connection: close");  // the connection will be closed after completion of the response				
				_client.println("Refresh: 5");  // refresh the page automatically every 5 sec
				_client.println();
				_client.println("<!DOCTYPE HTML>");
				_client.println("<html> <body>");
			    _client.println(latestData);
				_client.println("</body> </html>");
				break;
			}
			if (c == '\n')
			{
				// you're starting a new line
				currentLineIsBlank = true;
			}
			else if (c != '\r')
			{
				// you've gotten a character on the current line
				currentLineIsBlank = false;
			}
		}
	}

	// give the web browser time to receive the data
	delay(1);	
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