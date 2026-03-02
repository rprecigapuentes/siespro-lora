/*******************************************************************************************************
  SIESPRO - LoRa Basic Link Test / Receiver (ESP32-C3 Mini)
  Minimum setup to verify point-to-point LoRa connectivity.
  Based on example 4_LoRa_Receiver by Stuart Robinson.

  Listens for incoming packets and reports RSSI, SNR and packet count via Serial.
  Use together with esp32/basic_test transmitter to validate link before
  deploying ACK, IA or API configurations.

  Expected Serial output (packet OK):
    8s  Hello World 1234567890*,RSSI,-44dBm,SNR,9dB,Length,23,Packets,7,Errors,0,IRQreg,50

  Expected Serial output (timeout):
    112s RXTimeout
*******************************************************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include <SX127XLT.h>

SX127XLT LT;

// ===================== SPI Pin Mapping (ESP32-C3 Mini) =====================
// Avoid strapping pins: GPIO2, GPIO8, GPIO9
#define LORA_SCK   10
#define LORA_MISO  5     // verify this matches your physical wiring
#define LORA_MOSI  6
#define NSS        7
#define NRESET     3
#define DIO0       2

#define LORA_DEVICE   DEVICE_SX1278
#define RXBUFFER_SIZE 255

uint32_t RXpacketCount;
uint32_t errors;

uint8_t RXBUFFER[RXBUFFER_SIZE];
uint8_t RXPacketL;
int16_t PacketRSSI;
int8_t  PacketSNR;

void packet_is_OK();
void packet_is_Error();
void printElapsedTime();


void setup()
{
  Serial.begin(115200);    // NOTE: was 9600 in original — must match transmitter monitor session
  delay(2000);             // allow USB-CDC to enumerate on ESP32-C3
  Serial.println();
  Serial.println(F("SIESPRO Basic Test - LoRa Receiver (ESP32-C3 Mini)"));
  Serial.println();

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
      434000000,    // must match transmitter exactly
      0,
      LORA_SF7,
      LORA_BW_125,
      LORA_CR_4_5,
      LDRO_AUTO
  );

  Serial.print(F("Receiver ready — RXBUFFER_SIZE "));
  Serial.println(RXBUFFER_SIZE);
  Serial.println();
}


void loop()
{
  RXPacketL = LT.receive(RXBUFFER, RXBUFFER_SIZE, 60000, WAIT_RX);

  PacketRSSI = LT.readPacketRSSI();
  PacketSNR  = LT.readPacketSNR();

  if (RXPacketL == 0)
    packet_is_Error();
  else
    packet_is_OK();

  Serial.println();
}


void packet_is_OK()
{
  uint16_t IRQStatus = LT.readIrqStatus();
  RXpacketCount++;

  printElapsedTime();
  Serial.print(F("  "));
  LT.printASCIIPacket(RXBUFFER, RXPacketL);

  Serial.print(F(",RSSI,"));    Serial.print(PacketRSSI); Serial.print(F("dBm"));
  Serial.print(F(",SNR,"));     Serial.print(PacketSNR);  Serial.print(F("dB"));
  Serial.print(F(",Length,"));  Serial.print(RXPacketL);
  Serial.print(F(",Packets,")); Serial.print(RXpacketCount);
  Serial.print(F(",Errors,"));  Serial.print(errors);
  Serial.print(F(",IRQreg,"));  Serial.print(IRQStatus, HEX);
}


void packet_is_Error()
{
  uint16_t IRQStatus = LT.readIrqStatus();

  printElapsedTime();

  if (IRQStatus & IRQ_RX_TIMEOUT)
  {
    Serial.print(F(" RXTimeout"));
  }
  else
  {
    errors++;
    Serial.print(F(" PacketError"));
    Serial.print(F(",RSSI,"));    Serial.print(PacketRSSI); Serial.print(F("dBm"));
    Serial.print(F(",SNR,"));     Serial.print(PacketSNR);  Serial.print(F("dB"));
    Serial.print(F(",Length,"));  Serial.print(LT.readRXPacketL());
    Serial.print(F(",Packets,")); Serial.print(RXpacketCount);
    Serial.print(F(",Errors,"));  Serial.print(errors);
    Serial.print(F(",IRQreg,"));  Serial.print(IRQStatus, HEX);
    LT.printIrqStatus();
  }
}


void printElapsedTime()
{
  Serial.print(millis() / 1000);
  Serial.print(F("s"));
}
