///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Version:
// 0.1 --> 2017/07/09 --> Initial build of code and first testrun on the device
// 0.2 --> 2017/07/10 --> removed lot of const and changed to defines, added DEBUG sections and myPrint
// 0.3 --> 2020/11/22 --> changed MQTT server IP
// 1.0 --> 2020/11/23 --> transfer and recoded for visual studio code
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <Wire.h>
#include <Base64.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <myMQTT.h>

const char* ssid         = "SuEwIsWlAn";          //Hier SSID eures WLAN Netzes eintragen
const char* password     = "#My@WlaN$PasS#WorD!"; //Hier euer Passwort des WLAN Netzes eintragen
const char* mqtt_server  = "192.168.70.3";       // ioBroker MQTT Server IP

// define the MQTT nodes for the communication with server
#define mqtt_Name   "NodeMCU01"

// compiler block removals by commenting the whole line out
#define Humidity // comment if no Humidity Sensor attached
#define DEBUG    // comment if no SerialOutput required

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg  = 0;
long lastPing = 0;
int  value    = 0;
int  myPing   = 0;
char msg[50];

void myPrint(String in,boolean newLine = false){                        // use this output to console instead so that DEBUG is working
    #if defined DEBUG
    if (newLine) {
        Serial.println(in);
    }
    else {
        Serial.print(in);
    }
    #endif
}
void setupPINMode() {                                                   // setup all the usage of PINS
    pinMode(2, OUTPUT); // Initialize the BUILTIN_LED pin as an output
    //pinMode(D3, OUTPUT); // Initialize the BUILTIN_LED pin as an output
    //pinMode(D4, OUTPUT); // Initialize the BUILTIN_LED pin as an output
    //pinMode(D5, OUTPUT); // Initialize the BUILTIN_LED pin as an output
    //pinMode(D6, OUTPUT); // Initialize the BUILTIN_LED pin as an output
}
void setup_wifi(){                                                      // setup of the wifi
    delay(10);
    myPrint("Connecting to ");
    myPrint(ssid,true);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        myPrint(".");
    }
    
    myPrint("WiFi connected");
    myPrint("IP address: ");
    myPrint(WiFi.localIP().toString(),true);
}
#if defined Humidity
void writeI2CRegister8bit(int addr, int value){                         // wire write to get command
    Wire.beginTransmission(addr);
    Wire.write(value);
    Wire.endTransmission();
}
unsigned int readI2CRegister16bit(int addr, int reg){                   // wire read communication
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission();
    delay(20);
    Wire.requestFrom(addr, 2);
    unsigned int t = Wire.read() << 8;
    t = t | Wire.read();
    return t;
}
void doHumiSensor(){                                                    // liest alle Sensor Daten aus dem Feuchtigkeitssensor aus und übergibt diese an den MQTT Server
    //read capacitive register
    myPrint("Capacity: ");
    snprintf (msg, 75, "%d", readI2CRegister16bit(0x20, 0));
    client.publish(mqtt_1CAP, msg);
    myPrint(msg); 
    
    //read temperature register
    myPrint(",Temp: ");
    snprintf (msg, 75, "%d", readI2CRegister16bit(0x20, 5));
    int temp = int(msg);
    client.publish(mqtt_2TMP, msg);
    myPrint(msg); 
    if (temp <30)
    {
        digitalWrite(2, LOW);
    }
    else
    {
        digitalWrite(2, HIGH);
    }
    

    //read light register
    myPrint(",Light: ");
    writeI2CRegister8bit(0x20, 3); //request light measurement
    delay(3000);
    snprintf (msg, 75, "%d", readI2CRegister16bit(0x20, 4));
    client.publish(mqtt_3LIG, msg);
    myPrint(msg,true); 
}
#endif

void callback(char* topic, byte* payload, unsigned int length){         // MQTT callback function
    unsigned int i = 0;
    char message_buff[100];
    
    myPrint("Empfangen: " + String(topic));
    for (i = 0; i < length; i++){                                       // single step through buffer
        message_buff[i] = payload[i];
    }
    message_buff[i] = '\0';
    String msgString = String(message_buff); // Konvertierung der nachricht in ein String
    myPrint(" -> " + msgString,true);

    // Switch on the LED if an 1 was received as first character
    if (msgString == "D00") {    digitalWrite(2, LOW); } // Turn the LED on (Note that LOW is the voltage level
    if (msgString == "D01") {    digitalWrite(2, HIGH);} // Turn the LED off by making the voltage HIGH
    //if (msgString == "D30") {    digitalWrite(D3, LOW);          } // Turn the LED on (Note that LOW is the voltage level
    //if (msgString == "D31") {    digitalWrite(D3, HIGH);         } // Turn the LED on (Note that LOW is the voltage level

    client.publish(mqtt_IN, "IN read");
    delay(1000);
    client.publish(mqtt_IN, "");
}
void reconnect(){                                                       // Loop until we're reconnected
    while (!client.connected())
    {
        myPrint("Attempting MQTT connection...");
        if (client.connect(mqtt_Name, "admin", "Sup3rS4ndr401"))
        {
            myPrint("connected",true);
            client.publish(mqtt_OUT, "connected");
            client.subscribe(mqtt_IN);
        }
        else
        {
            myPrint("failed, rc=");
            myPrint(String(client.state()));
            myPrint("try again in 5 seconds",true);
            delay(5000);
        }
    }
}

void setup(){                                                           // general setup of connections
    setupPINMode();
 //   digitalWrite(BUILTIN_LED, HIGH);
    Wire.begin();
    Serial.begin(9600);
    #if defined Humidity
    writeI2CRegister8bit(0x20, 6); //reset
    #endif
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop(){                                                            // general loop
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    // jede Minute
    long now = millis();
    if (now - lastPing > 60000)
    {
        lastPing = now;
        myPing++;
        snprintf (msg, 75, "%d", myPing);
        client.publish(mqtt_9PNG, msg);
    }
#if defined Humidity
    if (now - lastMsg > 60000)
    {
        lastMsg = now;
        doHumiSensor();
    }
#endif
}
