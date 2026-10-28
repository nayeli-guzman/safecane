// Fase 1, paso 3 — FC-51 (detector de desnivel)
const int IR_OUT = 34;   // OUT del FC-51: LOW = ve el piso, HIGH = no ve nada

void setup() {
  Serial.begin(115200);
  pinMode(IR_OUT, INPUT);  // el FC-51 trae su propia resistencia pull-up
}

void loop() {
  static unsigned long tUltimoPiso = 0;
  bool piso = digitalRead(IR_OUT) == LOW;
  if (piso) tUltimoPiso = millis();
  Serial.printf("piso=%d  sin_piso_hace=%lu ms\n", piso, millis() - tUltimoPiso);
  delay(50);
}
