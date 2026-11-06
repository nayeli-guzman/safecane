// Prueba del buzzer de 2 pines con 2N2222: descubre si es activo o pasivo
// Cableado: GPIO 26 → 1 kΩ → base (B); buzzer (−) → colector (C); emisor (E) → GND;
// buzzer (+), pata larga → 5V/VIN; 1N4007 en paralelo con el buzzer, franja hacia 5 V.
// Abre el Monitor Serie a 115200.
const int BUZZER = 26;

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
}

void loop() {
  Serial.println("1) Nivel ALTO ... si suena: es ACTIVO (el firmware sirve tal cual)");
  digitalWrite(BUZZER, HIGH); delay(1500);
  digitalWrite(BUZZER, LOW);  delay(1000);

  Serial.println("2) Tono de 2 kHz ... si SOLO suena aquí: es PASIVO");
  tone(BUZZER, 2000); delay(1500);
  noTone(BUZZER); digitalWrite(BUZZER, LOW); delay(1000);

  Serial.println("--- (si nada suena: revisa el transistor y el cableado) ---\n");
  delay(1000);
}
