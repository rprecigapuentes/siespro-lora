/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 02/03/20

  This program is supplied as is. It is up to the user to determine whether the program is suitable
  for the intended application and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation
  -----------------
  This sketch reads environmental data from two sensors (DHT11 and HW-080 soil moisture sensor)
  and transmits them over LoRa as a CSV-formatted packet.

  CSV payload format:
      temperature_C,humidity_relative,soil_moisture

  Example:
      24.50,60.00,37

  The LoRa packet is transmitted using the SX127XLT library and printed over Serial for inspection.

  Serial monitor baud rate: 115200
*******************************************************************************************************/

#include <SPI.h>              // LoRa device uses the SPI bus
#include <SX127XLT.h>         // SX127x LoRa library
#include <Arduino.h>
#include "DHT.h"

SX127XLT LT;                  // LoRa driver instance

// ===================== LoRa Pin Definitions (ESP32) =====================
#define LORA_SCK   18         // SPI SCK pin
#define LORA_MISO  19         // SPI MISO pin
#define LORA_MOSI  23         // SPI MOSI pin

#define NSS        5          // LoRa chip-select (NSS / CS)
#define NRESET     14         // LoRa reset pin
#define DIO0       2          // LoRa DIO0: TX/RX done detection

#define LORA_DEVICE DEVICE_SX1278
#define TXpower 10            // LoRa transmit power in dBm

// ===================== DHT11 Sensor =====================
#define DHTPIN  17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===================== HW-080 Soil Moisture Sensor =====================
const int humsuelo = 33;      // Analog input pin for soil moisture sensor
int valHumsuelo;

// ===================== LoRa Packet Buffers =====================
uint8_t TXPacketL;            // Packet length
uint32_t TXPacketCount;       // Counter for transmitted packets

uint8_t buff[64];             // Buffer for CSV payload


// ===================== Forward Declarations =====================
void packet_is_OK();
void packet_is_Error();


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("LoRa TX - DHT11 + HW-080 CSV"));

  // Initialize DHT11 sensor
  dht.begin();

  // Initialize HW-080 sensor
  pinMode(humsuelo, INPUT);

  // Initialize SPI for LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, NSS);

  // Initialize LoRa device
  if (LT.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa device detected"));
    delay(1000);
  }
  else
  {
    Serial.println(F("No LoRa device detected"));
    while (1)
    {
      delay(2000); // Prevent watchdog reset on ESP32
    }
  }

  // Configure LoRa modem parameters
  LT.setupLoRa(
      434000000,   // Frequency: 434 MHz
      0,           // Frequency offset
      LORA_SF7,    // Spreading factor
      LORA_BW_125, // Bandwidth: 125 kHz
      LORA_CR_4_5, // Coding rate
      LDRO_AUTO    // Low data rate optimization
  );

  Serial.println(F("Transmitter ready"));
}


void loop()
{
  delay(1000);  // DHT11 maximum sampling rate: 1 Hz

  // -------------------- Read DHT11 --------------------
  float h = dht.readHumidity();         // Relative humidity
  float t = dht.readTemperature();      // Temperature in °C
  float f = dht.readTemperature(true);  // Temperature in °F

  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT11");
    return;  // Do not transmit invalid readings
  }

  // -------------------- Read HW-080 Soil Sensor --------------------
  valHumsuelo = map(analogRead(humsuelo), 4092, 0, 0, 100);

  // -------------------- Print sensor values --------------------
  Serial.print("DHT11 -> Humidity: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  Serial.print(" °C, ");
  Serial.print(f);
  Serial.println(" °F");

  Serial.print("HW-080 -> Soil moisture: ");
  Serial.print(valHumsuelo);
  Serial.println(" %");

  // -------------------- Prepare CSV payload --------------------
  // Format: temperature_C,humidity_relative,soil_moisture
  int len = snprintf((char *)buff, sizeof(buff), "%.2f,%.2f,%d", t, h, valHumsuelo);
  if (len <= 0 || len >= (int)sizeof(buff))
  {
    Serial.println(F("CSV formatting error"));
    return;
  }

  TXPacketL = (uint8_t)len;

  Serial.print(TXpower);
  Serial.print(F(" dBm  Packet CSV> "));
  Serial.println((char *)buff);

  // Debug output: show ASCII packet as seen by LoRa
  LT.printASCIIPacket(buff, TXPacketL);

  // -------------------- Transmit packet --------------------
  if (LT.transmit(buff, TXPacketL, 10000, TXpower, WAIT_TX))
  {
    TXPacketCount++;
    packet_is_OK();
  }
  else
  {
    packet_is_Error();
  }

  Serial.println();
}


// ===================== Packet OK Handler =====================
void packet_is_OK()
{
  Serial.print(F("  BytesSent,"));
  Serial.print(TXPacketL);
  Serial.print(F("  PacketsSent,"));
  Serial.print(TXPacketCount);
}


// ===================== Packet Error Handler =====================
void packet_is_Error()
{
  uint16_t IRQStatus = LT.readIrqStatus();  // Read LoRa IRQ register

  Serial.print(F(" SendError, Length,"));
  Serial.print(TXPacketL);
  Serial.print(F(", IRQreg,"));
  Serial.print(IRQStatus, HEX);

  // Print human-readable IRQ flags
  LT.printIrqStatus();
}
