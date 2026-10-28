// ===== SafeCane — firmware único (Fases 3 a 8) =====
// Cambia estas banderas según la fase (tabla en la guía, Fase 3).
#define WOKWI         0   // 1 = simulador Wokwi; 0 = bastón real
#define USAR_RED      1   // 0 = Fases 3–4 (sin red); 1 = Fase 5 en adelante
#define USAR_BATERIA  0   // 1 desde la Fase 6 (en Wokwi, el potenciómetro hace de celda)
#define MODO_REGISTRO 0   // 1 = imprime CSV a 50 Hz para la Fase 4

#define USAR_BLE (USAR_RED && !WOKWI)   // Wokwi no simula Bluetooth

#include <Wire.h>
#include <TinyGPSPlus.h>
#include "esp_sleep.h"
#include "esp_random.h"
#if USAR_RED
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#endif
#if USAR_BLE
#include <NimBLEDevice.h>
#endif

// ---------- Red (Fase 5) ----------
#if USAR_RED
#if WOKWI
const char* WIFI_SSID = "Wokwi-GUEST";        // red virtual de Wokwi, sin clave
const char* WIFI_PASS = "";
#else
const char* WIFI_SSID = "nay";                // hotspot del celular en 2,4 GHz
const char* WIFI_PASS = "72126102";
#endif
const char* MQTT_HOST = "broker.hivemq.com";  // broker público (opción B: IP de tu Mac con Mosquitto)
const int   MQTT_PORT = 1883;
const char* MQTT_USER = "";                   // vacío en el broker público
const char* MQTT_PASS = "";
const char* TOPICO    = "safecane/grupo6";    // cámbialo por algo único: el broker es compartido
#endif

// ---------- Pines ----------
const int TRIG = 5, ECHO = 18, SDA_P = 21, SCL_P = 22;
const int GPS_RX = 16, GPS_TX = 17;
const int IR_OUT = 34;            // OUT del FC-51: LOW = ve el piso
const int LDR = 32, BOTON = 27, VBAT = 36;
const int MOTOR = 25, BUZZER = 26, LED = 33;

// ---------- Umbrales (calibrar en Fase 4) ----------
const float SVM_CAIDA_LIBRE = 0.5;               // g
const float SVM_IMPACTO     = 2.5;               // g
const float INCLIN_MIN      = 60.0;              // grados
const unsigned long VENTANA_IMPACTO  = 1000;     // ms entre caída libre e impacto
const unsigned long T_ASENTAR        = 500;      // ms tras el impacto antes de medir la inclinación
const unsigned long INCLIN_SOSTENIDA = 2000;     // ms tumbado para confirmar
const unsigned long T_CANCELACION    = 10000;    // ms para cancelar con el pulsador
const unsigned long T_REPOSO         = WOKWI ? 120000 : 300000;   // sin movimiento
const unsigned long T_DESNIVEL       = 200;      // ms sin ver el piso = desnivel
const unsigned long T_DESNIVEL_MAX   = 1500;     // ms: el aviso de desnivel no dura más
const float INCL_USO_MIN = 20.0, INCL_USO_MAX = 75.0;   // grados: bastón en ángulo de uso
const float UMBRAL_MOVIMIENTO = 0.15;            // g: |svm - 1| que cuenta como movimiento
const int   UMBRAL_LDR = WOKWI ? 3000 : 2000;    // por encima = oscuro (Fase 1, paso 5)
const float VBAT_MIN   = 3.3;                    // V: por debajo 10 s, apagado ordenado

const int MPU = 0x68;
const float LSB_8G = 4096.0;

enum Estado { NORMAL, PRE_ALERTA, ALERTA, REPOSO };
const char* NOMBRE_ESTADO[] = {"NORMAL", "PRE_ALERTA", "ALERTA", "REPOSO"};
volatile Estado estado = NORMAL;

TinyGPSPlus gps;

unsigned long tIR = 0, tIMU = 0, tDist = 0, tLento = 0, tActuar = 0, tLog = 0;
unsigned long tCaidaLibre = 0, tImpacto = 0, tPreAlerta = 0, tUltimoMovimiento = 0;
unsigned long tUltimoPiso = 0, tBateriaOk = 0;
bool huboCaidaLibre = false;
float ax = 0, ay = 0, az = 1, svm = 1.0, inclinacion = 0, distancia = -1, vbat = 0;
int luz = 0;
bool pisoVisto = true, desnivel = false, usoActivo = false;
double latActual = 0, lonActual = 0;
float hdopActual = 0;
int satsActual = 0;

