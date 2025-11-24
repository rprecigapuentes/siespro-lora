import serial
import csv
from datetime import datetime
import sys
import select
import termios
import tty
import os

# Configuración del puerto serie
PORT = "/dev/ttyACM0"   # ajusta esto
BAUD = 9600             # ajusta según tu ESP32
CSV_FILE = "mediciones_loRa.csv"


def get_key_nonblocking():
    """
    Lee una tecla sin bloquear el programa.
    Devuelve un carácter o None si no se presionó nada.
    """
    dr, _, _ = select.select([sys.stdin], [], [], 0)
    if dr:
        return sys.stdin.read(1)
    return None


def ensure_header(file_obj):
    """
    Escribe el encabezado del CSV si el archivo está vacío.
    Columnas:
    - timestamp_iso: marca de tiempo
    - temp_C: temperatura aire [°C]
    - hum_aire_pct: humedad relativa aire [%]
    - hum_tierra_pct: humedad suelo [%]
    - rssi_dBm: potencia recibida [dBm]
    - snr_dB: SNR [dB]
    - label: etiqueta binaria (0/1)
    """
    file_obj.seek(0, os.SEEK_END)
    if file_obj.tell() == 0:
        writer = csv.writer(file_obj)
        writer.writerow([
            "timestamp_iso",
            "temp_C",
            "hum_aire_pct",
            "hum_tierra_pct",
            "rssi_dBm",
            "snr_dB",
            "label"
        ])
        file_obj.flush()


def main():
    # Guardar configuración del terminal y ponerlo en modo cbreak
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    tty.setcbreak(fd)

    try:
        with serial.Serial(PORT, BAUD, timeout=1) as ser, \
             open(CSV_FILE, "a+", newline="") as f:

            ensure_header(f)
            writer = csv.writer(f)

            midiendo = False
            label = 0  # etiqueta binaria inicial
            print("Controles: s = start, p = pause, q = quit, [Espacio] = toggle label (0/1)")
            print(f"Label actual: {label}")

            while True:
                # Teclas
                key = get_key_nonblocking()
                if key:
                    if key == "s":
                        midiendo = True
                        print("Medición: INICIO")
                    elif key == "p":
                        midiendo = False
                        print("Medición: PAUSA")
                    elif key == "q":
                        print("Saliendo...")
                        break
                    elif key == " ":
                        # Alternar etiqueta 0 ↔ 1
                        label = 1 - label
                        print(f"Label cambiado a: {label}")

                if not midiendo:
                    continue

                # Leer una línea del puerto serie
                raw = ser.readline()
                if not raw:
                    continue

                linea = raw.decode("utf-8", errors="ignore").strip()
                if not linea:
                    continue

                # Esperamos formato: temp,hum_aire,hum_tierra,rssi,snr
                partes = linea.split(",")
                if len(partes) != 5:
                    print(f"Línea inválida, ignorada: {linea}")
                    continue

                try:
                    temp = float(partes[0])
                    hum_aire = float(partes[1])
                    hum_tierra = float(partes[2])
                    rssi = float(partes[3])
                    snr = float(partes[4])
                except ValueError:
                    print(f"No se pudo parsear: {linea}")
                    continue

                timestamp = datetime.now().isoformat()

                # Escribir fila en CSV con label
                writer.writerow([timestamp, temp, hum_aire, hum_tierra, rssi, snr, label])
                f.flush()

                # Eco en pantalla
                print(timestamp, temp, hum_aire, hum_tierra, rssi, snr, f"label={label}")

    finally:
        # Restaurar configuración del terminal
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)


if __name__ == "__main__":
    main()
