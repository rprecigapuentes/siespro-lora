#include <Arduino.h>
#include <SPI.h>
#include <SX127XLT.h>

SX127XLT LT;

#define NSS     7
#define NRESET  3
#define DIO0    2
#define LORA_DEVICE DEVICE_SX1278

#define ACKdelay  100
#define RXtimeout 60000
#define TXpower   2

const uint8_t RXBUFFER_SIZE = 251;
uint8_t RXBUFFER[RXBUFFER_SIZE];

uint8_t  RXPacketL;
uint8_t  RXPayloadL;
uint8_t  PacketOK;
int16_t  PacketRSSI;
uint16_t LocalPayloadCRC;
uint16_t RXPayloadCRC;
uint16_t TransmitterNetworkID;

const uint16_t NetworkID = 0x3211;

// ---------- PROTOTIPOS ----------
void packet_is_OK();
void packet_is_Error();
void printPacketDetails();
// --------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("210_Reliable_Receiver_AutoACK Starting"));

  // ESP32-C3: SPI.begin(SCK, MISO, MOSI, SS)
  SPI.begin(4, 5, 6, NSS);

  if (LT.begin(NSS, NRESET, DIO0, LORA_DEVICE)) {
    Serial.println(F("LoRa Device found"));
    delay(1000);
  } else {
    Serial.println(F("No LoRa device responding"));
    while (1) { delay(1000); }
  }

  LT.setupLoRa(433500000, 0, LORA_SF7, LORA_BW_125, LORA_CR_4_5, LDRO_AUTO);
  Serial.println(F("Receiver ready"));
  Serial.println();
}

void loop()
{
  PacketOK = LT.receiveReliableAutoACK(
      RXBUFFER, RXBUFFER_SIZE, NetworkID, ACKdelay, TXpower, RXtimeout, WAIT_RX);

  RXPacketL  = LT.readRXPacketL();
  RXPayloadL = RXPacketL - 4;
  PacketRSSI = LT.readPacketRSSI();

  if (PacketOK > 0) {
    packet_is_OK();
  } else {
    packet_is_Error();
  }
  Serial.println();
}

void packet_is_OK()
{
  Serial.print(F("Payload received OK > "));
  LT.printASCIIPacket(RXBUFFER, RXPayloadL);
  Serial.println();
  printPacketDetails();
  Serial.println();
}

void packet_is_Error()
{
  uint16_t IRQStatus = LT.readIrqStatus();
  Serial.print(F("Error "));
  if (IRQStatus & IRQ_RX_TIMEOUT) {
    Serial.print(F(" RXTimeout "));
  } else {
    printPacketDetails();
  }
}

void printPacketDetails()
{
  LocalPayloadCRC      = LT.CRCCCITT(RXBUFFER, RXPayloadL, 0xFFFF);
  TransmitterNetworkID = LT.getRXNetworkID(RXPacketL);
  RXPayloadCRC         = LT.getRXPayloadCRC(RXPacketL);

  Serial.print(F("LocalNetworkID,0x"));
  Serial.print(NetworkID, HEX);
  Serial.print(F(",TransmitterNetworkID,0x"));
  Serial.print(TransmitterNetworkID, HEX);
  Serial.print(F(",LocalPayloadCRC,0x"));
  Serial.print(LocalPayloadCRC, HEX);
  Serial.print(F(",RXPayloadCRC,0x"));
  Serial.print(RXPayloadCRC, HEX);
  LT.printReliableStatus();
}
