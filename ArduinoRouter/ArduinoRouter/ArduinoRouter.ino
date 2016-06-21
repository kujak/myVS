/*
 Name:		ArduinoRouter.ino
 Created:	6/21/2016 2:26:15 PM
 Author:	Kujak
*/

#include <util.h>
#include <ICMPPing.h>
#include <SPI.h>
#include <Ethernet.h>
// comment if no Debug Info needed
#define DEBUG 1

#ifdef DEBUG
	#define DEBUG_PRINT(x)  Serial.println(x)
#else
	#define DEBUG_PRINT(x)
#endif

byte		mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress	ip(192, 168, 69, 7);
IPAddress	pingAddr(8, 8, 8, 8); // ip address to ping

SOCKET		pingSocket = 0;

char		buffer[256];
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));

EthernetClient client;
EthernetServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis	= 0;        // will store last time LED was updated
	
const long	interval			= 30 * 1000; // interval at which to blink (milliseconds)
const int	pingDownRouterMax	= 4;
const int	pingDownModemMax	= 10;
const int	Modem				= 5;
const int	Router				= 6;

boolean		pingOK				= true;
boolean		doneReset			= false;

int			pingDown;

void setup() 
{
	pinMode(Modem, OUTPUT);  // sets the digital pin as output - Relais1
	pinMode(Router, OUTPUT); // sets the digital pin as output - Relais2
	pinMode(7, OUTPUT);      // sets the digital pin as output - Relais3
	pinMode(8, OUTPUT);      // sets the digital pin as output - Relais4

	// Open serial communications and wait for port to open:
	Serial.begin(9600);

	while (!Serial) 
	{
		; // wait for serial port to connect. Needed for native USB port only
	}

	// start the Ethernet connection and the server:
	Ethernet.begin(mac, ip);
	server.begin();
	Serial.println("server is at : ");
	Serial.println(Ethernet.localIP());
}

void loop()
{
	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis >= interval)
	{
		// save the last time you blinked the LED
		previousMillis = currentMillis;
		// test the ping connection
		executePing();

		if (!pingOK)
		{
			// start the reset procedure if one fail
			pinReset();
		}
		else
		{
			// reset values when ping OK
			pingDown = 0;
			doneReset = false;
		}
	}
}	

void executePing()
{
	boolean			ret;
	ICMPEchoReply	echoReply = ping(pingAddr, 4);
	
	if (echoReply.status == SUCCESS)
	{
		// ping is OK
		ret = true;
		
		sprintf(buffer,
			"Reply[%d] from: %d.%d.%d.%d: bytes=%d time=%ldms TTL=%d",
			echoReply.data.seq,	echoReply.addr[0], echoReply.addr[1],
			echoReply.addr[2], echoReply.addr[3], REQ_DATASIZE,
			millis() - echoReply.data.time,	echoReply.ttl);
	}
	else
	{
		// ping failed
		ret = false;

		sprintf(buffer, "Echo request failed; %d", echoReply.status);	
	}

	Serial.println(buffer);
	
	pingOK = ret;
}

void pinReset()
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
				Serial.println("Modem reset");
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
			Serial.println("Router reset");
			digitalWrite(Router, HIGH);
			delay(100);
			digitalWrite(Router, LOW);

			// go for modem if not working
			doneReset = true;
		}
	}
}