/*******************************************************************************************************
  SIESPRO - LoRa Basic Link Test / Transmitter (ESP32)
  Minimum setup to verify point-to-point LoRa connectivity.
  Based on example 3_LoRa_Transmitter by Stuart Robinson.

  Sends a fixed ASCII packet every 1 second and reports TX status via Serial.
  Use together with esp32_mini/basic_test receiver to validate link before
  deploying ACK, IA or API configurations.

  Expected Serial output:
    10dBm Packet> Hello World 1234567890*  BytesSent,23  PacketsSent,6
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>

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

uint8_t  TXPacketL;
uint32_t TXPacketCount;

uint8_t buff[] = "Hello World 1234567890";

void packet_is_OK();
void packet_is_Error();


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("SIESPRO Basic Test - LoRa Transmitter (ESP32)"));

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
}


void loop()
{
  TXPacketL = sizeof(buff);
  buff[TXPacketL - 1] = '*';    // replace null terminator so payload is fully visible on receiver

  Serial.print(TXpower);
  Serial.print(F("dBm  Packet> "));
  LT.printASCIIPacket(buff, TXPacketL);

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
  delay(1000);
}


void packet_is_OK()
{
  Serial.print(F("  BytesSent,"));   Serial.print(TXPacketL);
  Serial.print(F("  PacketsSent,")); Serial.print(TXPacketCount);
}


void packet_is_Error()
{
  uint16_t IRQStatus = LT.readIrqStatus();
  Serial.print(F("  SendError — Length,"));
  Serial.print(TXPacketL);
  Serial.print(F("  IRQreg,"));
  Serial.print(IRQStatus, HEX);
  LT.printIrqStatus();
}
