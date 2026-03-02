
# Master Node — ESP32

[![ESP32](https://img.shields.io/badge/ESP32-Espressif-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32)
[![LoRa](https://img.shields.io/badge/LoRa-SX1278_433_MHz-4B9CD3)](https://www.semtech.com/products/wireless-rf/lora-connect/sx1278)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-IDE-orange?logo=platformio&logoColor=white)](https://platformio.org/)
[![Python](https://img.shields.io/badge/Python-3.10+-3776AB?logo=python&logoColor=white)](https://www.python.org/)
[![DHT11](https://img.shields.io/badge/Sensor-DHT11-green)](https://components101.com/sensors/dht11-temperature-sensor)
[![HW-080](https://img.shields.io/badge/Sensor-HW--080-green)](https://components101.com/modules/soil-moisture-sensor-module)

Central node of the SIESPRO system. Reads environmental sensors, maintains a
LoRa link with the slave node to extract RF spatial features (RSSI, SNR from
each ACK), and operates in one of three selectable modes depending on the
project phase.

---

## Operating Modes

| Mode | Folder | Output | Purpose |
|---|---|---|---|
| **ACK** | `ACK_config/` | Serial debug | End-to-end AutoACK link validation |
| **IA** | `IA_config/` | CSV via Serial | Offline dataset acquisition for RF training |
| **API** | `API_config/` | HTTPS POST | Online inference — sends data to backend REST API |

Each mode is a standalone PlatformIO project. Flash only the mode needed for
the current project phase. The slave node firmware does not change across modes.

---

## Structure

```
master_esp32/
├── ACK_config/
│ ├── src/main.cpp
│ └── platformio.ini
├── IA_config/
│ ├── dataset_tool/
│ │ ├── collect_dataset.py
│ │ ├── mediciones_loRa_[2s].csv ← generated at runtime
│ │ └── mediciones_loRa_[3s].csv ← generated at runtime
│ ├── src/main.cpp
│ └── platformio.ini
└── API_config/
├── src/
│ ├── credentials.h ← not committed (see below)
│ └── main.cpp
└── platformio.ini
```


---

## Sensor Configuration

All modes support two sensor configurations selectable at compile time:

| Config | Sensors | CSV columns |
|---|---|---|
| **2S** (default, active) | DHT11 | `temp_C, hum_air_pct, rssi_dBm, snr_dB` |
| **3S** (commented `[3S]`) | DHT11 + HW-080 | `temp_C, hum_air_pct, soil_pct, rssi_dBm, snr_dB` |

To switch to 3S: uncomment all lines marked `[3S]` in both `src/main.cpp`
and (for IA_config) `dataset_tool/collect_dataset.py`. Both files must use
the same config simultaneously.

---

## Pin Mapping

| Signal | GPIO |
|---|---|
| LoRa SCK | 18 |
| LoRa MISO | 19 |
| LoRa MOSI | 23 |
| LoRa NSS | 5 |
| LoRa NRESET | 14 |
| LoRa DIO0 | 2 |
| DHT11 data | 17 |
| HW-080 analog | 33 |

---

## WiFi Credentials (API_config only)

Credentials are **not** stored in `main.cpp`. Create the following file
before building `API_config/`:

**`API_config/src/credentials.h`**
```cpp
#pragma once
#define WIFI_SSID     "your_ssid"
#define WIFI_PASSWORD "your_password"
```

Add it to .gitignore at the repo root:

```
hardware/master_esp32/API_config/src/credentials.h
```

## IA_config — Dataset Tool

IA_config/dataset_tool/collect_dataset.py reads CSV lines from the master
via Serial and writes labeled samples to disk for offline Random Forest training.

```
cd IA_config/dataset_tool
pip install pyserial
python collect_dataset.py
```

### Keyboard controls:

| Key   | Action             |
| ----- | ------------------ |
| s     | Start recording    |
| p     | Pause recording    |
| Space | Toggle label 0 ↔ 1 |
| q     | Quit               |

Label semantics: 0 = outside perimeter, 1 = inside perimeter.

>The script filters out any Serial line that does not match the expected
>field count (4 for 2S, 5 for 3S). Debug prints in firmware are suppressed
>in IA_config intentionally to avoid parse errors.

## Recommended Phase Sequence

1. ACK_config   →  verify AutoACK link is stable
2. IA_config    →  collect labeled dataset (both classes, multiple positions)
3. API_config   →  deploy trained model to backend, begin online inference
