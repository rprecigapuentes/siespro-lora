/*******************************************************************************************************
  Reliable LoRa Transmitter with AutoACK (ESP32)
  - Based on example 209_Reliable_Transmitter_AutoACK by Stuart Robinson.
  - Extended to:
      * Read local environmental data from the DHT11.
      * Output a CSV line per successful TX+ACK with:
            temp_C, hum_air_pct, rssi_dBm, snr_dB
      * Send the same data (sin humedad de suelo) as JSON via HTTPS POST to a REST API.
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>
#include <Arduino.h>
#include "DHT.h"

// ===================== WiFi / HTTP (API) =====================
// WiFi network credentials
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>   // For HTTPS

const char* ssid     = "Stee";
const char* password = "123456789";

// URL of the backend server (REST API endpoint, HTTPS)
const char* serverUrl = "https://siespro.onrender.com/sensors/data";

// ===================== LoRa / Radios =====================
// LoRa radio driver instance
SX127XLT LT;

// LoRa pin definitions for ESP32
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23

#define NSS        5     // LoRa chip-select (NSS)
#define NRESET     14    // LoRa reset pin
#define DIO0       2     // LoRa DIO0 interrupt pin

// LoRa device type and RF power
#define LORA_DEVICE DEVICE_SX1278
#define TXpower 10

// Reliable / AutoACK configuration parameters
#define ACKtimeout 1000
#define TXtimeout  1000
#define TXattempts 10

const uint16_t NetworkID = 0x3210;

// LoRa payload (sensor data is NOT sent over LoRa)
uint8_t  buff[] = "SIESPRO";
uint16_t PayloadCRC;
uint8_t  TXPacketL;

// ===================== DHT11 Sensor =====================
// Air humidity + temperature sensor
#define DHTPIN  17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===================== Last Sensor Sample (used for CSV & JSON) =====================
float lastT            = NAN;
float lastH            = NAN;
bool  lastSensorsValid = false;

int16_t AckRSSI = 0;   // RSSI of the received ACK packet (LoRa link)
int8_t  AckSNR  = 0;   // SNR of the received ACK packet (LoRa link)

// ===================== Forward Declarations =====================
void packet_is_OK();
void packet_is_Error();
void sendData(float tempC, float humAir, int rssi, float snr);

// ===================== Setup =====================
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Reliable LoRa Transmitter AutoACK + DHT11 (ESP32) + HTTPS API"));

  // Initialize sensors
  dht.begin();

  // Initialize LoRa SPI interface
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, NSS);

  // Initialize LoRa module
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
      delay(2000);  // Fatal error, stop program
    }
  }

  // Configure LoRa radio parameters (frequency, SF, BW, CR, LDRO)
  LT.setupLoRa(
      434000000,   // carrier frequency (Hz)
      0,           // frequency offset
      LORA_SF7,    // spreading factor
      LORA_BW_125, // bandwidth
      LORA_CR_4_5, // coding rate
      LDRO_AUTO    // low data rate optimization
  );

  Serial.println(F("Transmitter ready"));
  Serial.println();
  Serial.println(F("CSV format: temp_C,hum_air_pct,rssi_dBm,snr_dB"));
  Serial.println();

  // ===================== WiFi Setup =====================
  Serial.println(F("Configuring WiFi..."));

  // Begin WiFi connection (DHCP)
  WiFi.begin(ssid, password);
  Serial.print(F("Connecting to WiFi"));

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(F("."));

    // Timeout after 30 seconds
    if (millis() - start > 30000)
    {
      Serial.println();
      Serial.println(F("WiFi connection timeout"));
      break;
    }
  }

  // Report WiFi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println();
    Serial.println(F("WiFi connected"));
    Serial.print(F("ESP32 IP: "));
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println(F("WiFi NOT connected at the end of setup"));
  }

  Serial.println();
}

// ===================== Loop =====================
void loop()
{
  // ===================== Local Sensor Readings =====================
  // Read humidity and temperature from DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);  // Fahrenheit (unused but validated)

  Serial.println(F("=== Sensor readings ==="));

  // Validate DHT11 sensor data
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println(F("DHT11 read failed"));
    lastSensorsValid = false;
  }
  else
  {
    Serial.print(F("DHT11  | Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F(" °C, "));
    Serial.print(f);
    Serial.println(F(" °F"));

    // Store last valid sample for CSV + JSON output
    lastT            = t;
    lastH            = h;
    lastSensorsValid = true;
  }

  Serial.println();

  // ===================== Reliable Transmission with AutoACK =====================
  uint8_t attempts = TXattempts;
  TXPacketL = 0;

  do
  {
    Serial.print(F("Transmit payload > "));
    LT.printASCIIArray(buff, sizeof(buff));  // Debug print of payload
    Serial.println();
    Serial.flush();

    Serial.print(F("Send attempt "));
    Serial.println(TXattempts - attempts + 1);

    // Send packet and wait for AutoACK response
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
      // TX and ACK were successful
      PayloadCRC = LT.getTXPayloadCRC(TXPacketL);

      // Extract link-quality metrics from ACK (LoRa)
      AckRSSI = LT.readPacketRSSI();
      AckSNR  = LT.readPacketSNR();

      packet_is_OK();

      // ===================== CSV Output for Dataset =====================
      if (lastSensorsValid)
      {
        // CSV: temp_C,hum_air_pct,rssi_dBm,snr_dB
        Serial.println();
        Serial.print(lastT);
        Serial.print(F(","));
        Serial.print(lastH);
        Serial.print(F(","));
        Serial.print(AckRSSI);
        Serial.print(F(","));
        Serial.print(AckSNR);
        Serial.println();

        // ===================== JSON → HTTPS POST to API =====================
        sendData(lastT, lastH, AckRSSI, (float)AckSNR);
      }

      Serial.println();
    }
    else
    {
      // TX failed or no ACK received
      packet_is_Error();
      Serial.println();
    }

    delay(500);
  }
  while ((TXPacketL == 0) && (attempts != 0));

  // Final transmission status messages
  if (TXPacketL > 0)
  {
    Serial.println(F("Packet acknowledged"));
  }

  if (attempts == 0)
  {
    Serial.print(F("No acknowledge after "));
    Serial.print(TXattempts);
    Serial.print(F(" attempts"));
  }

  Serial.println();
  delay(5000);  // Main loop delay
}

// ===================== Helpers =====================
// Print successful packet information
void packet_is_OK()
{
  Serial.print(F("LocalNetworkID,0x"));
  Serial.print(NetworkID, HEX);
  Serial.print(F(",TransmittedPayloadCRC,0x"));
  Serial.print(PayloadCRC, HEX);
}

// Print error information and diagnostic flags from LoRa driver
void packet_is_Error()
{
  Serial.print(F("No packet acknowledge"));
  LT.printIrqStatus();
  LT.printReliableStatus();
}

// ===================== HTTP Sender =====================
// Sends JSON-encoded sensor and link data to the backend HTTPS API
void sendData(float tempC, float humAir, int rssi, float snr)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("Error: WiFi not connected, cannot send data to API"));
    return;
  }

  // Build JSON with the keys expected by the backend (sin humedad_suelo):
  // {
  //   "temperatura": 21,
  //   "humedad_relativa": 54.7,
  //   "rssi": -47,
  //   "snr": 9
  // }
  String jsonData = "{";
  jsonData += "\"temperatura\": "      + String(tempC, 2) + ",";
  jsonData += "\"humedad_relativa\": " + String(humAir, 2) + ",";
  jsonData += "\"rssi\": "             + String(rssi) + ",";
  jsonData += "\"snr\": "              + String(snr, 2);
  jsonData += "}";

  Serial.print(F("JSON to send: "));
  Serial.println(jsonData);

  WiFiClientSecure client;
  client.setInsecure();   // Disable certificate validation for demo purposes

  HTTPClient http;
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(jsonData);

  if (httpCode > 0)
  {
    Serial.print(F("HTTP POST OK, code: "));
    Serial.println(httpCode);

    String payload = http.getString();
    Serial.print(F("Server response: "));
    Serial.println(payload);
  }
  else
  {
    Serial.print(F("HTTP POST error, code: "));
    Serial.println(httpCode);
  }

  http.end();
}
