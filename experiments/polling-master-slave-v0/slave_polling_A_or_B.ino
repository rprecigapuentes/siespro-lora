#include <SPI.h>
#include <LoRa.h>

/*
  Slave node for simple polling with SX1278.
  Behavior:
    - Listens continuously.
    - When a packet arrives, reads full payload as text.
    - If the payload equals "POLL:A" (and MY_ID == 'A') or "POLL:B" (and MY_ID == 'B'),
      replies with "OK:A" or "OK:B" respectively.
    - Logs ignored frames for visibility.

  Usage:
    - Flash this same sketch to two boards.
    - Set MY_ID to 'A' on one and 'B' on the other before flashing.

  Notes:
    - RF params must match the master's configuration.
    - Keep Serial at 9600 to align with logs used in tests.
*/

// SX1278 pins on Arduino UNO
const int csPin    = 10; // NSS
const int resetPin = 9;  // RESET
const int irqPin   = 2;  // DIO0 (RX done)

// Set this to 'A' for slave A, 'B' for slave B before flashing
const char MY_ID = 'A';  // change to 'B' on the second slave

// Helper: send "OK:A" or "OK:B" depending on MY_ID
void sendOk() {
  LoRa.beginPacket();
  if (MY_ID == 'A') {
    LoRa.print("OK:A");
  } else {
    LoRa.print("OK:B");
  }
  LoRa.endPacket();
}

void setup() {
  Serial.begin(9600);

  // Wire SX1278 pins
  LoRa.setPins(csPin, resetPin, irqPin);

  // Initialize radio at 433 MHz
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa FAIL");
    while (1) { /* halt */ }
  }

  // Match master's RF configuration
  LoRa.setTxPower(17);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62500);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.enableCrc();

  Serial.print("Slave ready. MY_ID=");
  Serial.println(MY_ID);
}

void loop() {
  // Non-blocking check for incoming packet
  int packetSize = LoRa.parsePacket();
  if (!packetSize) {
    // No packet yet; small rest to avoid busy loop
    delay(10);
    return;
  }

  // Read entire packet payload into a String
  String msg = "";
  while (LoRa.available()) {
    msg += (char)LoRa.read();
  }

  // If this packet is a poll addressed to this slave, reply with OK
  if ((MY_ID == 'A' && msg == "POLL:A") ||
      (MY_ID == 'B' && msg == "POLL:B")) {
    sendOk();
    Serial.print("[RESP] OK:");
    Serial.println(MY_ID);
  } else {
    // Ignore any other frames (including other node polls)
    Serial.print("[IGN] ");
    Serial.println(msg);
  }
}
