// Fase 1, paso 2a — escáner I2C
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  for (byte a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.print("Dispositivo en 0x");
      Serial.println(a, HEX);
    }
  }
}
void loop() {}
