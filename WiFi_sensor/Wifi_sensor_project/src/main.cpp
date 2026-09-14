#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <string>
#include <FastLED.h>

#define I2C_SDA 8
#define I2C_SCL 9

#define NUM_LEDS 1
#define LED_PIN 48

CRGB leds[NUM_LEDS];

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
WiFiUDP udp;

const String ssid = "ZC LTE";
const String password = "11223344";
//const int hostIPlastDigits = 108;
const int port = 5000;
const uint64_t sleep_duration = 30 * 1000000ULL;

String WriteDataString (float temperature, float humidity, float pressure);

void setup() {
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  leds[0] = CRGB::Red;
  FastLED.setBrightness(15);
  FastLED.show();

  Serial.begin(9600);
  delay(1000);
  Serial.println("System startup...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");

  WiFi.setTxPower(WIFI_POWER_5dBm);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected.");

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!aht.begin(&Wire))
  {
    Serial.println("AHT20 sensor not found");
  }
  else{
    Serial.println("AHT20 found");
  }

  if (!bmp.begin(0x77, BMP280_CHIPID))
  {
    if (!bmp.begin(0x76, BMP280_CHIPID))
    {
      Serial.print("BMP280 sensor not found");
    }  
  }
  else{
    Serial.print("BMP280 found");
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, Adafruit_BMP280::SAMPLING_X2, Adafruit_BMP280::SAMPLING_X16, Adafruit_BMP280::FILTER_X16, Adafruit_BMP280::STANDBY_MS_500);

  delay(200);

  sensors_event_t temperature, humidity;
  aht.getEvent(&humidity, &temperature);

  String line = WriteDataString(bmp.readTemperature(), humidity.relative_humidity, bmp.readPressure());
  
  //IPAddress ip = IPAddress(192, 168, 1, hostIPlastDigits); 
  IPAddress ip = IPAddress(255,255,255,255);
  udp.beginPacket(ip, port);
  udp.print(line);
  udp.endPacket();
  delay(500);
  Serial.print("Data sent");

  esp_sleep_enable_timer_wakeup(sleep_duration);
  Serial.println("Good night everybody");
  FastLED.clear();
  FastLED.show();
  esp_deep_sleep_start();
}

void loop() {

}

String WriteDataString(float temperature, float humidity, float pressure){
  String output = String(temperature) + ',' + String(humidity) + ',' + String(pressure);
  return output;
}