#if USAR_RED
WiFiClient espClient;
PubSubClient mqtt(espClient);
volatile bool alertaPendiente = false, cancelacionPendiente = false;
char topEstado[64], topAlerta[64], topGps[64];
#endif
#if USAR_BLE
NimBLECharacteristic* bleEstado = nullptr;
#endif

// ---------- Actuadores ----------
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

void buzzer(bool on) {
#if WOKWI
  static bool sonando = false;                     // el buzzer de Wokwi necesita un tono
  if (on && !sonando) tone(BUZZER, 2000);
  if (!on && sonando) noTone(BUZZER);
  sonando = on;
#else
  digitalWrite(BUZZER, on ? HIGH : LOW);           // buzzer activo
#endif
}

// ---------- Sensores ----------
void initMPU() {
  Wire.begin(SDA_P, SCL_P);
  Wire.beginTransmission(MPU); Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission(true);  // despertar
  Wire.beginTransmission(MPU); Wire.write(0x1C); Wire.write(0x10); Wire.endTransmission(true);  // ±8 g
  Wire.beginTransmission(MPU); Wire.write(0x1A); Wire.write(0x03); Wire.endTransmission(true);  // DLPF 44 Hz
}

#if WOKWI
unsigned long tSimCaida = 0;   // 'c' en el Monitor Serie inicia una caída simulada

bool caidaSimulada(unsigned long ahora) {
  if (tSimCaida == 0) return false;
  unsigned long t = ahora - tSimCaida;
  if (t < 300)        { ax = 0.0; ay = 0.0; az = 0.1; }   // caída libre
  else if (t < 400)   { ax = 2.0; ay = 2.0; az = 2.0; }   // impacto de 3,5 g
  else if (t < 15000) { ax = 1.0; ay = 0.0; az = 0.0; }   // bastón tumbado
  else { tSimCaida = 0; return false; }                   // vuelve a los deslizadores
  return true;
}
#endif

void leerAcel() {
#if WOKWI
  if (caidaSimulada(millis())) return;
#endif
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
  long t = pulseIn(ECHO, HIGH, 25000);   // bloquea hasta 25 ms
  if (t == 0) return -1.0;
  return (t * 0.0343) / 2.0;
}

#if WOKWI
// Wokwi no tiene GPS: genera las frases NMEA del NEO-6M (posición fija en UTEC, Barranco)
void gpsSimulado(unsigned long ahora) {
  static unsigned long t = 0;
  if (ahora - t < 1000) return;
  t = ahora;
  const char* frases[] = {
    "GPGGA,120000.00,1208.11500,S,07701.32900,W,1,08,0.9,95.0,M,10.0,M,,",
    "GPRMC,120000.00,A,1208.11500,S,07701.32900,W,0.0,0.0,261026,,,A"};
  for (const char* f : frases) {
    byte cs = 0;
    for (const char* p = f; *p; p++) cs ^= *p;
    char linea[100];
    snprintf(linea, sizeof(linea), "$%s*%02X\r\n", f, cs);
    for (char* p = linea; *p; p++) gps.encode(*p);
  }
}
#endif

void procesarGPS() {
  while (Serial2.available()) gps.encode(Serial2.read());
  if (gps.location.isUpdated()) {
    latActual = gps.location.lat();
    lonActual = gps.location.lng();
    satsActual = gps.satellites.value();
    hdopActual = gps.hdop.hdop();
  }
}

// ---------- Máquina de estados ----------
void cambiarEstado(Estado nuevo) {
  if (nuevo == estado) return;
  Serial.printf("[%lu ms] %s -> %s\n", millis(), NOMBRE_ESTADO[estado], NOMBRE_ESTADO[nuevo]);
  if (nuevo == PRE_ALERTA) tPreAlerta = millis();
#if USAR_RED
  if (nuevo == ALERTA) alertaPendiente = true;
  if (estado == ALERTA) cancelacionPendiente = true;
#endif
  estado = nuevo;
}

