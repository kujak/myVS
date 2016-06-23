/*
 Name:		ArduinoRouter.ino
 Created:	6/21/2016 2:26:15 PM
 Author:	Kujak
*/

//#include <SD.h>
#include <SdFat.h>
#include "myDHT11.h"
#include <dht11.h>
//#include <util.h>
#include <ICMPPing.h>
#include <SPI.h>
#include <Ethernet.h>

// comment if no Debug Info needed
//#define DEBUG 1
//#define UseInternetClient 1

#ifdef DEBUG
	#define DEBUG_PRINTLN(x)	Serial.println(x)
	#define DEBUG_PRINTLN2(x,y) Serial.println(x,y)
	#define DEBUG_PRINT(x)		Serial.print(x)
	#define DEBUG_PRINT2(x,y)	Serial.print(x,y)
#else
	#define DEBUG_PRINTLN(x)
	#define DEBUG_PRINTLN2(x,y)
	#define DEBUG_PRINT(x)
	#define DEBUG_PRINT2(x,y)
#endif

// used Pins on Chip
#define chipSelect	4 // Pin for SD Lock
#define	Modem		5 // Pin for Modem Relais
#define	Router		6 // Pin for Router Releais
#define	Relais3		7 // Pin for Releais3
#define	Relais4		8 // Pin for Releais4
#define DHT11PIN	9 // Pin for Temp Modul

#define	pingDownRouterMax	4  // max of not reachable Ping before Router restart
#define	pingDownModemMax	10 // max of not reachable Ping before Modem restart

byte		mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress	ip(192, 168, 69, 7);
IPAddress	pingAddr(8, 8, 8, 8); // ip address to ping

SOCKET		pingSocket = 0;

char		buffer[256];
ICMPPing	ping(pingSocket, (uint16_t)random(0, 255));

EthernetClient	client;
EthernetServer	server(80);
dht11			DHT11;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
//unsigned long previousMillis1	= 0; // will store last time ping was executed
unsigned long previousMillis1   = 0; // will store last time ping was executed	
unsigned long previousMillis2	= 0; // will store last time ping was executed	
unsigned long interval1			= 30 * 1000; // interval at which to test ping (milliseconds)
unsigned long interval2			= interval1*2.345; // interval at which to test ping (milliseconds)

boolean		SDOK;
boolean		pingOK				= true;
boolean		doneReset			= false;

int			pingDown; // counter how often ping fails
int			avgPing;  // average Ping time

uint8_t		avgHumidity;
uint8_t		avgTemperature;

SdFat	Sd;

