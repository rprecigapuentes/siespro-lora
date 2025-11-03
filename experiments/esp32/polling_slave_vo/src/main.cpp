#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("=== Prueba de monitor serial ===");
  Serial.println("Escribe algo en la consola y presiona Enter.");
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    Serial.print("Eco: ");
    Serial.println(data);
  }
  delay(50);
}
