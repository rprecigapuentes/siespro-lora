# SIESPRO — Hardware Subsystem

[![ESP32](https://img.shields.io/badge/ESP32-Espressif-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32)
[![ESP32-C3](https://img.shields.io/badge/ESP32--C3_Mini-Espressif-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32-c3)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-IDE-orange?logo=platformio&logoColor=white)](https://platformio.org/)
[![Arduino](https://img.shields.io/badge/Framework-Arduino-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![LoRa](https://img.shields.io/badge/LoRa-SX1278_433_MHz-4B9CD3)](https://www.semtech.com/products/wireless-rf/lora-connect/sx1278)
[![Python](https://img.shields.io/badge/Python-3.10+-3776AB?logo=python&logoColor=white)](https://www.python.org/)
[![UNAL](https://img.shields.io/badge/UNAL-Bogotá-8B0000)](https://unal.edu.co/)

Embedded firmware for the SIESPRO perimeter monitoring system.
Implements a point-to-point LoRa network between a master node (ESP32) and a
slave node (ESP32-C3 Mini) using Reliable Transmission with AutoACK.
The link-quality metrics extracted from each ACK — RSSI and SNR — serve as
features for a Random Forest classifier that determines whether the transmitter
is inside or outside a defined perimeter.

> **Sensor data is never transmitted over LoRa.** The radio link is used solely
> as a measurement instrument to extract spatial RF features (RSSI, SNR).
> Environmental data (temperature, humidity, soil moisture) is read locally by
> the master node and sent directly to the backend API via HTTPS.

---

## System Architecture

```mermaid
flowchart LR
    subgraph Master ["Master Node (ESP32)"]
        DHT11["DHT11\ntemp / humidity"]
        HW080["HW-080\nsoil moisture"]
        MCU["ESP32"]
        SX_TX["SX1278\nLoRa TX"]

        DHT11 --> MCU
        HW080 --> MCU
        MCU --> SX_TX
    end

    subgraph Slave ["Slave Node (ESP32-C3 Mini)"]
        SX_RX["SX1278\nLoRa RX + AutoACK"]
    end

    subgraph Backend ["Backend"]
        API["REST API\nhttps://siespro.onrender.com"]
        RF["Random Forest\nclassifier"]
        API --> RF
    end

    SX_TX -->|"TX: SIESPRO\nNetworkID 0x3210\n434 MHz / SF7"| SX_RX
    SX_RX -->|"ACK: RSSI, SNR"| SX_TX
    MCU -->|"HTTPS POST\nJSON: temp, hum, rssi, snr"| API
```

---

## Repository Structure

| Folder | Target Hardware | Purpose |
|---|---|---|
| `basic_tests/` | ESP32 + ESP32-C3 Mini | Point-to-point link validation |
| `master_esp32/` | ESP32 | Main firmware — three operating modes |
| `slave_esp32_mini/` | ESP32-C3 Mini | AutoACK responder — passive |
| `sensors_esp32/` | ESP32 | Isolated sensor verification |
| `library/` | — | SX12XX-LoRa driver (Stuart Robinson) |

---

## Hardware Requirements

| Component | Model | Qty | Role |
|---|---|---|---|
| Microcontroller | ESP32 DevKit v1 | 1 | Master node |
| Microcontroller | ESP32-C3 Mini | 1 | Slave node |
| LoRa transceiver | SX1278 (433 MHz) | 2 | RF link |
| Air sensor | DHT11 | 1 | Temperature + humidity |
| Soil sensor | HW-080 | 1 | Soil moisture (optional) |

---

## Prerequisites

- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) (VSCode extension)
- Python 3.10+ — required only for `IA_config/dataset_tool/collect_dataset.py`
- `pyserial` — `pip install pyserial`

No additional library installation is needed. The SX12XX-LoRa driver is
vendored locally under `library/` and resolved automatically by PlatformIO
via `lib_extra_dirs` in each `platformio.ini`.

---

## Recommended Reading Order

Follow this order when setting up the system for the first time:

1. `library/` — understand the LoRa driver being used
2. `sensors_esp32/` — verify each sensor independently
3. `basic_tests/` — validate the LoRa link before adding logic
4. `slave_esp32_mini/` — flash the slave node (unchanged across modes)
5. `master_esp32/ACK_config/` — validate AutoACK end-to-end
6. `master_esp32/IA_config/` — collect the training dataset
7. `master_esp32/API_config/` — deploy online inference

