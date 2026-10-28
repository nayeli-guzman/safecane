// Fase 1, paso 4a — GPS, NMEA crudo (solo hardware: Wokwi no tiene GPS)
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
}
void loop() {
  while (Serial2.available()) Serial.write(Serial2.read());
}
