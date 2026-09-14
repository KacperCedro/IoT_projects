#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <string>

#define I2C_SDA 8
#define I2C_SCL 9

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

void PrintData(float humidity, float temperature, float pressure);

void setup() {
  Serial.begin(9600);
  delay(2000);
  Serial.print("System startup...");
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
      Serial.println("BMP280 sensor not found");
    }
    
  }
  else{
    Serial.println("BMP280 found");
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, Adafruit_BMP280::SAMPLING_X2, Adafruit_BMP280::SAMPLING_X16, Adafruit_BMP280::FILTER_X16, Adafruit_BMP280::STANDBY_MS_500);

}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  PrintData(humidity.relative_humidity, bmp.readTemperature(), bmp.readPressure());
  delay(10000);
  /*
  Serial.print("Humidity (AHT20): ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  Serial.print("Temperature (BMP280): ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");
  
  Serial.print("Pressure (BMP280): ");
  Serial.print(bmp.readPressure() / 100.0F); // Devided by 100 to get hPa
  Serial.println(" hPa");

  Serial.println("-------------------");
  delay(3000);
  */
}
void PrintData(float humidity, float temperature, float pressure){ 
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
  Serial.print(pressure);  
  Serial.print("\n");
}
