// Fase 1, paso 2b — MPU6050 en ±8 g
#include <Wire.h>
const int MPU = 0x68;
const float LSB_8G = 4096.0;   // sensibilidad en ±8 g

void initMPU() {
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); Wire.write(0x00);   // despertar
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU);
  Wire.write(0x1C); Wire.write(0x10);   // ACCEL_CONFIG = ±8 g
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU);
  Wire.write(0x1A); Wire.write(0x03);   // DLPF 44 Hz
  Wire.endTransmission(true);
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

void setup() { Serial.begin(115200); initMPU(); }

void loop() {
  float ax, ay, az;
  leerAcel(ax, ay, az);
  float svm = sqrt(ax*ax + ay*ay + az*az);
  float incl = acos(constrain(az / svm, -1.0, 1.0)) * 180.0 / PI;
  Serial.printf("svm=%.2f g  incl=%.0f grados\n", svm, incl);
  delay(20);   // 50 Hz
}
