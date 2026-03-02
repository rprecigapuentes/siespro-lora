/*******************************************************************************************************
  SIESPRO - DHT11 Sensor Basic Test (ESP32)
  Reads air temperature and relative humidity.
  Use this sketch to verify wiring and sensor response before integrating into master firmware.
*******************************************************************************************************/

#include <Arduino.h>
#include "DHT.h"

#define DHTPIN  17
#define DHTTYPE DHT11     // Compatible with DHT22 and DHT21 — change type here if needed

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(115200);
  dht.begin();
}

void loop()
{
  delay(1000);    // DHT11 maximum sampling rate: 1 Hz

  float h = dht.readHumidity();
  float t = dht.readTemperature();       // Celsius
  float f = dht.readTemperature(true);   // Fahrenheit

  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println(F("DHT11 read failed — check wiring and pin definition"));
    return;
  }

  Serial.print(F("Humidity: "));    Serial.print(h);    Serial.print(F("%  "));
  Serial.print(F("Temp: "));        Serial.print(t);    Serial.print(F(" °C / "));
  Serial.print(f);                  Serial.println(F(" °F"));
}
