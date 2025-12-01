//HW-080
#include <Arduino.h>  

const int humsuelo = 33;  
int valHumsuelo;

void setup() {
  Serial.begin(115200);
  pinMode(humsuelo, INPUT);
}

void loop() {
  //Convert value to percent
  valHumsuelo = map(analogRead(humsuelo), 4092, 0, 0, 100);

  //print value
  Serial.print("Humedad del suelo: ");
  Serial.print(valHumsuelo);
  Serial.println(" %");
}