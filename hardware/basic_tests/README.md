# Basic Tests — LoRa Point-to-Point Link Validation

[![ESP32](https://img.shields.io/badge/TX-ESP32_DevKit-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32)
[![ESP32-C3](https://img.shields.io/badge/RX-ESP32--C3_Mini-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32-c3)
[![LoRa](https://img.shields.io/badge/LoRa-SX1278_433_MHz-4B9CD3)](https://www.semtech.com/products/wireless-rf/lora-connect/sx1278)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-IDE-orange?logo=platformio&logoColor=white)](https://platformio.org/)

Minimum firmware to verify SPI wiring and LoRa radio connectivity between the
ESP32 (transmitter) and the ESP32-C3 Mini (receiver) before deploying any
application logic. Run this test first whenever hardware changes are made.

> **This test does not use AutoACK or NetworkID filtering.**
> It is raw unidirectional transmission. For end-to-end AutoACK validation,
> proceed to `master_esp32/ACK_config/`.

---

## Structure

```
basic_tests/
├── esp32/ ← Transmitter (ESP32 DevKit)
│ ├── src/main.cpp
│ └── platformio.ini
└── esp32_mini/ ← Receiver (ESP32-C3 Mini)
├── src/main.cpp
└── platformio.ini
```

---

## Pin Mapping

### ESP32 — Transmitter

| Signal | GPIO |
|---|---|
| SCK | 18 |
| MISO | 19 |
| MOSI | 23 |
| NSS | 5 |
| NRESET | 14 |
| DIO0 | 2 |

### ESP32-C3 Mini — Receiver

| Signal | GPIO |
|---|---|
| SCK | 10 |
| MISO | 5 |
| MOSI | 6 |
| NSS | 7 |
| NRESET | 3 |
| DIO0 | 2 |

> GPIO2, GPIO8 and GPIO9 are strapping pins on the ESP32-C3 — avoid using
> them for LoRa signals. GPIO2 is assigned to DIO0 (interrupt input only,
> driven by the SX1278, not by the MCU at boot).

---

## LoRa Parameters

Both nodes must use identical RF settings:

| Parameter | Value |
|---|---|
| Frequency | 434 MHz |
| Spreading Factor | SF7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| TX Power | 10 dBm |

---

## How to Run

1. Open `esp32/` as a PlatformIO project and flash the transmitter.
2. Open `esp32_mini/` as a separate PlatformIO project and flash the receiver.
3. Open two Serial Monitor sessions simultaneously (115200 baud each).

**Transmitter output (expected):**

```
10dBm Packet> Hello World 1234567890* BytesSent,23 PacketsSent,6
```

**Receiver output (expected):**

```
8s Hello World 1234567890*,RSSI,-44dBm,SNR,9dB,Length,23,Packets,7,Errors,0,IRQreg,50
```

**Receiver timeout (no packets for 60 s):**

```
112s RXTimeout
```

---

## Pass Criteria

| Metric | Expected range |
|---|---|
| RSSI | > −100 dBm at short range |
| SNR | > 0 dB |
| Errors | 0 at < 10 m distance |