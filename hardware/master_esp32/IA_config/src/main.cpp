/*******************************************************************************************************
  SIESPRO - LoRa Master Node / Dataset Acquisition Mode (ESP32)
  Reliable transmission with AutoACK using the SX12XX library by Stuart Robinson.
  Reference: example 209_Reliable_Transmitter_AutoACK

  Role: Master node. Reads local sensors, transmits a LoRa packet to the slave,
        and extracts link-quality metrics (RSSI, SNR) from the received ACK.
        On each successful TX+ACK cycle, outputs a single CSV line via Serial
        consumed by collect_dataset.py for offline Random Forest training.

  Active sensor config: 2 sensors — DHT11 (temperature + humidity)
  Serial output format: temp_C,hum_air_pct,rssi_dBm,snr_dB

  To enable 3-sensor config (+ HW-080 soil moisture):
    uncomment all lines marked with [3S]
    Output becomes: temp_C,hum_air_pct,soil_moisture_pct,rssi_dBm,snr_dB
    Note: collect_dataset.py must also have [3S] lines enabled.
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>
#include <Arduino.h>
#include "DHT.h"

SX127XLT LT;

// ===================== SPI Pin Mapping (ESP32) =====================
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define NSS        5
#define NRESET     14
#define DIO0       2

#define LORA_DEVICE DEVICE_SX1278
#define TXpower     10

// ===================== Reliable Packet / AutoACK Parameters =====================
#define ACKtimeout 1000    // ms to wait for ACK after transmission
#define TXtimeout  1000    // ms timeout for TX operation
#define TXattempts 10      // max retransmission attempts before giving up

const uint16_t NetworkID = 0x3210;  // Must match slave node

// ===================== LoRa Payload =====================
// LoRa is used only for link quality evaluation (RSSI, SNR from ACK).
// Sensor data is NOT transported via LoRa.
uint8_t  buff[] = "Hello World";
uint16_t PayloadCRC;
uint8_t  TXPacketL;

// ===================== DHT11 — Air Temperature & Humidity =====================
#define DHTPIN  17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===================== [3S] HW-080 — Soil Moisture =====================
// [3S] const int soilSensorPin = 33;
// [3S] int       soilPercent   = 0;

// ===================== Last Valid Sensor Sample =====================
float lastT            = NAN;
float lastH            = NAN;
// [3S] int lastSoil    = 0;
bool  lastSensorsValid = false;

int16_t AckRSSI = 0;
int8_t  AckSNR  = 0;


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("SIESPRO Master - LoRa Dataset Acquisition (ESP32)"));

  dht.begin();
  // [3S] pinMode(soilSensorPin, INPUT);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, NSS);

  if (LT.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa device found"));
    delay(1000);
  }
  else
  {
    Serial.println(F("No LoRa device responding"));
    while (1) { delay(2000); }
  }

  LT.setupLoRa(
      434000000,    // carrier frequency (Hz) — SX1278 433 MHz band
      0,            // frequency offset
      LORA_SF7,     // spreading factor
      LORA_BW_125,  // bandwidth
      LORA_CR_4_5,  // coding rate
      LDRO_AUTO     // low data rate optimization
  );

  Serial.println(F("Transmitter ready"));
  Serial.println();
  Serial.println(F("Output: temp_C,hum_air_pct,rssi_dBm,snr_dB"));
  // [3S] Serial.println(F("Output: temp_C,hum_air_pct,soil_moisture_pct,rssi_dBm,snr_dB"));
  Serial.println();
}


void loop()
{
  // ===================== Sensor Readings =====================
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    lastSensorsValid = false;
  }
  else
  {
    // [3S] soilPercent = map(analogRead(soilSensorPin), 4092, 0, 0, 100);
    lastT = t;
    lastH = h;
    // [3S] lastSoil = soilPercent;
    lastSensorsValid = true;
  }

  // ===================== Reliable Transmission with AutoACK =====================
  uint8_t attempts = TXattempts;
  TXPacketL = 0;

  do
  {
    TXPacketL = LT.transmitReliableAutoACK(
        buff,
        sizeof(buff),
        NetworkID,
        ACKtimeout,
        TXtimeout,
        TXpower,
        WAIT_TX
    );
    attempts--;

    if (TXPacketL > 0)
    {
      AckRSSI = LT.readPacketRSSI();
      AckSNR  = LT.readPacketSNR();

      // ===================== CSV Output — single line for collect_dataset.py =====================
      if (lastSensorsValid)
      {
        Serial.print(lastT, 2);   Serial.print(F(","));
        Serial.print(lastH, 2);   Serial.print(F(","));
        // [3S] Serial.print(lastSoil); Serial.print(F(","));
        Serial.print(AckRSSI);    Serial.print(F(","));
        Serial.println(AckSNR);
      }
    }

    delay(500);
  }
  while ((TXPacketL == 0) && (attempts != 0));

  delay(5000);
}
