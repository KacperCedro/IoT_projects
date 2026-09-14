#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const String SSID = "ZC LTE";
const String PASSWORD = "11223344";
const IPAddress IP = IPAddress(0,0,0,0);
const int PORT = 5000;

WiFiUDP udp;

void setup() {
  Serial.begin(9600);
  Serial.println("System startup...");
  delay(2000);

  WiFi.begin(SSID,PASSWORD);
  Serial.print("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected.");

  udp.begin(IP, PORT);

  WiFi.softAP("Test_ESP32_P4", "12345678");
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    char Buffer[255];
    udp.read(Buffer,255);
    Buffer[packetSize]='\0';
    Serial.println(Buffer);
  }
  
}
