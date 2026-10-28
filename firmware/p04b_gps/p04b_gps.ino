// Fase 1, paso 4b — GPS decodificado
#define WOKWI 1   // 1 = simulador Wokwi (frases NMEA generadas); 0 = NEO-6M real

#include <TinyGPSPlus.h>
TinyGPSPlus gps;

#if WOKWI
// Wokwi no tiene GPS: se generan cada segundo las mismas frases que manda el NEO-6M
// (posición fija en UTEC, Barranco) y pasan por el mismo decodificador.
void gpsSimulado() {
  static unsigned long t = 0;
  if (millis() - t < 1000) return;
  t = millis();
  const char* frases[] = {
    "GPGGA,120000.00,1208.11500,S,07701.32900,W,1,08,0.9,95.0,M,10.0,M,,",
    "GPRMC,120000.00,A,1208.11500,S,07701.32900,W,0.0,0.0,261026,,,A"};
  for (const char* f : frases) {
    byte cs = 0;
    for (const char* p = f; *p; p++) cs ^= *p;             // checksum NMEA
    char linea[100];
    snprintf(linea, sizeof(linea), "$%s*%02X\r\n", f, cs);
    for (char* p = linea; *p; p++) gps.encode(*p);
  }
}
#endif

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
}

void loop() {
#if WOKWI
  gpsSimulado();
#endif
  while (Serial2.available()) gps.encode(Serial2.read());
  if (gps.location.isUpdated()) {
    Serial.printf("%.6f, %.6f  sats=%d  hdop=%.1f  t=%lus\n",
                  gps.location.lat(), gps.location.lng(),
                  (int)gps.satellites.value(), gps.hdop.hdop(), millis() / 1000);
  }
}
