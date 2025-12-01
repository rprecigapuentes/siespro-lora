#include <Arduino.h>   
#include "DHT.h"

#define DHTPIN 17
#define DHTTYPE DHT11
//DHTTYPE = DHT11, but there are also DHT22 and 21

DHT dht(DHTPIN, DHTTYPE); // constructor to declare sensor

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  delay(1000);
  // The DHT11 returns at most one measurement every 1s
  float h = dht.readHumidity();
  //Read the moisture content in %.
  float t = dht.readTemperature();
  //Read the temperature in degrees Celsius
  float f = dht.readTemperature(true);
  // true returns the temperature in Fahrenheit

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed reception");
    return;
    //Returns an error if the ESP32 does not receive any measurements
  }

  Serial.print("Humidite: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  Serial.print("°C, ");
  Serial.print(f);
  Serial.println("°F");
  // Transmits the measurements received in the serial monitor
}