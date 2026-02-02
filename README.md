# SIESPRO-LoRa

**SIESPRO-LoRa** is a LoRa-based student presence monitoring system designed for **educational institutions**, focused on **preventing unauthorized exits** while preserving **privacy and autonomy**.

The system provides a **non-intrusive alternative** to traditional location-tracking solutions by relying on **radio link characteristics** instead of explicit positioning, cameras, or personal identifiers.

---

## Problem Statement

Educational institutions face a recurring challenge:

- Detecting **unauthorized exits** of students from protected areas
- Doing so **without invasive surveillance**
- Avoiding dependence on:
  - GPS tracking
  - Continuous Wi-Fi connectivity
  - Cameras or biometric systems

Most existing solutions rely on **precise localization**, which introduces:
- Privacy concerns
- Infrastructure complexity
- High energy consumption
- Legal and ethical constraints

SIESPRO addresses this gap by rethinking **presence detection**, not localization.

---

## Project Objective

The main objective of SIESPRO is to **classify whether a node is inside or outside a predefined perimeter**, without computing its physical position.

This is achieved by combining:

- **LoRa radio link metrics**
- **Environmental sensor data**
- **Machine learning classification**

The result is a system that answers a single, relevant question:

> *Is this device inside or outside the authorized area?*

Nothing more. Nothing unnecessary.

---

## Core Design Principles

### Privacy by Design

- No GPS coordinates
- No triangulation
- No continuous tracking
- No personal identifiers
- No camera or audio data

The system does **not know where someone is**, only whether they are **inside or outside** a permitted zone.

---

### Low Infrastructure Dependency

- No local Wi-Fi network required for detection
- LoRa is used for:
  - Long-range communication
  - Low power consumption
  - Robust operation in complex environments

Wi-Fi is used **only at the master node** and **only when needed** to communicate with the backend API.

---

### Non-Intrusive Detection

- Devices behave as anonymous nodes
- Detection is based on:
  - RSSI
  - SNR
  - Environmental context
- No interaction required from the user

---

## System Overview

The system follows a **star topology**:

- One **master node**
- One or more **slave nodes**

### Node Roles

#### Slave Nodes
- Listen for LoRa packets
- Respond automatically with ACK frames
- Do not collect or transmit sensor data
- Do not connect to the internet

#### Master Node
- Polls slave nodes sequentially
- Collects:
  - RSSI and SNR from ACK responses
  - Environmental sensor data
- Sends aggregated data to the backend API
- Receives classification results (inside / outside)

---

## Detection Strategy

SIESPRO does **not perform localization**.

Instead, it performs **classification** based on:

- Radio link quality (RSSI, SNR)
- Environmental conditions (e.g., temperature, humidity)
- Learned patterns from real deployments

A **Random Forest model** is trained using labeled data collected on-site, allowing the system to adapt to:

- Building layout
- Materials
- Environmental noise
- Deployment-specific conditions

---

## Machine Learning Workflow

1. **Data Acquisition**
   - Master node logs sensor data and link metrics
   - Data is exported as CSV via Serial

2. **Labeling**
   - Samples are labeled as `inside` or `outside`
   - Labeling is performed externally using a Python script

3. **Training**
   - A Random Forest classifier is trained offline

4. **Inference**
   - The trained model runs on the backend API
   - The master node sends live data for classification

---

## Technology Stack

### Hardware
- ESP32 family microcontrollers
- SX1278 LoRa transceivers (433 MHz)
- Environmental sensors (e.g., DHT11)

### Communication
- LoRa Reliable Packets with AutoACK
- HTTPS REST API for backend communication

### Backend
- Random Forest classifier
- REST API for inference responses

---

## Intended Use Cases

- Schools and educational institutions
- Controlled academic environments
- Safety-oriented monitoring systems
- Scenarios where **privacy preservation is mandatory**

This project is **not intended for surveillance**, tracking, or behavioral monitoring.

---

## Project Scope

SIESPRO focuses on:

- Presence classification
- Privacy-aware system design
- Low-power, long-range communication
- Explainable and adaptable ML-based detection

Frontend visualization, user management, and policy enforcement are handled in **separate components** and repositories.

---

## Quick Start Tutorial

This tutorial is designed to understand and validate the system quickly.

### Requirements
- 2 ESP32-based boards (1 master, 1 slave)
- SX1278 LoRa modules
- Arduino IDE
- Python 3 (for labeling script)

---

### Step 1: Flash the Slave Node
- Upload the **Reliable Receiver AutoACK** firmware
- Verify:
  - LoRa initialization
  - Automatic ACK responses

Expected output:

```
Receiver ready
Payload received OK
```


---

### Step 2: Flash the Master Node
- Upload the **Reliable Transmitter + Sensors** firmware
- Connect a DHT11 sensor
- Verify:
  - Sensor readings
  - Successful TX + ACK cycles

Expected CSV output:

```
temp_C,hum_air_pct,rssi_dBm,snr_dB
```


---

### Step 3: Collect Dataset
- Move the slave node:
  - Inside the protected area
  - Outside the perimeter
- Collect Serial output
- Save data as CSV

---

### Step 4: Label and Train
- Use the provided Python script to:
  - Label data (`inside` / `outside`)
  - Train the Random Forest model

---

### Step 5: Online Inference
- Start the backend API
- Master node sends live data via HTTPS
- API returns classification result:
  - `inside`
  - `outside`

This completes the end-to-end demo.

---

## Summary

### Problem Addressed
- Student safety
- Unauthorized exits
- Privacy-preserving monitoring

### Target Audience
- Students
- Educational institutions
- School safety systems

### Why It Improves Student Life
- Prevents unsafe situations
- Avoids invasive surveillance
- Respects privacy and autonomy
- Operates in low-connectivity environments

### Submission Artifacts
- GitHub repository with source code
- Clear README and tutorial
- Optional demo video recommended

### Technology Scope
- Hardware + software system
- Beginner-friendly concept
- Modular and extensible design

### Intended Use Cases

- Schools
- Academic campuses
- Controlled educational environments

This project is **not intended for surveillance**, tracking, or behavioral analysis.

---

## Disclaimer

SIESPRO is a research and academic project.  
Its effectiveness depends on proper calibration, data collection, and ethical deployment.

The system is designed to **assist** institutional safety efforts, not replace human judgment.
