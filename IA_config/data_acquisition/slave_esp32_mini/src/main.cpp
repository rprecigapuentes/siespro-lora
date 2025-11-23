/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 02/03/20

  This program is supplied as is. It is up to the user to determine whether the program is suitable
  for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation
  -----------------
  This sketch implements a minimal LoRa receiver using the SX127XLT library. It listens for incoming
  packets using the LoRa settings configured in LT.setupLoRa().

  The received packet is assumed to contain ASCII-printable text. If the packet contains non-ASCII
  bytes (outside 0x20–0x7F), the serial output may show unexpected characters.

  Example output (valid packet):
      8s  Hello World 1234567890*,RSSI,-44dBm,SNR,9dB,Length,23,Packets,7,Errors,0,IRQreg,50

  Example output (CRC error):
      137s PacketError,RSSI,-89dBm,SNR,-8dB,Length,23,Packets,37,Errors,2,IRQreg,70,
           IRQ_HEADER_VALID,IRQ_CRC_ERROR,IRQ_RX_DONE

  Example timeout (no packets in 10 seconds):
      112s RXTimeout

  For more advanced receiver options see example "104_LoRa_Receiver".

  Serial monitor baud rate: 115200
*******************************************************************************************************/

#include <Arduino.h>        // Required for ESP32 and Arduino framework
#include <SPI.h>            // SX127x LoRa module communicates via SPI
#include <SX127XLT.h>       // SX127x LoRa driver library

SX127XLT LT;                // Create LoRa driver instance

// ===================== LoRa Pin Definitions (ESP32-C3 Mini) =====================
// These pins avoid strapping pins such as GPIO2, GPIO8, GPIO9.
#define LORA_SCK   10       // SPI SCK
#define LORA_MISO  6        // SPI MISO
#define LORA_MOSI  5        // SPI MOSI

#define NSS        7        // LoRa chip select (NSS / CS)
#define NRESET     3        // LoRa reset pin
#define DIO0       2        // LoRa DIO0 interrupt pin (RX/TX done)

#define LORA_DEVICE DEVICE_SX1278
#define RXBUFFER_SIZE 255   // Maximum payload size to receive

// ===================== Packet Statistics =====================
uint32_t RXpacketCount = 0;   // Total received packets
uint32_t errors = 0;          // Total packet errors

// ===================== RX Buffers and Metadata =====================
uint8_t RXBUFFER[RXBUFFER_SIZE];  // Buffer to hold received data
uint8_t RXPacketL;                // Length of last received packet
int16_t PacketRSSI;               // RSSI of received packet
int8_t  PacketSNR;                // SNR of received packet

// Function prototypes
void packet_is_OK();
void packet_is_Error();
void printElapsedTime();


/*******************************************************************************************************
  SETUP
*******************************************************************************************************/
void setup()
{
  Serial.begin(9600);
  delay(2000);    // Allow USB CDC to enumerate before printing
  Serial.println();
  Serial.println(F("4_LoRa_Receiver Starting"));
  Serial.println();

  // Initialize SPI with explicit pin mapping for ESP32-C3 Mini
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
      delay(2000);    // Prevent watchdog resets
    }
  }

  // Configure LoRa modem
  LT.setupLoRa(
      434000000,   // Frequency (Hz)
      0,           // Frequency offset
      LORA_SF7,    // Spreading factor
      LORA_BW_125, // Bandwidth
      LORA_CR_4_5, // Coding rate
      LDRO_AUTO    // Low-data-rate optimization
  );

  Serial.print(F("Receiver ready - RXBUFFER_SIZE "));
  Serial.println(RXBUFFER_SIZE);
  Serial.println();
}


/*******************************************************************************************************
  LOOP
*******************************************************************************************************/
void loop()
{
  // Wait up to 60 seconds (60000 ms) for a packet.
  RXPacketL = LT.receive(RXBUFFER, RXBUFFER_SIZE, 60000, WAIT_RX);

  PacketRSSI = LT.readPacketRSSI();   // Read RSSI of last received packet
  PacketSNR  = LT.readPacketSNR();    // Read SNR of last received packet

  if (RXPacketL == 0)
  {
    // Packet failed validation at LoRa hardware level (CRC / header)
    packet_is_Error();
  }
  else
  {
    packet_is_OK();
  }

  Serial.println();
}


/*******************************************************************************************************
  VALID PACKET HANDLER
*******************************************************************************************************/
void packet_is_OK()
{
  uint16_t IRQStatus;

  RXpacketCount++;
  IRQStatus = LT.readIrqStatus();      // Read IRQ flags from LoRa device
  printElapsedTime();                  // Print uptime in seconds

  Serial.print(F("  "));

  // Print received ASCII payload
  LT.printASCIIPacket(RXBUFFER, RXPacketL);

  // Append metadata: RSSI, SNR, packet length, counters, IRQ flags
  Serial.print(F(",RSSI,"));
  Serial.print(PacketRSSI);
  Serial.print(F("dBm,SNR,"));
  Serial.print(PacketSNR);
  Serial.print(F("dB,Length,"));
  Serial.print(RXPacketL);
  Serial.print(F(",Packets,"));
  Serial.print(RXpacketCount);
  Serial.print(F(",Errors,"));
  Serial.print(errors);
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);
}


/*******************************************************************************************************
  ERROR HANDLER
*******************************************************************************************************/
void packet_is_Error()
{
  uint16_t IRQStatus = LT.readIrqStatus();

  printElapsedTime();

  if (IRQStatus & IRQ_RX_TIMEOUT)
  {
    // No packet received before timeout
    Serial.print(F(" RXTimeout"));
  }
  else
  {
    // Packet received but failed CRC or header validation
    errors++;
    Serial.print(F(" PacketError"));
    Serial.print(F(",RSSI,"));
    Serial.print(PacketRSSI);
    Serial.print(F("dBm,SNR,"));
    Serial.print(PacketSNR);
    Serial.print(F("dB,Length,"));
    Serial.print(LT.readRXPacketL());
    Serial.print(F(",Packets,"));
    Serial.print(RXpacketCount);
    Serial.print(F(",Errors,"));
    Serial.print(errors);
    Serial.print(F(",IRQreg,"));
    Serial.print(IRQStatus, HEX);

    // Print human-readable names of IRQ flags
    LT.printIrqStatus();
  }
}


/*******************************************************************************************************
  PRINT ELAPSED TIME (SECONDS)
*******************************************************************************************************/
void printElapsedTime()
{
  float seconds = millis() / 1000.0;
  Serial.print(seconds, 0);
  Serial.print(F("s"));
}