/*
 Name:		ArduinoGardenMeasure.ino
 Created:	6/7/2016 4:24:28 PM
 Author:	Kujak
*/
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>

#define sensor01 0x20

// Hier die MAC Adresse des Shields eingeben (Aufkleber auf Rückseite)
byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0xFB, 0x80 };

// Eine IP im lokalen Netzwerk angeben
IPAddress ip(192, 168, 69, 190);

// Ethernet Library als Server initialisieren verwendet die obige IP, Port ist per default 80
EthernetServer server(80);

// the setup function runs once when you press reset or power the board
void setup() 
{
	Serial.begin(9600);
	setupI2C();
	setupEthernet();
}

// the loop function runs over and over again until power down or reset
void setupI2C()
{
	Wire.begin();
	I2CwriteRegister8bit(sensor01, 6); //reset
}

void setupEthernet()
{
	// Ethernet Verbindung und Server starten
	Ethernet.begin(mac, ip);
	server.begin();
	Serial.print("Server gestartet. IP: ");
	// IP des Arduino-Servers ausgeben
	Serial.println(Ethernet.localIP());
}

void loop()
{
	EthernetClient client = server.available();
	
	if (client)
	{
		Serial.println("Neuer Client");
		// Jetzt solange Zeichen lesen, bis eine leere Zeile empfangen wurde
		// HTTP Requests enden immer mit einer leeren Zeile 
		boolean currentLineIsBlank = true;
		// Solange Client verbunden 
		while (client.connected())
		{
			// client.available() gibt die Anzahl der Zeichen zurück, die zum Lesen
			// verfügbar sind
			if (client.available())
			{
				// Ein Zeichen lesen und am seriellen Monitor ausgeben
				char c = client.read();
				Serial.write(c);
				// In currentLineIsBlank merken wir uns, ob diese Zeile bisher leer war.
				// Wenn die Zeile leer ist und ein Zeilenwechsel (das \n) kommt,
				// dann ist die Anfrage zu Ende und wir können antworten
				if (c == '\n' && currentLineIsBlank)
				{
					// HTTP Header 200 an den Browser schicken
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println("Connection: close"); // Verbindung wird nach Antwort beendet
					client.println("Refresh: 5"); // Seite alle 25 Sekunden neu abfragen
					client.println();
					// Ab hier berginnt der HTML-Code, der an den Browser geschickt wird
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");
					getI2C(client);
					client.println("</html>");
					break;
				}
				if (c == '\n')
				{
					// Zeilenwechsel, also currentLineIsBlack erstmal auf True setzen
					currentLineIsBlank = true;
				}
				else if (c != '\r')
				{
					// Zeile enthält Zeichen, also currentLineIsBlack auf False setzen
					currentLineIsBlank = false;
				}
			}
		}
		// Kleine Pause
		delay(1);
		// Verbindung schliessen
		client.stop();
		Serial.println("Verbindung mit Client beendet.");
		Serial.println("");
	}
}


void getI2C(EthernetClient client)
{
	int moisture = I2CreadRegister16bit(sensor01, 0);
	int temp = I2CreadRegister16bit(sensor01, 5);
	I2CwriteRegister8bit(sensor01, 3);
	int light = I2CreadRegister16bit(sensor01, 4);

	if (client)
	{
		client.print("Temperatur: <b>");
		client.print(temp);
		client.print("</b>Humidity: <b>");
		client.print(moisture);
		client.print("</b>Light: <b>");
		client.print(light);
		client.println("</b><br />");
	}
	
		Serial.print("Moist: ");
		Serial.print(moisture); //read capacitance register
		Serial.print(", Temp: ");
		Serial.print(temp); //temperature register
		Serial.print(", Light: ");
		Serial.println(light); //read light register 65535 = finster
	
}

void I2CwriteRegister8bit(int addr, int value) 
{
	Wire.beginTransmission(addr);
	Wire.write(value);
	Wire.endTransmission();
}

unsigned int I2CreadRegister16bit(int addr, int reg) 
{
	Wire.beginTransmission(addr);
	Wire.write(reg);
	Wire.endTransmission();
	delay(20);

	Wire.requestFrom(addr, 2);

	unsigned int t = Wire.read() << 8;

	t = t | Wire.read();

	return t;
}