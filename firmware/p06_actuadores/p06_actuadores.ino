// Fase 1, paso 6 — motor (rampa PWM), buzzer y LED
#define WOKWI 1   // 1 = simulador (buzzer con tono, motor = LED naranja); 0 = hardware real

const int MOTOR = 25, BUZZER = 26, LED = 33;

void motorInit() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(MOTOR, 5000, 8);            // core 3.x: pin, 5 kHz, 8 bits
#else
  ledcSetup(4, 5000, 8); ledcAttachPin(MOTOR, 4);
#endif
}

void motor(int duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(MOTOR, duty);
#else
  ledcWrite(4, duty);
#endif
}

void buzzer(bool on) {
#if WOKWI
  if (on) tone(BUZZER, 2000); else noTone(BUZZER);   // el buzzer de Wokwi necesita un tono
#else
  digitalWrite(BUZZER, on ? HIGH : LOW);             // el buzzer activo suena con nivel alto
#endif
}

void setup() {
  motorInit();
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  for (int d = 0; d <= 255; d += 5) { motor(d); delay(50); }
  motor(0);
  buzzer(true);  digitalWrite(LED, HIGH); delay(300);
  buzzer(false); digitalWrite(LED, LOW);  delay(700);
}
