# Library — SX12XX-LoRa

[![SX1278](https://img.shields.io/badge/LoRa-SX1278_433_MHz-4B9CD3)](https://www.semtech.com/products/wireless-rf/lora-connect/sx1278)
[![Author](https://img.shields.io/badge/Author-Stuart_Robinson-lightgrey)](https://github.com/StuartsProjects)

This folder contains the [SX12XX-LoRa](https://github.com/StuartsProjects/SX12XX-LoRa)
library by Stuart Robinson, vendored locally to ensure reproducible builds
across all project configurations without requiring an internet connection
at compile time.

---

## Attribution

All code under this folder is authored by **Stuart Robinson** and distributed
under its original license. No modifications have been made to the library source.

> Stuart Robinson, *SX12XX-LoRa Arduino Library*, 2020.
> https://github.com/StuartsProjects/SX12XX-LoRa

---

## Examples Used as Reference

| Project example | Used in |
|---|---|
| `3_LoRa_Transmitter` | `basic_tests/esp32/` |
| `4_LoRa_Receiver` | `basic_tests/esp32_mini/` |
| `209_Reliable_Transmitter_AutoACK` | `master_esp32/` (all modes) |
| `210_Reliable_Receiver_AutoACK` | `slave_esp32_mini/` |

---

## Why This Library

- Native **NetworkID filtering**: discards packets from foreign networks at hardware level
- **AutoACK**: slave responds with an ACK containing RSSI and SNR of the received packet
- Built-in **CRC validation** and retransmission logic
- `readPacketRSSI()` / `readPacketSNR()` expose link-quality metrics from the ACK frame — the core feature used for RF-based localization in this project