void maquinaEstados(unsigned long ahora) {
  switch (estado) {
    case NORMAL:
      if (ahora - tUltimoMovimiento > T_REPOSO) cambiarEstado(REPOSO);
      break;
    case PRE_ALERTA:
      if (ahora - tPreAlerta >= T_CANCELACION) cambiarEstado(ALERTA);
      break;
    case ALERTA:
      break;                                             // solo sale con el pulsador
    case REPOSO:
      if (fabs(svm - 1.0) > 0.3) cambiarEstado(NORMAL);
      break;
  }
}

// ---------- Tareas ----------
void tareaIR(unsigned long ahora) {
  pisoVisto = digitalRead(IR_OUT) == LOW;
  if (pisoVisto) tUltimoPiso = ahora;
  unsigned long sinPiso = ahora - tUltimoPiso;
  bool enUso = inclinacion >= INCL_USO_MIN && inclinacion <= INCL_USO_MAX;   // no avisa con el bastón levantado
  bool antes = desnivel;
  desnivel = estado == NORMAL && enUso && sinPiso >= T_DESNIVEL && sinPiso < T_DESNIVEL + T_DESNIVEL_MAX;
  if (desnivel && !antes) Serial.println("DESNIVEL: no hay piso delante de la punta");
}

void tareaIMU() {
  leerAcel();
  svm = sqrt(ax*ax + ay*ay + az*az);
  inclinacion = acos(constrain(az / max(svm, 0.01f), -1.0f, 1.0f)) * 180.0 / PI;
  unsigned long ahora = millis();
  if (fabs(svm - 1.0) > UMBRAL_MOVIMIENTO) tUltimoMovimiento = ahora;

  // Fase 1 de la caída: caída libre
  if (svm < SVM_CAIDA_LIBRE) {
    huboCaidaLibre = true;
    tCaidaLibre = ahora;
  }
  // Fase 2: impacto dentro de la ventana
  if (huboCaidaLibre && ahora - tCaidaLibre < VENTANA_IMPACTO && svm > SVM_IMPACTO) {
    huboCaidaLibre = false;
    tImpacto = ahora;
  }
  if (huboCaidaLibre && ahora - tCaidaLibre >= VENTANA_IMPACTO) huboCaidaLibre = false;

  // Fase 3: inclinación sostenida, medida después de que el bastón se asienta
  if (tImpacto > 0 && estado == NORMAL && ahora - tImpacto >= T_ASENTAR) {
    if (inclinacion < INCLIN_MIN) {
      tImpacto = 0;                              // se reincorporó: falsa alarma
    } else if (ahora - tImpacto >= T_ASENTAR + INCLIN_SOSTENIDA) {
      tImpacto = 0;
      cambiarEstado(PRE_ALERTA);
    }
  }

  if (MODO_REGISTRO) {
    Serial.printf("%lu,%.3f,%.3f,%.3f,%.3f,%.1f,%d,%d,%s\n", ahora, ax, ay, az, svm,
                  inclinacion, pisoVisto, desnivel, NOMBRE_ESTADO[estado]);
  }
}

void tareaDistancia() { distancia = medirDistancia(); }

void apagarPorBateria() {
  Serial.printf("BATERIA BAJA (%.2f V): apagando. Carga la celda.\n", vbat);
  motor(0);
  digitalWrite(LED, LOW);
  for (int i = 0; i < 3; i++) { buzzer(true); delay(600); buzzer(false); delay(300); }
  esp_deep_sleep_start();                        // vuelve con el interruptor o con EN
}

void tareaLenta(unsigned long ahora) {
  luz = analogRead(LDR);
  usoActivo = ahora - tUltimoMovimiento < 5000;
#if USAR_BATERIA
  vbat = analogReadMilliVolts(VBAT) * 2 / 1000.0;   // divisor 100k/100k: V de la celda
  if (vbat >= VBAT_MIN) tBateriaOk = ahora;
  else if (ahora - tBateriaOk > 10000) apagarPorBateria();
#endif
}

