// Prueba de red — solo HC-SR04 → MQTT → tablero/tablero.html
// Publica la distancia cada 1 s en <TOPICO>/estado, con el mismo JSON que el firmware completo.
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "nay";         // hotspot del celular en 2,4 GHz
const char* WIFI_PASS = "72126102";
const char* MQTT_HOST = "broker.hivemq.com";  // broker público
const int   MQTT_PORT = 1883;
const char* TOPICO    = "safecane/grupo6";    // el mismo que escribes en el tablero

const int TRIG = 5, ECHO = 18;

WiFiClient espClient;
PubSubClient mqtt(espClient);
char topEstado[64];

float medirDistancia() {
  digitalWrite(TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long t = pulseIn(ECHO, HIGH, 25000);   // timeout 25 ms = ~4 m
  if (t == 0) return -1.0;               // sin eco
  return (t * 0.0343) / 2.0;             // cm
}

void conectar() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Conectando a Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.printf("\nWi-Fi OK, IP %s\n", WiFi.localIP().toString().c_str());
  }
  while (!mqtt.connected()) {
    char id[32];
    snprintf(id, sizeof(id), "safecane-%08lx", (unsigned long)esp_random());   // único en el broker
    Serial.print("Conectando a MQTT... ");
    if (mqtt.connect(id)) Serial.println("OK");
    else { Serial.printf("falló (rc=%d), reintento en 2 s\n", mqtt.state()); delay(2000); }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  snprintf(topEstado, sizeof(topEstado), "%s/estado", TOPICO);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
}

void loop() {
  conectar();
  mqtt.loop();

  static unsigned long tPub = 0;
  if (millis() - tPub >= 1000) {
    tPub = millis();
    JsonDocument doc;
    doc["estado"]   = "NORMAL";
    doc["dist"]     = medirDistancia();
    doc["desnivel"] = false;
    doc["uso"]      = true;
    doc["vbat"]     = 0;                  // sin batería en esta prueba: el tablero muestra "—"
    char buf[128];
    serializeJson(doc, buf);
    bool ok = mqtt.publish(topEstado, buf);
    Serial.printf("%s %s %s\n", topEstado, buf, ok ? "" : "(no se envió)");
  }
}
