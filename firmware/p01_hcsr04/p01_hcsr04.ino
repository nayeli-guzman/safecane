// Fase 1, paso 1 — HC-SR04
const int TRIG = 5, ECHO = 18;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
}

float medirDistancia() {
  digitalWrite(TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long t = pulseIn(ECHO, HIGH, 25000);   // timeout 25 ms = ~4 m
  if (t == 0) return -1.0;               // sin eco
  return (t * 0.0343) / 2.0;             // cm
}

void loop() {
  Serial.println(medirDistancia());
  delay(200);
}
