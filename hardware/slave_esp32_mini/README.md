# Slave Node — ESP32-C3 Mini

[![ESP32-C3](https://img.shields.io/badge/ESP32--C3_Mini-Espressif-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32-c3)
[![LoRa](https://img.shields.io/badge/LoRa-SX1278_433_MHz-4B9CD3)](https://www.semtech.com/products/wireless-rf/lora-connect/sx1278)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-IDE-orange?logo=platformio&logoColor=white)](https://platformio.org/)
[![Reference](https://img.shields.io/badge/Reference-Stuart_Robinson_ex210-lightgrey)](https://github.com/StuartsProjects/SX12XX-LoRa)

Passive AutoACK responder for the SIESPRO LoRa network.
The slave node listens for packets matching the project `NetworkID (0x3210)`,
and automatically sends an ACK containing its own RSSI and SNR measurements
back to the master. It carries no sensors and requires no configuration
changes across operating modes.

---

## Role in the System

```
Master (ESP32) ──[TX: "SIESPRO", NetworkID=0x3210]──► Slave (ESP32-C3 Mini)
Master (ESP32) ◄──[ACK: RSSI, SNR]──────────────────── Slave (ESP32-C3 Mini)
```


The RSSI and SNR values embedded in the ACK are the spatial RF features
extracted by the master to feed the Random Forest classifier.

---

## Structure

```
slave_esp32_mini/
├── src/
│ └── main.cpp
└── platformio.ini
```


This firmware is **single and shared** across all master operating modes
(ACK_config, IA_config, API_config). Flash once; no changes needed.

---

## Pin Mapping (ESP32-C3 Mini)

| Signal | GPIO | Note |
|---|---|---|
| SCK | 10 | |
| MISO | 5 | |
| MOSI | 6 | |
| NSS | 7 | |
| NRESET | 3 | |
| DIO0 | 2 | Interrupt input — driven by SX1278, not MCU |

> GPIO2, GPIO8 and GPIO9 are hardware strapping pins on the ESP32-C3.
> The SX1278 drives DIO0 only after boot is complete, so GPIO2 is safe
> as an interrupt input but must not be pulled externally at startup.

---

## LoRa Parameters

Must match the master node exactly:

| Parameter | Value |
|---|---|
| NetworkID | `0x3210` |
| Frequency | 434 MHz |
| Spreading Factor | SF7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| ACK Timeout | 1000 ms |

---

## Flash Instructions

```console
# From slave_esp32_mini/ directory
pio run --target upload
pio device monitor --baud 115200
```

Expected Serial output on each received packet:

```
LocalNetworkID,0x3210,ReceivedPayloadCRC,0x<CRC>,RSSI,-<n>dBm,SNR,<n>dB
```

Packets with a non-matching NetworkID are silently discarded by the library.