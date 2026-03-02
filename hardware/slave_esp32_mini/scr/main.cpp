/*******************************************************************************************************
  SIESPRO - LoRa Slave Node (ESP32-C3 Mini)
  Reliable packet reception with AutoACK using the SX12XX library by Stuart Robinson.
  Reference: example 210_Reliable_Receiver_AutoACK
  
  Role: Passive node. Listens for LoRa packets from the master and responds
        with automatic ACK frames. No sensor acquisition on this node.
        Link quality (RSSI, SNR) is extracted by the master from the ACK.
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>

SX127XLT LT;

// ===================== SPI Pin Mapping (ESP32-C3 Mini) =====================
#define LORA_SCK   10
#define LORA_MISO  5
#define LORA_MOSI  6
#define NSS        7
#define NRESET     3
#define DIO0       2

#define LORA_DEVICE DEVICE_SX1278

// ===================== Reliable Packet / AutoACK Parameters =====================
#define ACKdelay  100      // ms before sending ACK after valid reception
#define RXtimeout 60000    // ms to wait for incoming packet before timeout
#define TXpower   2        // ACK transmit power in dBm

const uint16_t NetworkID = 0x3210;  // Must match master node

const uint8_t RXBUFFER_SIZE = 251;
uint8_t RXBUFFER[RXBUFFER_SIZE];

uint8_t  RXPacketL;
uint8_t  RXPayloadL;
uint8_t  PacketOK;
int16_t  PacketRSSI;
uint16_t LocalPayloadCRC;
uint16_t RXPayloadCRC;
uint16_t TransmitterNetworkID;

// ===================== Forward Declarations =====================
void packet_is_OK();
void packet_is_Error();
void printPacketDetails();

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("SIESPRO Slave - Reliable LoRa AutoACK (ESP32-C3 Mini)"));

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

  Serial.println(F("Receiver ready"));
  Serial.println();
}

void loop()
{
  PacketOK = LT.receiveReliableAutoACK(
      RXBUFFER,
      RXBUFFER_SIZE,
      NetworkID,
      ACKdelay,
      TXpower,
      RXtimeout,
      WAIT_RX
  );

  RXPacketL  = LT.readRXPacketL();
  RXPayloadL = RXPacketL - 4;   // subtract 2B NetworkID + 2B PayloadCRC appended by library
  PacketRSSI = LT.readPacketRSSI();

  if (PacketOK > 0)
    packet_is_OK();
  else
    packet_is_Error();

  Serial.println();
}

void packet_is_OK()
{
  Serial.print(F("Payload OK > "));
  LT.printASCIIPacket(RXBUFFER, RXPayloadL);
  Serial.println();
  printPacketDetails();
  Serial.println();
}

void packet_is_Error()
{
  uint16_t IRQStatus = LT.readIrqStatus();
  Serial.print(F("Error "));

  if (IRQStatus & IRQ_RX_TIMEOUT)
    Serial.println(F("RX timeout"));
  else
    printPacketDetails();
}

void printPacketDetails()
{
  LocalPayloadCRC      = LT.CRCCCITT(RXBUFFER, RXPayloadL, 0xFFFF);
  TransmitterNetworkID = LT.getRXNetworkID(RXPacketL);
  RXPayloadCRC         = LT.getRXPayloadCRC(RXPacketL);

  Serial.print(F("LocalNetworkID,0x"));   Serial.print(NetworkID, HEX);
  Serial.print(F(",TransmitterNetworkID,0x")); Serial.print(TransmitterNetworkID, HEX);
  Serial.print(F(",LocalPayloadCRC,0x")); Serial.print(LocalPayloadCRC, HEX);
  Serial.print(F(",RXPayloadCRC,0x"));    Serial.print(RXPayloadCRC, HEX);

  LT.printReliableStatus();
}