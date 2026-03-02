# Sensor Tests — ESP32

[![ESP32](https://img.shields.io/badge/ESP32-Espressif-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-IDE-orange?logo=platformio&logoColor=white)](https://platformio.org/)
[![DHT11](https://img.shields.io/badge/Sensor-DHT11-green)](https://components101.com/sensors/dht11-temperature-sensor)
[![HW-080](https://img.shields.io/badge/Sensor-HW--080-green)](https://components101.com/modules/soil-moisture-sensor-module)

Isolated sensor verification firmware. Use these sketches to confirm correct
wiring and sensor response independently from the LoRa stack before integrating
sensors into the master node firmware.

---

## Structure

```
sensors_esp32/
├── dht11/
│ ├── src/main.cpp
│ └── platformio.ini
└── hw080/
├── src/main.cpp
└── platformio.ini
```


---

## DHT11 — Air Temperature & Humidity

**Pin:** GPIO 17

**Expected Serial output:**

```
Humidity: 54.00% Temp: 21.00 °C / 69.80 °F
```


**Common failure modes:**

| Output | Cause |
|---|---|
| `DHT11 read failed` repeatedly | Wrong GPIO, missing pull-up, or damaged sensor |
| Fixed value that never changes | Sensor not powered (3.3 V required) |
| Values update slower than 1 s | Normal — DHT11 maximum sampling rate is 1 Hz |

> A 10 kΩ pull-up resistor between the data line and VCC is required.
> Most breakout modules include it; bare sensors do not.

---

## HW-080 — Soil Moisture

**Pin:** GPIO 33 (ADC1 — required; ADC2 is unavailable when WiFi is active)

**Expected Serial output:**

```
Soil moisture: 42%
```


**Calibration:**

The `map()` function uses fixed ADC bounds:

```cpp
map(analogRead(SOIL_PIN), 4092, 0, 0, 100)
//                         ^^^^  ^
//                         dry   saturated
```

These values are empirical for a 12-bit ESP32 ADC. If readings at known
dry/wet states do not reach 0% or 100%, adjust the bounds:

1. Read raw ADC with sensor fully dry in air → use that value as the 4092 bound
2. Read raw ADC with sensor fully submerged → use that value as the 0 bound
3. Update both arguments in map() accordingly

>GPIO 33 is input-only on the ESP32 — do not use it for output.
>ADC readings are nonlinear near the rails; values below ~5% and above ~95%
>may be unreliable without additional calibration.
