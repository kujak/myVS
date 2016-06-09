/*
 Name:		NetworkOnly.ino
 Created:	6/8/2016 11:53:37 PM
 Author:	Kujak
*/
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02 };
IPAddress ip(192, 168, 69, 190); 
EthernetServer server(80);

// the setup function runs once when you press reset or power the board

void setup() 
{
	// start the Ethernet connection and the server:
	Ethernet.begin(mac, ip);
	server.begin();
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	EthernetClient client = server.available();
	
	if (client) 
	{
		Serial.println("client connected");
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
					client.println("Connection: close");  // the connection will be closed after completion of the response
					client.println("Refresh: 5");  // refresh the page automatically every 5 sec
					client.println();
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");
					client.println("</html>");
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
		Serial.println("client disconnected");
	}
}
