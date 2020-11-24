#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Base64.h>

const char* ssid = "SuEwIsWlAn"; //Hier SSID eures WLAN Netzes eintragen
const char* password = "#My@WlaN$PasS#WorD!"; //Hier euer Passwort des WLAN Netzes eintragen

const char* host = "192.168.69.9"; //Server der die temperatur empfangen soll 
                                   //(er sollte feste IP haben)
const char* script = "/sensor/temperatur/"; //URL/Verzeichnis das wir gewaehlt haben

const char* passwort = "passwortxyz"; //Passwort, derzeit ungenutzt
const char* idnr = "1"; //Hier kann man dem Sensor eine beliebe ID zuteilen (derzeit ungenutzt)

#define BUILTIN_LED D0

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Wire.begin();
  Serial.begin(9600);
  Serial.println();
 Serial.print("Verbinde mich mit Netz: ");
 Serial.println(ssid);
 
 WiFi.begin(ssid, password);
 String s = WiFi.macAddress();
 Serial.println(s);

}
// the loop function runs over and over again forever
void loop() {
}


