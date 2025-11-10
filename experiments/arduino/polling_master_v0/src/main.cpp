#include <SPI.h>
#include <LoRa.h>

/*
  Master node for simple polling of two slaves (A and B) using SX1278.
  Behavior:
    - Alternates sending "POLL:A" and "POLL:B".
    - Waits up to RX_TIMEOUT_MS for "OK:A" or "OK:B" respectively.
    - Prints RSSI and SNR when the expected reply arrives.
    - Ignores any other frames during the wait window.

  Notes:
    - RF params match the slave to avoid desync.
    - Keep Serial at 9600 to match provided setup logs.
*/

// SX1278 pins on Arduino UNO
const int csPin    = 10; // NSS
const int resetPin = 9;  // RESET
const int irqPin   = 3;  // DIO0 (RX done)

// Poll messages
const char* POLL_A = "POLL:A";
const char* POLL_B = "POLL:B";

// Poll cadence and RX timeout
const unsigned long POLL_INTERVAL_MS = 8000; // 8 s between polls
const unsigned long RX_TIMEOUT_MS    = 1500; // 1.5 s wait for reply

// Sends a poll and waits for a specific "OK:X" tag.
// Returns true on matching reply within timeout, false otherwise.
bool pollTarget(const char* pollMsg, const char* expectOkTag) {
  // TX: begin a new packet and send the poll text
  LoRa.beginPacket();
  LoRa.print(pollMsg);
  LoRa.endPacket();

  Serial.print("[TX] ");
  Serial.println(pollMsg);

  // RX wait window
  unsigned long t0 = millis();
  while (millis() - t0 < RX_TIMEOUT_MS) {
    // parsePacket() is non-blocking; returns packet size when something arrives
    int packetSize = LoRa.parsePacket();
    if (!packetSize) {
      delay(5); // small back-off to avoid busy-wait
      continue;
    }

    // Read entire packet payload into a String
    String msg = "";
    while (LoRa.available()) {
      msg += (char)LoRa.read();
    }

    // Validate exact match with expected OK tag
    if (msg == expectOkTag) {
      int rssi   = LoRa.packetRssi(); // RSSI reported by chip for this packet
      float snr  = LoRa.packetSnr();  // SNR metric
      Serial.print("[RX] ");
      Serial.print(msg);
      Serial.print(" | RSSI=");
      Serial.print(rssi);
      Serial.print(" dBm | SNR=");
      Serial.println(snr);
      return true;
    } else {
      // Anything else is ignored by design in this prototype
      Serial.print("[RX ignored] ");
      Serial.println(msg);
    }
  }

  // Timeout: no matching reply in the allowed window
  Serial.println("[TIMEOUT] No response from expected node.");
  return false;
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

  // Common RF configuration (must match slaves)
  LoRa.setTxPower(17);           // 2–20 dBm; 17 is a reasonable default
  LoRa.setSpreadingFactor(12);   // High range, low data rate
  LoRa.setSignalBandwidth(62500);// 62.5 kHz BW
  LoRa.setCodingRate4(8);        // 4/8 coding rate
  LoRa.setSyncWord(0x12);        // Private network sync word
  LoRa.setPreambleLength(8);     // Preamble symbols
  LoRa.enableCrc();              // Enable CRC to filter noise

  Serial.println("Master ready (poll A/B every 8 s).");
}

void loop() {
  // Poll slave A
  pollTarget(POLL_A, "OK:A");
  delay(POLL_INTERVAL_MS);

  // Poll slave B
  pollTarget(POLL_B, "OK:B");
  delay(POLL_INTERVAL_MS);
}
