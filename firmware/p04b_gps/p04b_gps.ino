// Fase 1, paso 4b — GPS decodificado

#include <TinyGPSPlus.h>
TinyGPSPlus gps;


void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
}

void loop() {
  while (Serial2.available()) gps.encode(Serial2.read());
  if (gps.location.isUpdated()) {
    Serial.printf("%.6f, %.6f  sats=%d  hdop=%.1f  t=%lus\n",
                  gps.location.lat(), gps.location.lng(),
                  (int)gps.satellites.value(), gps.hdop.hdop(), millis() / 1000);
  }
}