void tareaActuadores(unsigned long ahora) {
  bool oscuro = luz > UMBRAL_LDR;
  switch (estado) {
    case NORMAL: {
      digitalWrite(LED, oscuro ? HIGH : LOW);
      if (desnivel) {                                    // prioridad: escalón o bajada delante
        bool on = (ahora / 100) % 2;                     // 100 ms sí, 100 ms no
        motor(on ? 255 : 0);
        buzzer(on);
        break;
      }
      int duty = 0;
      if (distancia > 0 && distancia < 50)       duty = 255;                                // continuo
      else if (distancia > 0 && distancia < 100) duty = (ahora / 100) % 2 ? 200 : 0;        // pulsos rápidos
      else if (distancia > 0 && distancia < 200) duty = (ahora / 300) % 2 ? 120 : 0;        // pulsos lentos
      motor(duty);
      buzzer(false);
      break;
    }
    case PRE_ALERTA:                                     // patrón distintivo: 600 ms sí, 400 ms no
      digitalWrite(LED, oscuro ? HIGH : LOW);
      motor((ahora % 1000) < 600 ? 255 : 0);
      buzzer(false);
      break;
    case ALERTA: {                                       // buzzer y LED intermitentes
      bool on = (ahora % 500) < 250;
      motor(0);
      buzzer(on);
      digitalWrite(LED, on ? HIGH : LOW);
      break;
    }
    case REPOSO:
      motor(0);
      buzzer(false);
      digitalWrite(LED, LOW);
      break;
  }
}

void atenderBoton(unsigned long ahora) {
  static bool ultimaLectura = HIGH, estable = HIGH;
  static unsigned long tCambio = 0;
  bool lectura = digitalRead(BOTON);
  if (lectura != ultimaLectura) { tCambio = ahora; ultimaLectura = lectura; }
  if (ahora - tCambio > 50 && lectura != estable) {      // antirrebote 50 ms
    estable = lectura;
    if (estable == LOW && (estado == PRE_ALERTA || estado == ALERTA)) cambiarEstado(NORMAL);
  }
}

void atenderSerial(unsigned long ahora) {
  while (Serial.available()) {
    char c = Serial.read();
#if WOKWI
    if (c == 'c') { tSimCaida = ahora; Serial.println("SIM: caida simulada"); }
#endif
  }
}

void imprimirEstado() {
  Serial.printf("%s dist=%.0f svm=%.2f incl=%.0f piso=%d desnivel=%d uso=%d luz=%d sats=%d vbat=%.2f",
                NOMBRE_ESTADO[estado], distancia, svm, inclinacion, pisoVisto, desnivel,
                usoActivo, luz, satsActual, vbat);
#if USAR_RED
  Serial.printf(" wifi=%d mqtt=%d", WiFi.status() == WL_CONNECTED, mqtt.connected());
#endif
  Serial.println();
}

// ---------- Comunicación: corre en el núcleo 0 ----------
#if USAR_RED
void notificarBLE(const char* json) {
#if USAR_BLE
  if (bleEstado) { bleEstado->setValue(json); bleEstado->notify(); }
#endif
}

void publicarEstado() {
  JsonDocument doc;
  doc["estado"]   = NOMBRE_ESTADO[estado];
  doc["dist"]     = distancia;
  doc["desnivel"] = desnivel;
  doc["uso"]      = usoActivo;
  doc["vbat"]     = vbat;
  char buf[200];
  serializeJson(doc, buf);
  mqtt.publish(topEstado, buf);
  notificarBLE(buf);
}

bool publicarAlerta(const char* tipo) {
  JsonDocument doc;
  doc["tipo"] = tipo;                            // "caida" o "cancelada"
  doc["lat"]  = latActual;
  doc["lon"]  = lonActual;
  doc["sats"] = satsActual;
  char buf[200];
  serializeJson(doc, buf);
  notificarBLE(buf);
  return mqtt.connected() && mqtt.publish(topAlerta, buf, true);   // retained
}

void publicarGPS() {
  if (satsActual == 0) return;                   // sin fix todavía
  JsonDocument doc;
  doc["lat"]  = latActual;
  doc["lon"]  = lonActual;
  doc["sats"] = satsActual;
  doc["hdop"] = hdopActual;
  char buf[160];
  serializeJson(doc, buf);
  mqtt.publish(topGps, buf);
}