void setup() 
{
	// Setup Relais Pins
	pinMode(Modem, OUTPUT);   // sets the digital pin as output - Relais1
	pinMode(Router, OUTPUT);  // sets the digital pin as output - Relais2
	pinMode(Relais3, OUTPUT); // sets the digital pin as output - Relais3
	pinMode(Relais4, OUTPUT); // sets the digital pin as output - Relais4
	
	// Open serial communications and wait for port to open:
	setupSerial();
	
	// start the Ethernet connection and the server:
	setupEthernet();
	// start all once to init the values
	setupValues();	

	setupSDCard();
	
	SDcreateOutput("Setup");
}
void setupSerial()
{
	Serial.begin(14400);

	while (!Serial)
	{
		; // wait for serial port to connect. Needed for native USB port only
	}
}
void setupEthernet()
{
	Ethernet.begin(mac, ip);
	server.begin();
	DEBUG_PRINT("server is at : ");
	DEBUG_PRINTLN(Ethernet.localIP());
}
void setupValues()
{
//	pingExecute();
//	DHT11measure();
	previousMillis1 = millis();	
	previousMillis2 = previousMillis1;
}
void setupSDCard()
{
	DEBUG_PRINTLN(".- setupSDCard");
	// SD Karte initialisieren bzw als inaktiv markieren
	if (!Sd.begin(chipSelect))
	{
		DEBUG_PRINTLN(".-- Card failed, or not present");
		// don't do anything more:
		return;
	}
	else
	{
		SDOK = true;
		DEBUG_PRINTLN(".-- card initialized.");
	}
}
void loop()
{
	timerCheck(millis());	

#ifdef UseInternetClient
	listenForEthernetClients();
#endif
}	
void timerCheck(unsigned long _currentMillis)
{
	if (_currentMillis - previousMillis1 >= interval1)
	{
		Serial.println("Timer1");
		DEBUG_PRINTLN("timer1check");
		// save the last time you blinked the LED
		previousMillis1 = _currentMillis;
		//execute Timer1
		timer1();		
	}

	if (_currentMillis - previousMillis2 >= interval2)
	{
		Serial.println("Timer2");
		DEBUG_PRINTLN("timer2check");
		// save the last time you blinked the LED
		previousMillis2 = _currentMillis;
		//execute Timer2 content
		timer2();
	}
}
void timer1()
{
	pingExecute();

	// do measure only if ping is OK
	if (!pingOK)
	{
		// start the reset procedure if one fail
		pingReset();
	}
	else
	{
		// reset values when ping OK
		pingDown = 0;
		doneReset = false;
	}
}
void timer2()
{
	DHT11measure();
}
void pingExecute()
{
	boolean			ret;
	int				currentPing;
	ICMPEchoReply	echoReply = ping(pingAddr, 4);
	
	if (echoReply.status == SUCCESS)
	{
		// ping is OK
		ret = true;
		currentPing = millis() - echoReply.data.time;
		avgPing = mkAvg(avgPing, currentPing);

#ifdef DEBUG		
/*		sprintf(buffer,
			"Reply[%d] from: %d.%d.%d.%d: bytes=%d time=%ldms TTL=%d",
			echoReply.data.seq,	echoReply.addr[0], echoReply.addr[1],
			echoReply.addr[2], echoReply.addr[3], REQ_DATASIZE,
			currentPing,	echoReply.ttl);*/
#endif
	}
	else
	{
		// ping failed
		ret = false;
#ifdef DEBUG
//		sprintf(buffer, "Echo request failed; %d", echoReply.status);	
#endif
	}

	DEBUG_PRINTLN(buffer);
	DEBUG_PRINT("Avg Ping : ");
	DEBUG_PRINTLN(avgPing);
	Serial.println(avgPing);
	pingOK = ret;
}
void pingReset()
{
	// increment each call of this function
	pingDown++;

	//maximum of valid failed pings reached
	if (pingDown >= pingDownRouterMax)
	{
		// did we already reset the router and have additional ping failures		
		if (doneReset)
		{
			if (pingDown >= pingDownModemMax)
			{
				// reset the modem
				DEBUG_PRINTLN("Modem reset");				
				SDcreateOutput("Modem reset");
				digitalWrite(Modem, HIGH);
				delay(100);
				digitalWrite(Modem, LOW);
				pingDown = 0;
			}
		}
		// max failed pings reached, reset router first
		else 
		{
			// reset router
			DEBUG_PRINTLN("Router reset");			
			SDcreateOutput("Router reset");
			digitalWrite(Router, HIGH);
			delay(100);
			digitalWrite(Router, LOW);

			// go for modem if not working
			doneReset = true;
		}
	}
}
#ifdef UseInternetClient
void listenForEthernetClients() 
{
	// listen for incoming clients
	EthernetClient client = server.available();
	if (client) 
	{
		DEBUG_PRINTLN("Got a client");
		// an http request ends with a blank line
		boolean currentLineIsBlank = true;
		while (client.connected()) 
		{
			if (client.available()) 
			{
				char c = client.read();
				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank) 
				{
					// send a standard http response header
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println();
					// print the current readings, in HTML format:
					client.print("Temperature: ");
					client.print(avgTemperature);
					client.print(" degrees C");client.println("<br />");
					client.print("Humidity: " + String(avgHumidity));
					client.print(" %");client.println("<br />");
					client.print("Ping time : " + String(avgPing));
					client.print(" ms");client.println("<br />");
					client.print("|- Relais 1 (Modem) : " + String(digitalRead(Modem)));	client.println("<br />");
					client.print("|- Relais 2 (Router) : " + String(digitalRead(Router)));	client.println("<br />");
					client.print("|- Relais 3 () : " + String(digitalRead(7)));				client.println("<br />");
					client.print("|- Relais 4 () : " + String(digitalRead(8)));				client.println("<br />");
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
		// close the connection:
		client.stop();
	}
}
#endif
void DHT11measure() // measure temp and humidity when a ping was sucessfull
{
	int chk = DHT11.read(DHT11PIN);

	DEBUG_PRINT("Read sensor : ");

	switch (chk)
	{
		case DHTLIB_OK:
			DEBUG_PRINTLN("OK");
			break;
		case DHTLIB_ERROR_CHECKSUM:
			DEBUG_PRINTLN("Checksum error");
			break;
		case DHTLIB_ERROR_TIMEOUT:
			DEBUG_PRINTLN("Time out error");
			break;
		default:
			DEBUG_PRINTLN("Unknown error");
			break;
	}

	Serial.println(chk);

	if (chk == DHTLIB_OK)
	{
		avgTemperature = mkAvg(avgTemperature, DHT11.temperature);
		avgHumidity = mkAvg(avgHumidity, DHT11.humidity);

//		myDHT11 myDHT11calc(DHT11.temperature, DHT11.humidity);

		DEBUG_PRINT("Humidity (%) : ");
		DEBUG_PRINT2((float)DHT11.humidity, 2);
		DEBUG_PRINT(" - avg : ");
		DEBUG_PRINTLN2((float)avgHumidity, 2);		

		DEBUG_PRINT("Temperature (C) : ");
		DEBUG_PRINT2((float)DHT11.temperature, 2);
		DEBUG_PRINT(" - avg : ");
		DEBUG_PRINTLN2((float)avgTemperature, 2);

		//	Serial.print("Temperature (F): ");
		//	Serial.println(Fahrenheit(DHT11.temperature), 2);

		//	Serial.print("Temperature (K): ");
		//	Serial.println(Kelvin(DHT11.temperature), 2);

		//DEBUG_PRINT("Dew Point (C): ");
		//DEBUG_PRINTLN(myDHT11calc.dewPoint(false));

		//DEBUG_PRINT("Dew PointFast (C): ");
		//DEBUG_PRINTLN(myDHT11calc.dewPoint(true));
	}
}
void SDcreateOutput(String _dataString)
{
	if (SDOK)
	{
		SDwriteTo(_dataString);
	}

	DEBUG_PRINTLN(_dataString);
}
void SDwriteTo(String _dataString)
{
	File dataFile = Sd.open("datalog.txt", FILE_WRITE);

	// if the file is available, write to it:
	if (dataFile)
	{
		dataFile.print(String(millis(), 10)); 
		dataFile.print(" - ");
		dataFile.println(_dataString);
		dataFile.close();
	}
	// if the file isn't open, pop up an error:
	else
	{
		DEBUG_PRINTLN("error opening datalog.txt");
	}
}
uint8_t mkAvg(uint8_t _old, uint8_t _new)
{
	uint8_t ret;

	if (_old == 0)
	{
		ret = _new;
	}
	else
	{
		ret = (_old + (2 * _new)) / 3;
	}

	return ret;
}