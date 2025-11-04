#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

/*
  Slave node for simple polling with SX1278 on ESP32-C3 Super Mini.

  - Escucha continuamente.
  - Si recibe "POLL:A" (y MY_ID == 'A') o "POLL:B" (y MY_ID == 'B'),
    responde "OK:A" o "OK:B".
  - Config de RF debe coincidir con el master.
*/

// ===== Pines ESP32-C3 Super Mini ↔ SX1278 =====
#define LORA_SCK   4   // SCK
#define LORA_MISO  5   // MISO
#define LORA_MOSI  6   // MOSI
#define LORA_CS    7   // NSS/CS
#define LORA_RST   3   // RESET
#define LORA_DIO0  2   // DIO0 (RX done)

// Cambia a 'B' para el segundo slave si lo necesitas
#ifndef MY_ID
#define MY_ID 'A'
#endif

// Helper: enviar "OK:A" o "OK:B"
static void sendOk() {
  LoRa.beginPacket();
  if (MY_ID == 'A') {
    LoRa.print("OK:A");
  } else {
    LoRa.print("OK:B");
  }
  LoRa.endPacket();
}

void setup() {
  Serial.begin(115200);      // En ESP32-C3 es más cómodo a 115200
  delay(10);

  // Inicializa SPI con los pines elegidos
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

  // Indica a la lib los pines del LoRa
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  // Arranca el radio a 433 MHz
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa FAIL");
    while (true) { delay(100); }
  }

  // Debe coincidir con el master
  LoRa.setTxPower(17);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62500);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.enableCrc();

  Serial.print("Slave ready (ESP32-C3). MY_ID=");
  Serial.println((char)MY_ID);
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) {
    delay(10);
    return;
  }

  String msg;
  while (LoRa.available()) {
    msg += (char)LoRa.read();
  }

  if ((MY_ID == 'A' && msg == "POLL:A") ||
      (MY_ID == 'B' && msg == "POLL:B")) {
    sendOk();
    Serial.print("[RESP] OK:");
    Serial.println((char)MY_ID);
  } else {
    Serial.print("[IGN] ");
    Serial.println(msg);
  }
}
