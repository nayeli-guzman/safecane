// Fase 2 — prueba de integración: todos los sensores + motor intermitente
#include <Wire.h>
#include <TinyGPSPlus.h>
#include "esp_system.h"

const int TRIG = 5, ECHO = 18, GPS_RX = 16, GPS_TX = 17;
const int IR_OUT = 34, LDR = 32, BOTON = 27;
const int MOTOR = 25, BUZZER = 26, LED = 33;
const int MPU = 0x68;
const float LSB_8G = 4096.0;

TinyGPSPlus gps;

void motorInit() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(MOTOR, 5000, 8);
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

void initMPU() {
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU); Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission(true);
  Wire.beginTransmission(MPU); Wire.write(0x1C); Wire.write(0x10); Wire.endTransmission(true);
  Wire.beginTransmission(MPU); Wire.write(0x1A); Wire.write(0x03); Wire.endTransmission(true);
}

void leerAcel(float &ax, float &ay, float &az) {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  int16_t rx = Wire.read() << 8 | Wire.read();
  int16_t ry = Wire.read() << 8 | Wire.read();
  int16_t rz = Wire.read() << 8 | Wire.read();
  ax = rx / LSB_8G; ay = ry / LSB_8G; az = rz / LSB_8G;
}

float medirDistancia() {
  digitalWrite(TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long t = pulseIn(ECHO, HIGH, 25000);
  if (t == 0) return -1.0;
  return (t * 0.0343) / 2.0;
}

void setup() {
  Serial.begin(115200);
  Serial.printf("=== ARRANQUE === motivo de reinicio: %d\n", (int)esp_reset_reason());
  Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  pinMode(IR_OUT, INPUT);
  pinMode(BOTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT); pinMode(LED, OUTPUT);
  motorInit();
  analogReadResolution(12);
  initMPU();
}

void loop() {
  static unsigned long tImp = 0;
  unsigned long ahora = millis();

  while (Serial2.available()) gps.encode(Serial2.read());

  bool motorOn = (ahora / 2000) % 2;                    // motor 2 s sí, 2 s no
  motor(motorOn ? 200 : 0);

  if (ahora - tImp >= 200) {
    tImp = ahora;
    float ax, ay, az;
    leerAcel(ax, ay, az);
    float svm = sqrt(ax*ax + ay*ay + az*az);
    Serial.printf("t=%lus motor=%d dist=%.1f svm=%.2f piso=%d ldr=%d boton=%d sats=%d\n",
                  ahora / 1000, motorOn, medirDistancia(), svm, digitalRead(IR_OUT) == LOW,
                  (int)analogRead(LDR), digitalRead(BOTON), (int)gps.satellites.value());
  }
}
