"""
SIESPRO - Dataset Acquisition Tool
Reads CSV lines from the master ESP32 via Serial and writes labeled samples
to a CSV file for offline Random Forest training.

Active sensor config: 2 sensors — DHT11 (temperature + humidity)
Expected Serial format: temp_C,hum_air_pct,rssi_dBm,snr_dB

To enable 3-sensor config (+ HW-080 soil moisture):
  uncomment all lines marked with [3S]
  Expected format becomes: temp_C,hum_air_pct,soil_moisture_pct,rssi_dBm,snr_dB
  Note: firmware must also have [3S] lines enabled.

Controls:
  s         start recording
  p         pause recording
  [Space]   toggle label 0 ↔ 1  (0 = outside perimeter, 1 = inside)
  q         quit

Output CSV columns (2S): timestamp_iso, temp_C, hum_aire_pct, rssi_dBm, snr_dB, label
Output CSV columns (3S): timestamp_iso, temp_C, hum_aire_pct, hum_tierra_pct, rssi_dBm, snr_dB, label
"""

import serial
import csv
from datetime import datetime
import sys
import select
import termios
import tty
import os
import time

# ===================== Serial Configuration =====================
PORT     = "/dev/ttyUSB0"   # [3S] was /dev/ttyACM0 in 3-sensor tests
BAUD     = 115200            # [3S] was 9600 in 3-sensor tests; must match firmware
CSV_FILE = "mediciones_loRa.csv"

# ===================== Sampling =====================
SAMPLE_INTERVAL = 3.0        # seconds between accepted samples (matches firmware delay)
# [3S] SAMPLE_INTERVAL = 0    # 3S firmware has no sleep; remove throttle if needed

# ===================== Expected field count =====================
EXPECTED_FIELDS = 4          # temp, hum_air, rssi, snr
# [3S] EXPECTED_FIELDS = 5   # temp, hum_air, soil, rssi, snr


def get_key_nonblocking():
    dr, _, _ = select.select([sys.stdin], [], [], 0)
    if dr:
        return sys.stdin.read(1)
    return None


def write_header(file_obj, writer):
    """Write CSV header if file is empty."""
    file_obj.seek(0, os.SEEK_END)
    if file_obj.tell() == 0:
        writer.writerow([
            "timestamp_iso",
            "temp_C",
            "hum_aire_pct",
            # [3S] "hum_tierra_pct",
            "rssi_dBm",
            "snr_dB",
            "label"
        ])
        file_obj.flush()


def parse_line(line):
    """
    Parse a CSV line from Serial.
    Returns a tuple of floats or None on failure.
    Expected (2S): temp, hum_air, rssi, snr
    Expected (3S): temp, hum_air, soil, rssi, snr
    """
    parts = line.split(",")
    if len(parts) != EXPECTED_FIELDS:
        return None
    try:
        return tuple(float(p) for p in parts)
    except ValueError:
        return None


def build_row(timestamp, values, label):
    """Build CSV row from parsed values."""
    temp, hum_air, rssi, snr = values          # 2S
    # [3S] temp, hum_air, soil, rssi, snr = values
    return [
        timestamp,
        temp,
        hum_air,
        # [3S] soil,
        rssi,
        snr,
        label
    ]


def main():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    tty.setcbreak(fd)

    try:
        with serial.Serial(PORT, BAUD, timeout=1) as ser, \
             open(CSV_FILE, "a+", newline="") as f:

            writer = csv.writer(f)
            write_header(f, writer)

            recording = False
            label     = 0
            print("Controls: s=start  p=pause  q=quit  [Space]=toggle label")
            print(f"Label: {label}  |  File: {CSV_FILE}")

            while True:
                key = get_key_nonblocking()
                if key:
                    if key == "s":
                        recording = True
                        print("Recording: START")
                    elif key == "p":
                        recording = False
                        print("Recording: PAUSED")
                    elif key == "q":
                        print("Exiting...")
                        break
                    elif key == " ":
                        label = 1 - label
                        print(f"Label → {label}  (0=outside  1=inside perimeter)")

                if not recording:
                    continue

                raw = ser.readline()
                if not raw:
                    continue

                line = raw.decode("utf-8", errors="ignore").strip()
                if not line:
                    continue

                values = parse_line(line)
                if values is None:
                    print(f"Skipped (invalid): {line}")
                    continue

                timestamp = datetime.now().isoformat()
                row = build_row(timestamp, values, label)
                writer.writerow(row)
                f.flush()

                print(f"{timestamp}  {values}  label={label}")

                time.sleep(SAMPLE_INTERVAL)

    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)


if __name__ == "__main__":
    main()
