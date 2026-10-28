// Fase 1, paso 5 — LDR y pulsador
const int LDR = 32, BOTON = 27;

void setup() {
  Serial.begin(115200);
  pinMode(BOTON, INPUT_PULLUP);
  analogReadResolution(12);
}

void loop() {
  // Más luz = número más bajo (igual en Wokwi y en el circuito real)
  Serial.printf("ldr=%d  boton=%s\n", analogRead(LDR),
                digitalRead(BOTON) == LOW ? "PRESIONADO" : "suelto");
  delay(100);
}
