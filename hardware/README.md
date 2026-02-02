# Hardware Overview

This section documents the **hardware layer** of the SIESPRO project.  
It focuses exclusively on **LoRa-based embedded nodes**, sensor acquisition, and communication with the backend API.

The hardware system is built around **ESP32-family microcontrollers** and **SX1278 LoRa radios**, using the **SX12XX Reliable Packet library by Stuart Robinson** to ensure robust communication.

---

## Hardware Architecture

The network follows a **star topology**:

- One **master node**
- One or more **slave nodes**
- Slaves only listen and respond with automatic ACK frames
- All sensing, data processing, and external communication are handled by the master

LoRa communication is used **only to evaluate link quality**, not to transport sensor data.

---

## Basic Communication Tests

The hardware layer includes basic firmware examples intended to:

- Verify correct SPI wiring and radio configuration
- Validate LoRa transmission and reception
- Confirm reliable packet exchange using **AutoACK**

These tests use static payloads and are meant solely for hardware validation and link debugging.

---

## LoRa Communication Layer

All nodes rely on the **SX12XX Reliable Packet protocol**, which provides:

- Network ID filtering
- Automatic ACK handling
- Packet integrity via CRC
- RSSI and SNR extraction from received ACK frames

This abstraction allows the application logic to remain independent of low-level radio details.

---

## Sensor-Based Configurations

The system supports multiple operating modes depending on the deployment scenario.

---

### ACK Configuration (Link Validation)

- The master transmits a LoRa packet.
- The slave automatically responds with an ACK.
- No sensor data is transmitted via LoRa.
- Used for:
  - Range testing
  - Link stability analysis
  - RSSI and SNR characterization

---

### IA Configuration (Dataset Acquisition)

This mode is used to **collect labeled data for machine learning**.

- Sensors are connected to the **master node**:
  - Air temperature
  - Air humidity
- On each successful TX + ACK cycle:
  - The following data is produced locally via Serial:
    ```
    temp_C, hum_air_pct, rssi_dBm, snr_dB
    ```
- A Python script is used to:
  - Read the serial output
  - Label samples as `inside` or `outside` the perimeter
  - Generate a dataset for training a **Random Forest classifier**

LoRa is used exclusively to extract **link-quality metrics**.

---

### API Configuration (Online Inference)

In this mode, the master node performs **real-time inference via the backend API**.

- The master:
  - Reads sensor data
  - Sends a LoRa packet
  - Extracts RSSI and SNR from the ACK
- The collected data is sent to the backend via **HTTPS POST**.

#### Example JSON payload:
```json
{
  "temperatura": 21.5,
  "humedad_relativa": 54.2,
  "rssi": -47,
  "snr": 9.3
}
```
- The backend:
    
    - Executes the trained Random Forest model
        
    - Returns whether the node is **inside or outside the protected perimeter**
        

---

## Multi-Node Operation

The same logic extends naturally to configurations with:

- One master
    
- Multiple slaves
    

The master polls each slave sequentially, obtaining independent RSSI and SNR measurements per node, enabling spatial discrimination based on link quality.

---

## Hardware Summary

- **Microcontrollers**: ESP32, ESP32-B, ESP32-C3 Mini
    
- **Radio Module**: SX1278 (433 MHz)
    
- **Protocol**: LoRa Reliable Packets with AutoACK
    
- **Sensors**: DHT11 (air temperature and humidity)
    
- **ML Workflow**:
    
    - Offline data labeling and training
        
    - Online inference via REST API
        

This hardware layer is designed to be **modular**, **scalable**, and independent from frontend and backend implementations.