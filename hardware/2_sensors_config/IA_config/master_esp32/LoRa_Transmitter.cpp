/*******************************************************************************************************
  Reliable LoRa Transmitter with AutoACK (ESP32)
  - Based on example 209_Reliable_Transmitter_AutoACK by Stuart Robinson.
  - Extended to:
      * Read local environmental data from DHT1 sensor.
      * Output a CSV line per successful TX+ACK with:
          temp_C, hum_air_pct, rssi_dBm, snr_dB
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>
#include <Arduino.h>
#include "DHT.h"

SX127XLT LT;

// ===================== LoRa Pin Definitions (ESP32) =====================
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23

#define NSS        5
#define NRESET     14
#define DIO0       2

#define LORA_DEVICE DEVICE_SX1278
#define TXpower 10

// ===================== Reliable / AutoACK Configuration =====================
#define ACKtimeout 1000
#define TXtimeout  1000
#define TXattempts 10

const uint16_t NetworkID = 0x3210;

// ===================== LoRa Payload (does NOT carry sensor data) =====================
uint8_t  buff[] = "Hello World";
uint16_t PayloadCRC;
uint8_t  TXPacketL;

// ===================== DHT11 Sensor =====================
#define DHTPIN  17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===================== Last Sensor Sample (for CSV) =====================
float lastT       = NAN;
float lastH       = NAN;
bool  lastSensorsValid = false;

int16_t AckRSSI = 0;
int8_t  AckSNR  = 0;

// ===================== Forward Declarations =====================
void packet_is_OK();
void packet_is_Error();


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Reliable LoRa Transmitter AutoACK + Sensors (ESP32)"));

  dht.begin();

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, NSS);

  if (LT.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa device found"));
    delay(1000);
  }
  else
  {
    Serial.println(F("No LoRa device responding"));
    while (1)
    {
      delay(2000);
    }
  }

  LT.setupLoRa(
      434000000,   // carrier frequency (Hz)
      0,           // offset
      LORA_SF7,    // spreading factor
      LORA_BW_125, // bandwidth
      LORA_CR_4_5, // coding rate
      LDRO_AUTO    // low data rate optimization
  );

  Serial.println(F("Transmitter ready"));
  Serial.println();
  Serial.println(F("CSV format: temp_C,hum_air_pct,rssi_dBm,snr_dB"));
  Serial.println();
}


void loop()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    lastSensorsValid = false;
  } else {
    lastT = t;
    lastH = h;
    lastSensorsValid = true;
  }

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

      if (lastSensorsValid)
      {
        // única línea que imprime
        Serial.print(lastT, 2);
        Serial.print(",");
        Serial.print(lastH, 2);
        Serial.print(",");
        Serial.print(AckRSSI);
        Serial.print(",");
        Serial.println(AckSNR);
      }
    }

    delay(500);
  }
  while ((TXPacketL == 0) && (attempts != 0));

  delay(5000);
}

void packet_is_OK()
{
  Serial.print(F("LocalNetworkID,0x"));
  Serial.print(NetworkID, HEX);
  Serial.print(F(",TransmittedPayloadCRC,0x"));
  Serial.print(PayloadCRC, HEX);
}


void packet_is_Error()
{
  Serial.print(F("No packet acknowledge"));
  LT.printIrqStatus();
  LT.printReliableStatus();
}