bool conectarMQTT() {
  char id[32];
  snprintf(id, sizeof(id), "safecane-%08lx", (unsigned long)esp_random());   // único en el broker
  if (strlen(MQTT_USER) == 0) return mqtt.connect(id);
  return mqtt.connect(id, MQTT_USER, MQTT_PASS);
}

#if USAR_BLE
void initBLE() {
  NimBLEDevice::init("SafeCane");
  NimBLEServer* servidor = NimBLEDevice::createServer();
  NimBLEService* servicio = servidor->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
  bleEstado = servicio->createCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E",
                                             NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  servicio->start();
  NimBLEAdvertising* anuncio = NimBLEDevice::getAdvertising();
  anuncio->setName("SafeCane");
  anuncio->addServiceUUID(servicio->getUUID());
  anuncio->start();
}
#endif

void tareaRed(void*) {
  WiFi.mode(WIFI_STA);
#if WOKWI
  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);           // canal 6: Wokwi conecta más rápido
#else
  WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setSocketTimeout(2);
  unsigned long tEstado = 0, tGps = 0, tAlerta = 0, tReintento = 0;
  bool alertaPublicada = false;
  for (;;) {
    unsigned long ahora = millis();
    if (WiFi.status() == WL_CONNECTED && !mqtt.connected() && ahora - tReintento >= 3000) {
      tReintento = ahora;
      Serial.println("Conectando a MQTT...");
      conectarMQTT();
    }
    if (mqtt.connected()) mqtt.loop();
    if (alertaPendiente && estado != ALERTA) alertaPendiente = false;   // la cancelaron antes de salir
    if (alertaPendiente && ahora - tAlerta >= 1000) {                  // reintenta cada 1 s
      tAlerta = ahora;
      if (publicarAlerta("caida")) { alertaPendiente = false; alertaPublicada = true; }
    }
    if (cancelacionPendiente && !alertaPendiente) {                    // borra la alerta retenida
      if (!alertaPublicada) cancelacionPendiente = false;
      else if (publicarAlerta("cancelada")) { cancelacionPendiente = false; alertaPublicada = false; }
    }
    if (ahora - tEstado >= 5000)  { tEstado = ahora; publicarEstado(); }
    if (ahora - tGps    >= 30000) { tGps    = ahora; publicarGPS(); }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
#endif

void setup() {
  Serial.begin(115200);
  Serial.printf("=== SafeCane (%s) ===\n", WOKWI ? "Wokwi" : "hardware");
  Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  pinMode(IR_OUT, INPUT);
  pinMode(BOTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT); pinMode(LED, OUTPUT);
  motorInit();
  analogReadResolution(12);
  initMPU();
#if USAR_RED
  snprintf(topEstado, sizeof(topEstado), "%s/estado", TOPICO);
  snprintf(topAlerta, sizeof(topAlerta), "%s/alerta", TOPICO);
  snprintf(topGps,    sizeof(topGps),    "%s/gps",    TOPICO);
#if USAR_BLE
  initBLE();
#endif
  xTaskCreatePinnedToCore(tareaRed, "red", 8192, nullptr, 1, nullptr, 0);
#endif
  if (MODO_REGISTRO) Serial.println("millis,ax,ay,az,svm,incl,piso,desnivel,estado");
#if WOKWI
  Serial.println("Wokwi: escribe c y Enter en el Monitor Serie para simular una caida");
#endif
}

void loop() {
  unsigned long ahora = millis();
#if WOKWI
  gpsSimulado(ahora);
#endif
  procesarGPS();                                          // vaciar UART cada vuelta
  if (ahora - tIR     >= 10)  { tIR     = ahora; tareaIR(ahora); }
  if (ahora - tIMU    >= 20)  { tIMU    = ahora; tareaIMU(); }
  if (ahora - tDist   >= 100) { tDist   = ahora; tareaDistancia(); }
  if (ahora - tLento  >= 200) { tLento  = ahora; tareaLenta(ahora); }
  if (ahora - tActuar >= 50)  { tActuar = ahora; tareaActuadores(ahora); }
  if (!MODO_REGISTRO && ahora - tLog >= 1000) { tLog = ahora; imprimirEstado(); }
  maquinaEstados(ahora);
  atenderBoton(ahora);
  atenderSerial(ahora);
}
