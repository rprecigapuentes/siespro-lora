/*******************************************************************************************************
  SIESPRO - HW-080 Soil Moisture Sensor Basic Test (ESP32)
  Reads raw ADC value and maps it to a 0–100% moisture scale.
  Use this sketch to verify wiring and calibrate min/max ADC values before integrating into master firmware.

  Calibration note:
    4092 = dry (0%)  →  0 = saturated (100%)
    Adjust these bounds based on your specific sensor and soil conditions.
*******************************************************************************************************/

#include <Arduino.h>

#define SOIL_PIN 33    // Analog input pin for HW-080

void setup()
{
  Serial.begin(115200);
  pinMode(SOIL_PIN, INPUT);
}

void loop()
{
  int soilPercent = map(analogRead(SOIL_PIN), 4092, 0, 0, 100);

  Serial.print(F("Soil moisture: "));
  Serial.print(soilPercent);
  Serial.println(F(" %"));

  delay(1000);
}
