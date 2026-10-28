// Fase 0 — blink
void setup() {
  pinMode(2, OUTPUT);          // LED azul integrado del DevKit (Wokwi no lo dibuja)
  Serial.begin(115200);
}

void loop() {
  digitalWrite(2, HIGH); delay(500);
  digitalWrite(2, LOW);  delay(500);
  Serial.println("ESP32 vivo");
}
