# Guía paso a paso del MVP IoT (FC-51 + Wokwi)

Oct 28, 2026 · @Shupa

## 0. Cómo usar esta guía

Esta guía reemplaza a `guia_safecane_infrarrojo.md` (versión del 22 de octubre) y a `guia_safecane.pdf`. Está pensada para una **demo ante el profesor**: un bastón que avisa de obstáculos, avisa cuando la punta llega a un escalón que baja, detecta caídas y envía la alerta con su ubicación a un tablero.

**Dos caminos, el mismo código.** Todo se prueba primero en **Wokwi** (simulador gratuito en wokwi.com) y después en el **bastón real**. Los sketches son los mismos; los que se comportan distinto en el simulador tienen al inicio una línea `#define WOKWI 1` (simulador) o `0` (hardware). Así puedes avanzar mientras llegan las piezas, y en la demo tienes un **plan B** si algo físico falla.

**Regla principal:** no pases a la sección siguiente hasta marcar todas las casillas de su bloque "Criterio exacto para avanzar". Las casillas marcadas con **(Wokwi)** se cumplen en el simulador; las marcadas con **(real)** solo con hardware.

### Ruta solo Wokwi (para quien hace la simulación)

Esta guía se basta sola para la parte de Wokwi: el circuito y todos los sketches están copiados dentro de ella, así que no hace falta la carpeta del proyecto. Sigue este orden y marca solo las casillas **(Wokwi)**:

1. **Sección 2:** lee el mapa de pines y la tabla "Qué se simula en Wokwi y qué no".
2. **Fase 0B:** crea el proyecto, pega el circuito y agrega las 3 librerías.
3. **Fase 1A y 1B:** pega cada sketch en `sketch.ino` y cumple sus casillas (Wokwi). El sketch `p04a_gps_crudo` se salta: es solo para hardware.
4. **Fase 2:** opcional en Wokwi.
5. **Fase 3:** firmware completo, con las banderas de la fila "3 en Wokwi".
6. **Fase 5:** red y tablero, con las banderas de la fila "5 en Wokwi". Para ver la alerta no instalas nada: abres `tablero/tablero.html` en el navegador. Acuerda con el equipo un `TOPICO` único y úsenlo todos igual.
7. **Fase 6:** solo el bloque "En Wokwi" (batería baja con el potenciómetro).
8. Se saltan la Fase 0 (salvo la tabla de software), la Fase 4 y la Fase 7: necesitan el bastón real.

Al terminar, guarda el proyecto y comparte su enlace con el equipo: se usa como plan B en la demo (Anexo C). Si algo no coincide con lo que dice la guía, revisa la tabla "Wokwi" del Anexo A.

### Archivos del proyecto (ya creados)

```text
project/
├── guia_safecane_infrarrojo.md   ← esta guía
├── lista_compras_mvp.md          ← lista de compras con estado
├── tablero/
│   └── tablero.html              ← tablero web de la Fase 5 (doble clic para abrir)
├── wokwi/
│   ├── diagram.json              ← circuito completo para Wokwi
│   └── libraries.txt             ← librerías que Wokwi debe instalar
└── firmware/
    ├── p00_blink/ … p06_actuadores/   ← pruebas de la Fase 0 y la Fase 1
    ├── p07_mqtt_hcsr04/                ← prueba de red: solo HC-SR04 → tablero (Fase 5)
    ├── f2_integracion/                 ← Fase 2
    ├── safecane/                       ← firmware único (Fases 3 a 8)
    └── calibracion/                    ← captura.py, metricas.py, desniveles.py (Fase 4)
```

Todo el firmware está compilado con el core ESP32 3.3.12 que tienes en el Arduino IDE, en sus cinco combinaciones (Wokwi o hardware, con o sin red, con o sin batería). Lo que **no** pudo verificarse sin abrir Wokwi en el navegador es el comportamiento dentro del simulador: por eso la Fase 0B tiene sus propias casillas de comprobación.

### El sensor infrarrojo: FC-51

El FC-51 es un módulo de 3 pines (VCC, GND, OUT) con un LED infrarrojo transparente, un receptor oscuro, un comparador LM393, un potenciómetro y dos LED (encendido y detección). **OUT = LOW cuando ve una superficie** (se enciende su LED de detección) y **OUT = HIGH cuando no ve nada**.

En el bastón va cerca de la punta, mirando al piso. Mientras ve el piso, todo sigue normal. Si el piso "desaparece" (escalón que baja, bordillo, hueco), el bastón da un aviso propio, distinto al de obstáculos.

**Lo que hay que saber para defenderlo en la demo:**

- El FC-51 mide **cuánta luz infrarroja vuelve**, no la distancia. A 3,3 V ve el piso a unos 2–8 cm según el color: los pisos claros se ven mejor que los oscuros, y el sol directo lo ciega.
- Como va pegado a la punta, el aviso llega **cuando la punta alcanza el borde**, no antes. Es una prueba de concepto del aviso de desniveles; para avisar desde 30 cm antes haría falta un sensor de distancia (Sharp GP2Y0A21 o un ToF VL53L0X). Eso va como trabajo futuro en el informe.
- El aviso solo se activa con el bastón **en ángulo de uso** (20–75° respecto de la vertical, medido por el MPU6050) y dura como máximo 1,5 s. Así no suena al levantar el bastón para subir a un bus o sentarse.

### Qué cambia respecto de la versión del 22 de octubre

| Antes | Ahora | Por qué |
| --- | --- | --- |
| Módulo IR "FC-51 o TCRT5000" | **FC-51** confirmado | El TCRT5000 solo ve a 1–2 cm; no sirve en el bastón |
| Motor de 3 V en el riel de 3,3 V | **Motor de 5 V** (el que tienes) en el riel de 5 V, base con 470 Ω | Coincide con la lista de compras |
| LDR arriba y 10 kΩ abajo (más luz = número más alto) | **10 kΩ arriba y LDR abajo (más luz = número más bajo)** | Así se comporta el sensor de luz de Wokwi: un solo código para los dos |
| El aviso de desnivel sonaba mientras no hubiera piso | Solo en ángulo de uso y máximo 1,5 s | No suena al levantar o cargar el bastón |
| La inclinación se medía en el mismo instante del impacto | Se mide 0,5 s después del impacto | En el impacto el ángulo es aleatorio y anulaba caídas reales |
| La alerta retenida en MQTT nunca se borraba | Al cancelar se publica `"tipo": "cancelada"` | El tablero ya no muestra caídas viejas |
| Tablero en Node-RED (Node.js + 2 paquetes + flujo a mano) | **Una página `tablero/tablero.html`** que se abre con doble clic | Nada que instalar; funciona en cualquier laptop con internet |
| Mosquitto en la Mac + Wi-Fi de casa | **Broker público + hotspot del celular** (Mosquitto queda como opción B) | Funciona en Wokwi, en la universidad y al aire libre |
| Fase 5 = aplicar 8 cambios a mano | **Un solo firmware** con banderas `USAR_RED`, `USAR_BATERIA` y `MODO_REGISTRO` | Menos errores al copiar |
| Sin protección por batería baja | Bajo 3,3 V durante 10 s: 3 pitidos y apagado | Evita reinicios en bucle |
| GPS "dentro del tubo" | GPS arriba de la caja de la empuñadura | El aluminio bloquea la señal y el módulo no cabe en el tubo |
| Bastón plegable con cables por dentro | **Bastón rígido** (o cables por fuera con conectores) | Por dentro del plegable pasa el elástico |

### Correcciones al PDF original que siguen aplicadas

- **Cadena de alimentación:** el MT3608 toma energía de la salida protegida del TP4056 (OUT+/OUT−), pasando por el interruptor.
- **Aviso de obstáculos:** se comprueba `distancia > 0`, porque sin eco la distancia vale −1.
- **Drivers:** 3 transistores 2N2222 (motor, buzzer y LED).
- **Batería:** divisor 100 kΩ/100 kΩ hacia GPIO 36 para medirla.
- **Red:** Wi-Fi/MQTT corren en el núcleo 0; los sensores, en el núcleo 1.
- **ArduinoJson 7:** se usa `JsonDocument`.
- **Sin piezo ni FSR:** el uso activo y el modo REPOSO usan el movimiento del MPU6050.

### Estructura fija de cada sección

Cada fase tiene los mismos bloques: **Para qué sirve**, **Qué usas**, **Conexiones**, **Código**, **En Wokwi** y **Criterio exacto para avanzar**. Para cambiar algo, di la fase y el bloque: por ejemplo, "en Fase 3, cambia T\_DESNIVEL a 300".

## 1. Lista de compras MVP

La lista con la columna de estado está en `lista_compras_mvp.md`. Resumen:

### Procesamiento y sensores

| Componente | Especificación | Cant. | Para qué sirve | Se usa en |
| --- | --- | --- | --- | --- |
| ESP32-WROOM-32 DevKit V1 | 38 pines, no la variante "U" | 1 | Cerebro | Fase 0 |
| HC-SR04 | 5 V, 2–400 cm | 1 | Obstáculos al frente | Fase 1, paso 1 |
| MPU6050 (GY-521) | Acelerómetro I2C | 1 | Caídas, inclinación y movimiento | Fase 1, paso 2 |
| **FC-51** | VCC, GND, OUT; potenciómetro | 1 (**ya lo tienes**) | Escalones que bajan y bordillos | Fase 1, paso 3 |
| GPS NEO-6M (GY-NEO6MV2) | Antena cerámica | 1 | Ubicación de la alerta | Fase 1, paso 4 |
| LDR GL5528 | 8–20 kΩ a 10 lux | 1 | Encender el LED cuando oscurece | Fase 1, paso 5 |
| Pulsador 12 mm + capuchón alto | Normalmente abierto | 1 | Cancelar una alerta | Fase 1, paso 5 |

### Actuadores

| Componente | Especificación | Cant. | Para qué sirve | Se usa en |
| --- | --- | --- | --- | --- |
| Motor vibrador | **5 V** | 1 (**ya lo tienes**) | Aviso háptico | Fase 1, paso 6 |
| Buzzer **activo** | 5 V, ≈85 dB | 1 | Alarma sonora | Fase 1, paso 6 |
| LED blanco alto brillo | 5 mm | 1 | Visibilidad de noche | Fase 1, paso 6 |

### Alimentación

| Componente | Especificación | Cant. | Se usa en |
| --- | --- | --- | --- |
| Celda 18650 | 3000 mAh reales (Samsung 30Q, LG MJ1, Sony VTC) | 1 | Fase 6 |
| Portaceldas 18650 | Con cables | 1 | Fase 6 |
| TP4056 **con protección** | DW01A + FS8205A, pads OUT+/OUT− | 1 | Fase 6 |
| MT3608 | Elevador ajustable a 5,0 V | 1 | Fase 6 |
| Interruptor deslizante | 3 A | 1 | Fase 6 |
| Cable micro-USB **de datos** | **2 m** (para grabar la Fase 4 caminando) | 1 | Fase 0 |

### Pasivos y discretos

| Componente | Valor | Cant. | Dónde va |
| --- | --- | --- | --- |
| Resistencia | 10 kΩ | 4 | Divisor del LDR (1) + 3 de reserva |
| Resistencia | 100 kΩ | 2 | Divisor de batería |
| Resistencia | 1 kΩ | 3 | Divisor del Echo (1) + bases del buzzer y del LED (2) |
| Resistencia | 470 Ω | 1 | Base del transistor del motor |
| Resistencia | 2 kΩ (o 2 × 1 kΩ) | 1 | Divisor del Echo |
| Resistencia | 100 Ω | 1 | LED blanco |
| Transistor NPN 2N2222 | 600 mA | 3 | Motor, buzzer y LED |
| Diodo 1N4007 | — | 2 | Rueda libre en motor y buzzer |
| Capacitor electrolítico | 100–470 µF / 10 V | 2 | Rieles de 5 V y 3,3 V |
| Capacitor cerámico | 100 nF | 7 | HC-SR04, MPU6050, GPS, FC-51, ESP32, bornes del motor y divisor de batería |

### Mecánica y cableado

| Componente | Especificación | Cant. |
| --- | --- | --- |
| Bastón **rígido** (o telescópico) | Si solo consigues plegable, los cables van por fuera | 1 |
| Protoboard | 830 puntos | 1 |
| Perfboard | 7 × 9 cm | 2 |
| Jumpers Dupont | M-M, M-H, H-H | 3 juegos |
| Espadines | Macho y hembra, tira de 40 | 2 |
| Cable flexible AWG 26–28 | 4 hilos, ≈1,5 m | 1 |
| Conectores JST-XH | 3 y 4 pines, macho + hembra | 4 pares |
| Funda espiral para cables | — | 1 m |
| Caja 3D (empuñadura) + soporte del FC-51 | PLA | 2 piezas |
| Termorretráctil, bridas, velcro, tornillos M3 | Surtido | 1 juego |

Herramientas: **multímetro** (indispensable), cautín de 30–40 W con estaño, pelacables, pinza fina.

**Para Wokwi no hay que comprar nada:** basta con una cuenta gratuita en wokwi.com.

## 2. Arquitectura y mapa de pines

```text
ENTRADAS                          ESP32-WROOM-32                 SALIDAS (vía 2N2222)
HC-SR04 (obstáculos)  Trig 5, Echo 18 ─┐  ┌────────────────────┐  ┌─ Motor vibrador 5 V  PWM 25
MPU6050 (caídas)      SDA 21, SCL 22  ─┤  │ Núcleo 1: sensores │  ├─ Buzzer activo 5 V   26
FC-51 (desnivel)      OUT 34          ─┼─►│ y máquina estados  │─►┼─ LED blanco 5 V      33
GPS NEO-6M            TX→16, RX←17    ─┤  │ Núcleo 0: Wi-Fi,   │  │
LDR (luz)             32              ─┤  │ MQTT y BLE         │  │ RED
Pulsador (cancelar)   27 (pull-up)    ─┤  │                    │  ├─ Wi-Fi + MQTT → broker → tablero web
Batería (divisor)     36 (VP)         ─┘  └────────────────────┘  └─ BLE → celular (solo hardware)

ALIMENTACIÓN (Fase 6): 18650 → TP4056 (OUT protegido) → interruptor → MT3608 5,0 V
  → riel 5 V: VIN del ESP32, HC-SR04, GPS, motor, buzzer, LED
  Pin 3V3 del ESP32 → riel 3,3 V: MPU6050, FC-51, LDR
```

### Mapa de pines definitivo

Es el mismo en el bastón real y en `wokwi/diagram.json`.

| Señal | GPIO | Modo | Conexión real | En Wokwi |
| --- | --- | --- | --- | --- |
| HC-SR04 Trig | 5 | Salida | Directo | Directo |
| HC-SR04 Echo | 18 | Entrada | Por divisor 1 kΩ / 2 kΩ | Directo (no hace falta divisor) |
| MPU6050 SDA / SCL | 21 / 22 | I2C | Directo | Directo |
| GPS TX → ESP32 | 16 | UART2 RX | Directo | Sin conectar: el firmware genera el GPS |
| ESP32 → GPS RX | 17 | UART2 TX | Directo | Sin conectar |
| FC-51 OUT | 34 | Entrada digital | Directo (el módulo trae pull-up) | Interruptor deslizante |
| LDR | 32 | ADC1 | 3V3 → 10 kΩ → nodo → LDR → GND | Sensor de luz (AO) |
| Batería | 36 (VP) | ADC1 | Divisor 100 kΩ / 100 kΩ desde TP4056 OUT+ | Potenciómetro |
| Pulsador | 27 | `INPUT_PULLUP` | A GND | Pulsador "Cancelar" |
| Motor vibrador | 25 | PWM | 470 Ω → base de Q1 | LED naranja "Motor" |
| Buzzer | 26 | Salida | 1 kΩ → base de Q2 | Buzzer de Wokwi |
| LED blanco | 33 | Salida | 1 kΩ → base de Q3 | LED blanco |

Tres reglas explican el mapa:

- **ADC2 no funciona con Wi-Fi activo.** Todas las entradas analógicas van en ADC1 (GPIO 32–39).
- **GPIO 34–39 son solo entrada** y no tienen pull-up interno. Sirven para el OUT del FC-51 (trae su resistencia) y para medir la batería.
- **GPIO 6–11** son de la memoria flash, y **GPIO 0, 2, 12 y 15** son pines de arranque. Quedan libres 4, 13, 14, 19, 23 y 35.

### Rieles de alimentación

| Riel | De dónde sale | Qué alimenta |
| --- | --- | --- |
| 5 V | Pin 5V/VIN del DevKit con USB (Fases 0–5); MT3608 con batería (Fase 6 en adelante) | HC-SR04, GPS, motor, buzzer, LED blanco |
| 3,3 V | Pin 3V3 del DevKit (su regulador) | MPU6050, VCC del FC-51, LDR |

El FC-51 va a **3,3 V**, no a 5 V: así su OUT nunca supera 3,3 V y no daña el GPIO 34.

### Qué se simula en Wokwi y qué no

| Parte real | En Wokwi | Cómo se maneja |
| --- | --- | --- |
| ESP32 DevKit | `board-esp32-devkit-c-v4` (mismos GPIO) | — |
| HC-SR04 | `wokwi-hc-sr04` | Clic en el sensor → deslizador de distancia |
| MPU6050 | `wokwi-mpu6050` | Clic → deslizadores accelX/Y/Z. La caída se simula escribiendo `c` en el Monitor Serie |
| FC-51 | Interruptor deslizante | **Izquierda = ve el piso** (OUT a GND). **Derecha = vacío** (OUT a 3,3 V) |
| GPS NEO-6M | No existe en Wokwi | Con `WOKWI 1`, el firmware genera las mismas frases NMEA (posición fija en UTEC) |
| LDR + 10 kΩ | `wokwi-photoresistor-sensor` | Clic → deslizador de lux |
| Pulsador | Pulsador rojo "Cancelar" | Clic |
| Motor vibrador | LED naranja "Motor" | Su brillo es la intensidad del PWM |
| Buzzer activo | `wokwi-buzzer` | Con `WOKWI 1` se maneja con `tone()`; activa el sonido del navegador |
| LED blanco | LED blanco | — |
| Celda + divisor | Potenciómetro en VP | Girado al máximo = 6,6 V; a dos tercios ≈ 4,4 V; a la mitad ≈ 3,3 V |
| Transistores, diodos, capacitores, divisor del Echo, TP4056, MT3608 | **No se simulan** | Wokwi simula la lógica, no la electricidad: esto se prueba solo con hardware |
| Wi-Fi | Red `Wokwi-GUEST` (sin clave) | Sale a internet: llega al broker público |
| BLE | **No se simula** | Solo en hardware |

**Lo que Wokwi no puede demostrar:** el ruido del motor, el consumo y la autonomía, el alcance real del FC-51 sobre cada piso y los umbrales de caída con movimientos reales. Para eso están las casillas **(real)**.

## Fase 0 — Preparar el entorno

**Para qué sirve:** comprobar que la PC programa el ESP32 y que el chip ejecuta código.

**Qué usas:** ESP32 DevKit, cable micro-USB de datos y la PC.

### Software

En tu Mac ya están el Arduino IDE, el core ESP32 3.3.12 y las librerías TinyGPSPlus 1.0.3, PubSubClient 2.8 y ArduinoJson 7.4.3. Falta:

| Programa | Para qué | Cómo instalarlo | Se usa en |
| --- | --- | --- | --- |
| Cuenta en wokwi.com | Simulador | Registrarte con Google o GitHub | Fase 0B |
| NimBLE-Arduino (h2zero) 2.x | BLE (solo hardware) | Arduino IDE → Herramientas → Administrar bibliotecas | Fase 5 |
| Navegador (Chrome, Safari o Firefox) | Tablero `tablero/tablero.html` | Ya lo tienes | Fase 5 |
| MQTT Explorer (opcional) | Ver los JSON crudos si el tablero falla | mqtt-explorer.com | Fase 5 |
| nRF Connect (celular) | Probar BLE | Play Store / App Store | Fase 5 |
| Python 3 + pyserial | Grabar CSV de calibración | `pip3 install pyserial` | Fase 4 |

### Pasos

1. Arduino IDE → Herramientas → Placa → esp32 → **ESP32 Dev Module**.
2. Mira el chip junto al USB de la placa. Si dice **CP2102**, instala el driver de Silicon Labs. Si dice **CH340** o **CH9102**, el de WCH.
3. Conecta la placa. En Herramientas → Puerto debe aparecer `/dev/cu.usbserial-…` o `/dev/cu.wchusbserial-…`. Si no aparece, cambia de cable antes que nada.
4. Abre `firmware/p00_blink/p00_blink.ino`, súbelo y abre el Monitor Serie a **115200 baudios**.
5. Si la carga se queda en `Connecting…_____`, mantén presionado **BOOT** hasta que empiece a subir.

### Código (`firmware/p00_blink/`)

```cpp
// Fase 0 — blink
void setup() {
  pinMode(2, OUTPUT);          // LED azul integrado del DevKit (Wokwi no lo dibuja)
  Serial.begin(115200);
}

void loop() {
  digitalWrite(2, HIGH); delay(500);
  digitalWrite(2, LOW);  delay(500);
  Serial.println("ESP32 vivo");
}
```

### En Wokwi

Esta prueba se hace en la Fase 0B, ya con el circuito completo.

### Criterio exacto para avanzar

- [ ] **(real)** El LED azul de la placa parpadea una vez por segundo.
- [ ] **(real)** El Monitor Serie imprime `ESP32 vivo`, una línea por segundo.
- [ ] **(real)** El sketch se sube 3 veces seguidas sin errores.

## Fase 0B — Montar el circuito en Wokwi

**Para qué sirve:** tener el bastón completo en el simulador. Se arma una sola vez: todas las pruebas siguientes usan este mismo circuito, porque los pines son los del mapa de la sección 2.

**Qué usas:** el navegador, tu cuenta de Wokwi y los archivos de la carpeta `wokwi/`.

### Pasos

1. Entra a **wokwi.com → New Project → ESP32**. Se abre un proyecto con tres pestañas: `sketch.ino`, `diagram.json` y `Library Manager`.
2. Copia **todo** el bloque `diagram.json` de abajo (es el mismo que el archivo `wokwi/diagram.json`) y pégalo reemplazando el contenido de la pestaña `diagram.json`. A la derecha aparecen el ESP32, el HC-SR04, el MPU6050, el interruptor del FC-51, el sensor de luz, el pulsador, el potenciómetro de batería, el LED "Motor", el buzzer y el LED blanco.
3. En la pestaña **Library Manager**, pulsa **+** y agrega `TinyGPSPlus`, `PubSubClient` y `ArduinoJson` (son las tres líneas de `wokwi/libraries.txt`). Escríbelas tal cual, respetando mayúsculas.
4. Pega en `sketch.ino` el contenido de `firmware/p00_blink/p00_blink.ino` y pulsa ▶ (Start).
5. Guarda el proyecto con **Save** y ponle de nombre `SafeCane`. Wokwi te da un enlace: guárdalo, sirve para la demo.

### Circuito (`diagram.json`)

```json
{
  "version": 1,
  "author": "SafeCane",
  "editor": "wokwi",
  "parts": [
    { "type": "board-esp32-devkit-c-v4", "id": "esp", "top": 0, "left": 0, "attrs": {} },
    { "type": "wokwi-hc-sr04", "id": "ultra", "top": -200, "left": -260, "attrs": { "distance": "150" } },
    { "type": "wokwi-mpu6050", "id": "imu", "top": -170, "left": 190, "attrs": { "accelX": "0.7", "accelZ": "0.7" } },
    { "type": "wokwi-slide-switch", "id": "ir", "top": 40, "left": -230, "attrs": { "value": "" } },
    { "type": "wokwi-photoresistor-sensor", "id": "ldr", "top": 120, "left": -420, "attrs": { "lux": "500" } },
    { "type": "wokwi-pushbutton", "id": "btn", "top": 230, "left": -230, "attrs": { "color": "red", "label": "Cancelar" } },
    { "type": "wokwi-potentiometer", "id": "bat", "top": -100, "left": -120, "attrs": {} },
    { "type": "wokwi-resistor", "id": "r1", "top": 300, "left": 180, "rotate": 90, "attrs": { "value": "220" } },
    { "type": "wokwi-led", "id": "motor", "top": 360, "left": 170, "attrs": { "color": "orange", "label": "Motor" } },
    { "type": "wokwi-buzzer", "id": "bz", "top": 330, "left": 260, "attrs": { "volume": "0.2" } },
    { "type": "wokwi-resistor", "id": "r2", "top": 300, "left": 360, "rotate": 90, "attrs": { "value": "220" } },
    { "type": "wokwi-led", "id": "luz", "top": 360, "left": 350, "attrs": { "color": "white", "label": "LED" } }
  ],
  "connections": [
    [ "esp:TX", "$serialMonitor:RX", "", [] ],
    [ "esp:RX", "$serialMonitor:TX", "", [] ],

    [ "ultra:VCC", "esp:5V", "red", [] ],
    [ "ultra:GND", "esp:GND.1", "black", [] ],
    [ "ultra:TRIG", "esp:5", "green", [] ],
    [ "ultra:ECHO", "esp:18", "blue", [] ],

    [ "imu:VCC", "esp:3V3", "red", [] ],
    [ "imu:GND", "esp:GND.2", "black", [] ],
    [ "imu:SDA", "esp:21", "green", [] ],
    [ "imu:SCL", "esp:22", "blue", [] ],

    [ "ir:1", "esp:GND.1", "black", [] ],
    [ "ir:2", "esp:34", "violet", [] ],
    [ "ir:3", "esp:3V3", "red", [] ],

    [ "ldr:VCC", "esp:3V3", "red", [] ],
    [ "ldr:GND", "esp:GND.1", "black", [] ],
    [ "ldr:AO", "esp:32", "orange", [] ],

    [ "btn:1.l", "esp:27", "green", [] ],
    [ "btn:2.l", "esp:GND.1", "black", [] ],

    [ "bat:VCC", "esp:3V3", "red", [] ],
    [ "bat:GND", "esp:GND.1", "black", [] ],
    [ "bat:SIG", "esp:VP", "gray", [] ],

    [ "esp:25", "r1:1", "orange", [] ],
    [ "r1:2", "motor:A", "orange", [] ],
    [ "motor:C", "esp:GND.2", "black", [] ],

    [ "bz:2", "esp:26", "purple", [] ],
    [ "bz:1", "esp:GND.2", "black", [] ],

    [ "esp:33", "r2:1", "white", [] ],
    [ "r2:2", "luz:A", "white", [] ],
    [ "luz:C", "esp:GND.3", "black", [] ]
  ],
  "dependencies": {}
}
```

**Cómo usar cada sketch en Wokwi:** pega su contenido en `sketch.ino` (reemplazando el anterior) y pulsa ▶. Si el sketch tiene `#define WOKWI`, déjalo en `1`. Para escribir en el Monitor Serie, haz clic en la consola de abajo y escribe.

**Los cables se dibujan rectos** y pueden cruzarse: es solo estético. Si quieres ordenarlos, arrastra las piezas; las conexiones no cambian.

### Criterio exacto para avanzar

- [ ] **(Wokwi)** El circuito muestra las 12 piezas sin avisos de conexión en rojo.
- [ ] **(Wokwi)** Con `p00_blink`, el Monitor Serie imprime `ESP32 vivo` una vez por segundo.
- [ ] **(Wokwi)** El proyecto está guardado y tienes su enlace.

## Fase 1A — Sensores uno a uno: distancia, caídas y desniveles

**Para qué sirve:** probar cada sensor solo para conocer su comportamiento real antes de mezclarlo con otros.

**Regla en hardware:** un sensor a la vez en la protoboard. Desconecta el anterior antes de conectar el siguiente. En Wokwi todo está conectado siempre; no molesta, porque cada sketch solo lee su sensor.

### Paso 1 — HC-SR04 (obstáculos)

**Qué usas:** HC-SR04, resistencias de 1 kΩ y 2 kΩ.

**Conexiones:**

| Pata del HC-SR04 | Va a |
| --- | --- |
| VCC | Pin **5V/VIN** del DevKit (USB) |
| GND | GND |
| Trig | GPIO 5 |
| Echo | 1 kΩ → nodo → GPIO 18; del nodo, 2 kΩ → GND |

El divisor baja los 5 V del Echo a 3,33 V. Conectar Echo directo daña el pin con el tiempo.

```cpp
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
```

**En Wokwi:** haz clic en el HC-SR04 mientras corre la simulación y mueve el deslizador de distancia.

**Criterio exacto para avanzar:**

- [ ] **(Wokwi)** Con el deslizador en 20, 100 y 300 cm, imprime esos valores con menos de 2 cm de diferencia.
- [ ] **(real)** Contra una pared lisa a 20, 50, 100 y 200 cm (con cinta métrica), el error es menor a 3 cm.
- [ ] **(real)** Entre lecturas seguidas, la variación es menor a 2 cm.
- [ ] **(real)** Sin obstáculo en 4 m imprime `-1.00`.
- [ ] **(real)** Anotaste las lecturas contra concreto, vidrio y tela (la tela absorbe el ultrasonido).

### Paso 2 — MPU6050 (caídas)

**Qué usas:** módulo GY-521.

**Conexiones:** VCC → 3V3, GND → GND, SDA → GPIO 21, SCL → GPIO 22. El GY-521 ya trae pull-ups.

Primero el escáner I2C (`firmware/p02a_i2c_scanner/`):

```cpp
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
```

Luego la lectura en **±8 g**. Por defecto arranca en ±2 g y se satura justo en el impacto de una caída (`firmware/p02b_mpu6050/`):

```cpp
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
```

La inclinación se mide respecto del **eje Z del GY-521**, que en el bastón debe quedar **paralelo al tubo**. Con el bastón vertical, la inclinación es 0°; acostado, 90°; en uso normal, entre 30° y 60°.

**En Wokwi:** haz clic en el MPU6050 para ver los deslizadores. El `diagram.json` arranca con accelX = 0,7 y accelZ = 0,7: es el bastón inclinado 45°, como al caminar.

**Criterio exacto para avanzar:**

- [ ] **(Wokwi y real)** El escáner imprime `Dispositivo en 0x68`.
- [ ] **(Wokwi)** Con accelX = 0, accelY = 0, accelZ = 1: `svm=1.00` e `incl=0`. Con accelX = 0,7 y accelZ = 0,7: `incl=45`. Si en reposo `svm` sale 0,25 o 4,00 en vez de 1,00, mira el Anexo A.
- [ ] **(real)** Quieto sobre la mesa con Z vertical: `svm` entre 0,97 y 1,03 g e `incl` menor a 5°.
- [ ] **(real)** Sacudido con fuerza: `svm` supera 3 g.
- [ ] **(real)** Girado 90°: `incl` entre 80° y 100°.

### Paso 3 — FC-51 (escaleras que bajan)

**Para qué sirve:** avisar cuando la punta llega al borde de un escalón que baja, un bordillo o un hueco. Mientras ve el piso, OUT está en LOW. Si el piso se aleja más allá de su alcance, OUT pasa a HIGH. Cuando eso dura más de `T_DESNIVEL` (200 ms), el bastón da su aviso de desnivel.

**Qué usas:** el FC-51 y un capacitor de 100 nF.

**Conexiones:**

| Pin del FC-51 | Va a |
| --- | --- |
| VCC | **3V3**, no 5 V: así OUT nunca supera 3,3 V |
| GND | GND |
| OUT | GPIO 34 |
| Capacitor 100 nF | Entre VCC y GND, junto al módulo |

```cpp
// Fase 1, paso 3 — FC-51 (detector de desnivel)
const int IR_OUT = 34;   // OUT del FC-51: LOW = ve el piso, HIGH = no ve nada

void setup() {
  Serial.begin(115200);
  pinMode(IR_OUT, INPUT);  // el FC-51 trae su propia resistencia pull-up
}

void loop() {
  static unsigned long tUltimoPiso = 0;
  bool piso = digitalRead(IR_OUT) == LOW;
  if (piso) tUltimoPiso = millis();
  Serial.printf("piso=%d  sin_piso_hace=%lu ms\n", piso, millis() - tUltimoPiso);
  delay(50);
}
```

**En Wokwi:** el interruptor deslizante hace de FC-51. A la izquierda imprime `piso=1`; a la derecha, `piso=0` y `sin_piso_hace` empieza a crecer.

**Calibración del potenciómetro (real):**

1. Fija el FC-51 con cinta a **4–6 cm de la punta**, con las lentes mirando al piso y un poco hacia adelante (20–30°). Sujeta el bastón con el ángulo con que caminas: las lentes quedan a **2–4 cm del piso**.
2. Gira el potenciómetro hasta que se encienda el LED de detección (`piso=1`) sobre el piso de la sala.
3. Gira un poco más para dejar margen: debe seguir en `piso=1` sobre el piso más oscuro que vayas a usar.
4. Lleva la punta al borde de un escalón que baja, con el módulo sobre el vacío: debe pasar a `piso=0`.
5. Si sobre piso oscuro no llega a `piso=1` ni con el potenciómetro al máximo, baja el módulo (lentes a 2 cm) antes de cambiar cualquier otra cosa.

| Superficie o situación | ¿`piso=1`? | Nota |
| --- | --- | --- |
| Losa clara | | |
| Losa o madera oscura | | |
| Alfombra | | |
| Vereda o asfalto | | |
| Al sol directo | | |
| Módulo sobre el borde de un escalón que baja | Debe ser 0 | |

**Criterio exacto para avanzar:**

- [ ] **(Wokwi)** El interruptor a la izquierda da `piso=1`, y a la derecha da `piso=0` con `sin_piso_hace` creciendo.
- [ ] **(real)** Sobre losa clara, losa oscura y vereda, con el bastón en su ángulo de uso, `piso=1` en 20 de 20 lecturas.
- [ ] **(real)** Con el módulo sobre el borde de un escalón de bajada de 15 cm o más, `piso=0` en 10 de 10 intentos.
- [ ] **(real)** Anotaste si funciona al sol directo.
- [ ] **(real)** Con VCC en 3V3, el multímetro mide en OUT entre 0 y 3,3 V, nunca más.
- [ ] **(real)** Elegiste el piso de la demo (el más claro y sin sol directo) y lo probaste ahí.

## Fase 1B — Ubicación, luz, pulsador y actuadores

**Para qué sirve:** validar las entradas y salidas restantes, cada una sola.

### Paso 4 — GPS NEO-6M (ubicación)

**Para qué sirve:** adjuntar latitud y longitud a cada alerta.

**Conexiones:** VCC → 5V/VIN, GND → GND, **TX del GPS → GPIO 16**, **RX del GPS → GPIO 17**. Se cruzan: el TX de uno va al RX del otro.

Prueba cruda, **solo hardware** (`firmware/p04a_gps_crudo/`):

```cpp
// Fase 1, paso 4a — GPS, NMEA crudo (solo hardware: Wokwi no tiene GPS)
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
}
void loop() {
  while (Serial2.available()) Serial.write(Serial2.read());
}
```

Deben salir líneas `$GPGGA,…` y `$GPRMC,…`. Si no sale nada, TX y RX están invertidos. Si salen con campos vacíos, aún no hay satélites.

Decodificado (`firmware/p04b_gps/`):

```cpp
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
```

Sal al aire libre con la antena cerámica hacia arriba. Dentro del laboratorio no va a fijar.

**En Wokwi:** con `#define WOKWI 1`, el sketch genera cada segundo las mismas frases que manda un NEO-6M, con una posición fija en UTEC. Pasan por el mismo decodificador TinyGPSPlus, así que el resto del código no cambia. Para el bastón real, cambia a `#define WOKWI 0`.

**Criterio exacto para avanzar:**

- [ ] **(Wokwi)** Imprime `-12.135250, -77.022150  sats=8  hdop=0.9` una vez por segundo. Pegada en Google Maps, la coordenada cae en UTEC.
- [ ] **(real)** Al aire libre, la primera coordenada aparece antes de `t=120s`, con `sats` 4 o más.
- [ ] **(real)** En Google Maps la coordenada cae donde estás. Anota el error en metros (referencia: 4,63 m de Fitriandini et al.).

### Paso 5 — LDR y pulsador

**Para qué sirven:** el LDR decide cuándo encender el LED de visibilidad. El pulsador cancela una pre-alerta o alerta.

**Conexiones:**

| Elemento | Conexión |
| --- | --- |
| LDR | 3V3 → **10 kΩ** → nodo → GPIO 32; del nodo, **LDR → GND** |
| Pulsador | Un terminal a GPIO 27, el otro a GND (pull-up interno) |

Con esta conexión, **más luz da un número más bajo**, igual que el sensor de luz de Wokwi. Ponle al pulsador un capuchón alto: el usuario debe encontrarlo al tacto después de una caída.

```cpp
// Fase 1, paso 5 — LDR y pulsador
const int LDR = 32, BOTON = 27;

void setup() {
  Serial.begin(115200);
  pinMode(BOTON, INPUT_PULLUP);
  analogReadResolution(12);
}

void loop() {
  // Más luz = número más bajo (igual en Wokwi y en el circuito real)
  Serial.printf("ldr=%d  boton=%s\n", analogRead(LDR),
                digitalRead(BOTON) == LOW ? "PRESIONADO" : "suelto");
  delay(100);
}
```

**En Wokwi:** haz clic en el sensor de luz y mueve el deslizador de lux. Haz clic en el pulsador rojo.

**Criterio exacto para avanzar:**

- [ ] **(Wokwi)** Con 1000 lux, `ldr` sale bajo (unas 700); con 10 lux, alto (unas 3400). El umbral del firmware para Wokwi es 3000.
- [ ] **(Wokwi y real)** 10 de 10 pulsaciones muestran `PRESIONADO`, y al soltar vuelve a `suelto`.
- [ ] **(real)** Al tapar el LDR con la mano, `ldr` sube al menos al doble. Anota el valor con luz de sala y tapado: el punto medio será `UMBRAL_LDR` en el firmware (por defecto 2000).

### Paso 6 — Motor vibrador, buzzer y LED

**Para qué sirven:** el motor avisa de obstáculos, desniveles y la pre-alerta. El buzzer suena en ALERTA y en los desniveles. El LED hace visible al usuario de noche.

**Qué usas:** 3 transistores 2N2222, resistencias de 470 Ω (1) y 1 kΩ (2), 2 diodos 1N4007, una resistencia de 100 Ω y un capacitor de 100 nF.

**Conexiones** (2N2222 visto de frente, cara plana hacia ti: E – B – C; confírmalo en la hoja de datos de tu encapsulado):

| Actuador | Base | Colector | Emisor | Alimentación y protección |
| --- | --- | --- | --- | --- |
| Motor 5 V | GPIO 25 → **470 Ω** → B (Q1) | Motor (−) | GND | Motor (+) → **5V/VIN**. 1N4007 en paralelo, cátodo (franja) hacia 5 V. 100 nF entre los bornes del motor |
| Buzzer activo | GPIO 26 → 1 kΩ → B (Q2) | Buzzer (−) | GND | Buzzer (+) → **5V/VIN**. 1N4007 en paralelo, cátodo hacia 5 V |
| LED blanco | GPIO 33 → 1 kΩ → B (Q3) | Cátodo del LED (pata corta) | GND | 5V/VIN → 100 Ω → ánodo del LED (≈ 17 mA) |

Nunca conectes el motor directo a un GPIO: consume 80–120 mA y el pin entrega unos 12 mA cómodos. Sin el 1N4007, el pico al apagar el motor reinicia el ESP32. Mide con el multímetro la corriente del motor: si pasa de 150 mA, cambia la resistencia de base a 330 Ω.

```cpp
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
```

**En Wokwi:** el LED naranja "Motor" sube de brillo en cada ciclo. Para oír el buzzer, activa el sonido del navegador (Wokwi lo pide la primera vez). Para el bastón real, cambia a `#define WOKWI 0`: el buzzer activo se maneja con nivel alto, no con un tono.

**Criterio exacto para avanzar (cierre de la Fase 1):**

- [ ] **(Wokwi)** El LED "Motor" sube de brillo de forma gradual; el buzzer y el LED blanco se encienden juntos 0,3 s por ciclo.
- [ ] **(real)** El motor sube de intensidad de forma gradual en cada ciclo.
- [ ] **(real)** Buzzer y LED se encienden juntos 0,3 s por ciclo.
- [ ] **(real)** Durante 5 minutos, el ESP32 no se reinicia (sin texto de arranque en el Monitor Serie).

## Fase 2 — Integración eléctrica en protoboard

**Para qué sirve:** conectar todo a la vez, todavía alimentado por USB, y comprobar que ningún componente arruina a otro. El motor es la principal fuente de ruido, así que la prueba se hace con el motor vibrando.

**Qué usas:** todo lo de la Fase 1, más los 2 electrolíticos y los 100 nF.

### Orden de conexión

Conecta uno por uno. Después de cada uno, sube `f2_integracion` y confirma que los anteriores siguen leyendo bien.

1. MPU6050 (I2C)
2. HC-SR04 (picos de corriente)
3. GPS (sensible al ruido de alimentación)
4. Entradas: FC-51 y LDR
5. Pulsador
6. Actuadores: motor, buzzer, LED

### Reglas de cableado

- **Un solo GND común:** todas las masas llegan a la misma fila de la protoboard.
- **100 nF junto a cada módulo**, entre su VCC y su GND: HC-SR04, MPU6050, GPS, FC-51 y junto al ESP32.
- **Electrolítico de 100–470 µF en cada riel:** uno en 5V/VIN–GND y otro en 3V3–GND. La franja va a GND. El motor de 5 V comparte riel con el GPS: este capacitor es obligatorio.
- **Cable del FC-51:** baja hasta la punta (≈ 1 m). Trénzalo y llévalo lejos del cable del motor.

### Código (`firmware/f2_integracion/`)

```cpp
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
```

### En Wokwi

Corre igual, pero no prueba lo importante de esta fase (ruido y caídas de tensión), porque Wokwi no simula la electricidad. Úsalo solo para ver que todas las lecturas cambian al mover cada control. `sats` queda en 0: este sketch no incluye el GPS simulado.

### Criterio exacto para avanzar

Deja correr el sketch **10 minutos seguidos** con el GPS cerca de una ventana, y compara las líneas con `motor=1` contra las de `motor=0`:

- [ ] **(Wokwi)** Al mover cada control (distancia, accelZ, interruptor, lux, pulsador), cambia su campo en el Monitor Serie.
- [ ] **(real)** `=== ARRANQUE ===` aparece una sola vez en los 10 minutos.
- [ ] **(real)** Con el montaje quieto, `svm` se mantiene entre 0,97 y 1,03 con motor encendido y apagado.
- [ ] **(real)** Contra una pared fija, `dist` varía menos de 2 cm entre `motor=1` y `motor=0`.
- [ ] **(real)** Con el FC-51 mirando al piso, `piso=1` con `motor=1` y con `motor=0`. Apuntándolo al aire, `piso=0`.
- [ ] **(real)** `sats` no cae a 0 cuando el motor se enciende. Si cae, falta desacoplo.
- [ ] **(real)** Al presionar el pulsador, `boton=0`.

## Fase 3 — Firmware integrado con máquina de estados

**Para qué sirve:** reunir todo en un solo programa que nunca se detiene. Avisa de obstáculos y de escalones que bajan, y detecta caídas. Todavía sin red.

**Qué usas:** el montaje de la Fase 2 (real) o el circuito de la Fase 0B (Wokwi).

### Un firmware para todas las fases

`firmware/safecane/safecane.ino` es el **único** firmware desde aquí hasta la demo. Lo que cambia entre fases son cuatro banderas al inicio del archivo:

| Fase | `WOKWI` | `USAR_RED` | `USAR_BATERIA` | `MODO_REGISTRO` |
| --- | --- | --- | --- | --- |
| 3 en Wokwi | 1 | 0 | 0 | 0 |
| 3 en el bastón | 0 | 0 | 0 | 0 |
| 4 (calibración, solo real) | 0 | 0 | 0 | 1 |
| 5 en Wokwi | 1 | 1 | 0 | 0 |
| 5 en el bastón | 0 | 1 | 0 | 0 |
| 6–8 y demo en el bastón | 0 | 1 | 1 | 0 |
| Demo en Wokwi (plan B) | 1 | 1 | 1 | 0 |

### Tiempos de cada tarea

No hay `delay()`: cada tarea corre con su propio periodo, medido con `millis()`.

| Tarea | Periodo | Motivo |
| --- | --- | --- |
| Leer FC-51 (piso) | 10 ms | Detectar el borde sin retraso |
| Leer IMU, detectar caída y movimiento | 20 ms (50 Hz) | Mínimo para capturar el impacto |
| Medir distancia | 100 ms | `pulseIn` bloquea hasta 25 ms |
| Leer LDR y batería | 200 ms | Cambian lento |
| Actualizar motor, buzzer y LED | 50 ms | Patrones hápticos |
| GPS | Cada vuelta | Vaciar el buffer de UART2 |
| Imprimir estado | 1 s | Observar en el Monitor Serie |

### Cómo decide que hubo una caída

Se deben cumplir tres condiciones en orden:

1. **Caída libre:** `svm` < 0,5 g.
2. **Impacto:** `svm` > 2,5 g dentro del segundo siguiente.
3. **Inclinación sostenida:** medio segundo después del impacto (cuando el bastón se asentó), más de 60° durante 2 s. Entonces pasa a PRE\_ALERTA.

### Cómo avisa de un escalón que baja

Cada 10 ms se lee el FC-51. Hay desnivel cuando se cumplen las tres:

- el FC-51 no ve el piso durante `T_DESNIVEL` (200 ms) seguidos;
- el bastón está en ángulo de uso: entre 20° y 75° respecto de la vertical;
- el estado es NORMAL.

Mientras dura, el motor y el buzzer pulsan juntos (100 ms sí, 100 ms no), con prioridad sobre el aviso de obstáculos. El aviso se corta solo a los 1,5 s (`T_DESNIVEL_MAX`) aunque siga sin ver el piso, y vuelve a poder sonar cuando el FC-51 ve el piso otra vez.

### Máquina de estados

```text
            caída confirmada               10 s sin pulsar
  NORMAL ─────────────────────► PRE_ALERTA ─────────────────► ALERTA
  (vibra según distancia,  ◄──── (vibra 600 ms/1 s)            (buzzer + LED + MQTT)
   y avisa de desniveles)  pulsador < 10 s
     ▲  │                                                         │
     │  └──────────────────────────── pulsador ◄──────────────────┘
     │  │
     │  ▼ 5 min sin movimiento (2 min en Wokwi)
  REPOSO (actuadores apagados)
     └─ ▲ movimiento → vuelve a NORMAL
```

El pulsador es la única salida de ALERTA. Los 10 s de PRE\_ALERTA son la confirmación interactiva que mantiene bajas las falsas alarmas (Geng).

### Código (`firmware/safecane/`)

Los umbrales del inicio se ajustan con los valores de la Fase 1 (`UMBRAL_LDR`) y de la Fase 4 (`T_DESNIVEL` y los de caída).

```cpp
// ===== SafeCane — firmware único (Fases 3 a 8) =====
// Cambia estas banderas según la fase (tabla en la guía, Fase 3).
#define WOKWI         1   // 1 = simulador Wokwi; 0 = bastón real
#define USAR_RED      0   // 0 = Fases 3–4 (sin red); 1 = Fase 5 en adelante
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
const char* WIFI_SSID = "TU_HOTSPOT";         // hotspot del celular en 2,4 GHz
const char* WIFI_PASS = "TU_CLAVE";
#endif
const char* MQTT_HOST = "broker.hivemq.com";  // broker público (opción B: IP de tu Mac con Mosquitto)
const int   MQTT_PORT = 1883;
const char* MQTT_USER = "";                   // vacío en el broker público
const char* MQTT_PASS = "";
const char* TOPICO    = "safecane/grupo1";    // cámbialo por algo único: el broker es compartido
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
```

### En Wokwi

Pega `safecane.ino` en `sketch.ino` con las banderas de la fila "3 en Wokwi" y pulsa ▶. Cómo probar cada cosa:

| Qué probar | Qué haces |
| --- | --- |
| Obstáculos | Mueve la distancia del HC-SR04: 30, 70 y 150 cm. Mira el LED "Motor" |
| Desnivel | Deja el MPU en accelX = 0,7 y accelZ = 0,7 (45°) y pasa el interruptor a la derecha |
| Desnivel con el bastón levantado | Pon accelX = 0 y accelZ = 1 (vertical) y pasa el interruptor a la derecha: no debe avisar |
| Caída | Haz clic en el Monitor Serie, escribe `c` y pulsa Enter. El firmware reproduce una caída libre de 0,3 s, un impacto de 3,5 g y 15 s con el bastón tumbado |
| Cancelar | Haz clic en el pulsador rojo durante la PRE\_ALERTA o la ALERTA |
| Luz | Baja el sensor de luz a 10 lux: se enciende el LED blanco |
| REPOSO | No toques nada durante 2 minutos. Para despertarlo, sube accelZ a 1,5 un instante |

### Criterio exacto para avanzar

- [ ] **(Wokwi y real)** Obstáculo a 30 cm: vibración continua. A 70 cm: pulsos rápidos. A 150 cm: pulsos lentos. A más de 2 m: nada.
- [ ] **(Wokwi)** Con el MPU a 45° y el interruptor a la derecha, aparece `DESNIVEL` y el motor y el buzzer pulsan juntos unos 1,5 s y se callan. Con el MPU vertical, no aparece.
- [ ] **(Wokwi)** Con `c` aparece `NORMAL -> PRE_ALERTA` unos 3 s después, y `PRE_ALERTA -> ALERTA` 10 s más tarde con buzzer y LED intermitentes. El pulsador vuelve a `NORMAL`.
- [ ] **(Wokwi)** Con `c` y el pulsador antes de 10 s aparece `PRE_ALERTA -> NORMAL`.
- [ ] **(real)** Con el bastón (o la protoboard con el MPU) inclinado unos 45° sobre el piso, `piso=1`. Al llevar el FC-51 al borde de un escalón aparece `DESNIVEL`. **Con el MPU plano sobre la mesa no avisa: es lo esperado.**
- [ ] **(real)** Caída simulada sobre colchoneta sujetando el bastón: aparece `NORMAL -> PRE_ALERTA` y el motor cambia al patrón de 600 ms.
- [ ] **(real)** Durante 1 minuto caminando en plano no aparece ningún `DESNIVEL`.
- [ ] **(real)** Con `T_REPOSO` cambiado temporalmente a `30000`, el bastón quieto pasa a `REPOSO` a los 30 s y vuelve a `NORMAL` al levantarlo. Después devuélvelo a su valor.

## Fase 4 — Calibración del algoritmo (solo hardware)

**Para qué sirve:** reemplazar los umbrales de diseño (0,5 g, 2,5 g, 60°, `T_DESNIVEL`) por valores medidos en tu propio bastón. Produce la evidencia experimental del informe: sensibilidad y especificidad de caídas, y detección de escalones.

**Por qué no en Wokwi:** en el simulador los datos los inventas tú con los deslizadores; calibrar con ellos no demuestra nada.

**Qué usas:** el bastón con el firmware en `MODO_REGISTRO 1`, una colchoneta y la laptop **en una mochila** conectada con el cable USB de 2 m (la batería llega en la Fase 6).

### Paso 1 — Activar el modo registro

Pon `#define MODO_REGISTRO 1`, con `WOKWI 0` y `USAR_RED 0`, y súbelo. El Monitor Serie mostrará 50 líneas por segundo:

```csv
millis,ax,ay,az,svm,incl,piso,desnivel,estado
15020,0.012,-0.031,0.998,0.999,2.1,1,0,NORMAL
```

Cierra el Monitor Serie antes de grabar: el puerto solo puede abrirlo un programa a la vez.

### Paso 2 — Grabar cada repetición

Los scripts están en `firmware/calibracion/`. Ejecútalos desde esa carpeta. Cada repetición es un archivo y el nombre dice el escenario.

```python
# captura.py — uso: python3 captura.py /dev/cu.usbserial-0001 caida_adelante_01
# Graba el CSV del modo registro en datos/<etiqueta>.csv. Ctrl+C para terminar.
import os, sys, serial

puerto, etiqueta = sys.argv[1], sys.argv[2]
os.makedirs("datos", exist_ok=True)
with serial.Serial(puerto, 115200, timeout=1) as s, open(f"datos/{etiqueta}.csv", "w") as f:
    f.write("millis,ax,ay,az,svm,incl,piso,desnivel,estado\n")
    print("Grabando", etiqueta, "... Ctrl+C para terminar")
    filas = 0
    try:
        while True:
            linea = s.readline().decode(errors="ignore").strip()
            if linea.count(",") == 8 and linea[0].isdigit():
                f.write(linea + "\n")
                filas += 1
    except KeyboardInterrupt:
        print(f"\n{filas} filas guardadas ({filas / 50:.1f} s a 50 Hz)")
```

En cada repetición: inicia la captura, espera 3 s, ejecuta el escenario, espera 15 s y pulsa Ctrl+C. Después pulsa el botón del bastón para volver a NORMAL.

### Paso 3 — Protocolo de captura (dimensionado para la demo)

| Escenario | Nombre del archivo | ¿Debe alertar? | Repeticiones |
| --- | --- | --- | --- |
| Caída hacia adelante sujetando el bastón (colchoneta) | `caida_adelante_01` … `_10` | Sí | 10 |
| Caída lateral | `caida_lateral_01` … | Sí | 10 |
| Caída hacia atrás | `caida_atras_01` … | Sí | 10 |
| Marcha normal | `marcha_01` | No | 3 min |
| Sentarse bruscamente | `sentarse_01` … | No | 10 |
| Golpear la punta fuerte contra el suelo | `golpe_01` … | No | 10 |
| Subir y bajar escaleras | `escaleras_01` … | No | 10 |
| Soltar el bastón al suelo | `soltar_01` … | No (se cancela con el pulsador) | 10 |
| Bajar escaleras, un archivo por escalón | `escalon_01` … | Aviso de desnivel | 10 |
| Bajar un bordillo de vereda | `bordillo_01` … | Aviso de desnivel | 10 |
| Caminar en plano (losa y vereda) | `plano_01` | Ningún aviso de desnivel | 3 min |

`soltar` probablemente generará pre-alerta, porque no hay sensor de agarre. Anótalo: entra como falsa alarma en la especificidad.

**Seguridad:** las caídas se simulan sobre colchoneta, solo con integrantes del equipo y nunca con adultos mayores ni personas con discapacidad visual. En escaleras, baja agarrado al pasamanos y con alguien al lado.

### Paso 4 — Calcular las métricas

```python
# metricas.py — uso: python3 metricas.py datos
# Cada .csv es una repetición. Los que empiezan con "caida_" deben generar alerta.
import csv, glob, os, sys

vp = fn = vn = fp = 0
for ruta in sorted(glob.glob(os.path.join(sys.argv[1], "*.csv"))):
    nombre = os.path.basename(ruta)
    if nombre.startswith(("escalon_", "bordillo_", "plano_")):
        continue                                    # esos los evalúa desniveles.py
    with open(ruta) as f:
        hubo = any(fila["estado"] in ("PRE_ALERTA", "ALERTA") for fila in csv.DictReader(f))
    debe = nombre.startswith("caida_")
    if debe and hubo:  vp += 1
    elif debe:         fn += 1; print("No detectada:", nombre)
    elif hubo:         fp += 1; print("Falsa alarma:", nombre)
    else:              vn += 1

print(f"VP={vp} FN={fn} VN={vn} FP={fp}")
if vp + fn: print(f"Sensibilidad  = {100 * vp / (vp + fn):.1f} %  (objetivo >= 90 %)")
if vn + fp: print(f"Especificidad = {100 * vn / (vn + fp):.1f} %")
```

```latex
\text{Sensibilidad} = \frac{VP}{VP + FN} \qquad \text{Especificidad} = \frac{VN}{VN + FP}
```

Para los desniveles, `desniveles.py` cuenta los avisos que dio el firmware en cada archivo (la columna `desnivel`, que ya incluye el filtro de ángulo de uso):

```python
# desniveles.py — uso: python3 desniveles.py datos
# Cuenta los avisos de desnivel que dio el firmware (flancos de subida de la columna desnivel).
import csv, glob, os, sys

for prefijo in ("escalon_", "bordillo_", "plano_"):
    rutas = sorted(glob.glob(os.path.join(sys.argv[1], prefijo + "*.csv")))
    con_aviso = 0
    for ruta in rutas:
        avisos, antes = 0, "0"
        with open(ruta) as f:
            filas = list(csv.DictReader(f))
        for fila in filas:
            if fila["desnivel"] == "1" and antes == "0": avisos += 1
            antes = fila["desnivel"]
        con_aviso += avisos > 0
        minutos = (int(filas[-1]["millis"]) - int(filas[0]["millis"])) / 60000 if filas else 0
        extra = f"  ({avisos / minutos:.1f} por minuto)" if prefijo == "plano_" and minutos else ""
        print(f"{os.path.basename(ruta)}: {avisos} avisos{extra}")
    if rutas and prefijo != "plano_":
        print(f"== {prefijo}*: {con_aviso} de {len(rutas)} archivos con aviso\n")
```

### Paso 5 — Ajustar y repetir

| Si pasa esto | Ajusta | Hacia |
| --- | --- | --- |
| Sensibilidad < 90 % | `SVM_IMPACTO` | Bajar de 0,25 en 0,25 g |
| Caídas reales sin fase de caída libre en el CSV | `SVM_CAIDA_LIBRE` | Subir a 0,6–0,7 g |
| Falsas alarmas al sentarse o en escaleras | `INCLIN_SOSTENIDA` o `SVM_IMPACTO` | Subir |
| Avisos de desnivel al caminar en plano | `T_DESNIVEL` o potenciómetro | Subir `T_DESNIVEL` a 300–400 ms; más alcance en el potenciómetro |
| Escalones sin aviso | `T_DESNIVEL`, `INCL_USO_MIN/MAX` o posición | Bajar `T_DESNIVEL` a 100 ms; revisar en el CSV la columna `incl` al llegar al borde; acercar el módulo a la punta |

Anota los valores iniciales, los finales y por qué cambiaste cada uno: ese razonamiento se evalúa en el informe. Guarda la carpeta `datos/`.

### Criterio exacto para avanzar

- [ ] **(real)** Tienes las repeticiones de la tabla.
- [ ] **(real)** Sensibilidad **≥ 90 %** (27 de 30 caídas) con los umbrales finales.
- [ ] **(real)** Aviso de desnivel en al menos 9 de 10 `escalon_*` y 9 de 10 `bordillo_*`.
- [ ] **(real)** Anotaste los falsos avisos por minuto en `plano_01` y las pre-alertas en `soltar_*`.
- [ ] **(real)** Los umbrales finales están en el firmware y `MODO_REGISTRO` vuelve a `0`.

## Fase 5 — Comunicación: Wi-Fi, MQTT, BLE y tablero

**Para qué sirve:** que una caída confirmada llegue a un tablero, con su ubicación, en menos de 5 s, y que un celular cercano la reciba por BLE. La alerta local (buzzer y vibración) nunca depende de la red: la red corre en el núcleo 0 y los sensores en el núcleo 1.

**Qué usas:** el navegador de tu laptop con `tablero/tablero.html` y, para el bastón real, el **hotspot del celular**.

### Paso 1 — Elegir broker y red

**Opción A (recomendada para la demo): broker público + hotspot.**

- Broker: `broker.hivemq.com`, puerto 1883, sin usuario. Si no responde, usa `test.mosquitto.org`.
- Es público: cualquiera que conozca el tópico puede leerlo. Por eso `TOPICO` debe ser único, por ejemplo `safecane/utec-grupo3-x7k2`. No publiques datos personales.
- Red del bastón: el **hotspot del celular en 2,4 GHz** (el ESP32 no ve redes de 5 GHz). En iPhone: Compartir Internet → activar **Maximizar compatibilidad**. En Android: punto de acceso → banda **2,4 GHz**. Así funciona en la universidad, en la calle y en la demo, sin depender del Wi-Fi del lugar.
- En Wokwi la red es `Wokwi-GUEST` y llega sola al broker público: no hay que configurar nada.

**Opción B (si te piden seguridad): Mosquitto en la Mac con usuario y contraseña.** Solo sirve con el bastón real, con la Mac y el bastón en el mismo hotspot:

```bash
brew install mosquitto
mosquitto_passwd -c /opt/homebrew/etc/mosquitto/passwd baston
```

Reemplaza `/opt/homebrew/etc/mosquitto/mosquitto.conf` por:

```
listener 1883 0.0.0.0
listener 9001 0.0.0.0
protocol websockets
allow_anonymous false
password_file /opt/homebrew/etc/mosquitto/passwd
```

El puerto 9001 (WebSocket) es el que usa el tablero: el navegador no puede hablar MQTT por el 1883.

Arráncalo con `mosquitto -c /opt/homebrew/etc/mosquitto/mosquitto.conf -v`, acepta el aviso del firewall de macOS y pon en el firmware `MQTT_HOST` = la IP de `ipconfig getifaddr en0`, `MQTT_USER = "baston"` y su clave. La IP cambia cada vez que te conectas al hotspot: revísala antes de la demo.

### Paso 2 — Activar la red en el firmware

En `safecane.ino`:

1. Pon `#define USAR_RED 1`.
2. Cambia `TOPICO` por tu tópico único.
3. Solo para el bastón real: pon el nombre y la clave del hotspot en `WIFI_SSID` y `WIFI_PASS`, y verifica que NimBLE-Arduino esté instalada (el BLE se activa solo con `WOKWI 0`).

El firmware con BLE ocupa el 92 % de la partición por defecto, así que entra sin cambiar nada. Si algún día el IDE dice `Sketch too big`, elige **Herramientas → Partition Scheme → Huge APP**.

### Paso 3 — Tópicos MQTT

| Tópico | Contenido (JSON) | Frecuencia |
| --- | --- | --- |
| `<TOPICO>/estado` | estado, dist, desnivel, uso, vbat | Cada 5 s |
| `<TOPICO>/alerta` | tipo (`caida` o `cancelada`), lat, lon, sats — retained | Al entrar en ALERTA (reintenta cada 1 s) y al cancelarla |
| `<TOPICO>/gps` | lat, lon, sats, hdop | Cada 30 s, solo con fix |

Comprueba en el tablero (Paso 5) que llegan los tres tópicos: su panel **Mensajes** muestra cada JSON con su tópico y la hora.

### Paso 4 — BLE con nRF Connect (solo hardware)

En nRF Connect, escanea, conecta a **SafeCane**, abre el servicio `6E400001-…` y activa las notificaciones de la característica `6E400003-…`. Llega el JSON de estado cada 5 s, y el de alerta al confirmarse una caída.

### Paso 5 — Tablero web (`tablero/tablero.html`)

El tablero es una sola página HTML: no hay que instalar Node.js ni nada más. Se conecta al mismo broker que el bastón, pero por **WebSocket** (el navegador no puede usar el puerto 1883), y muestra:

- **Estado** del bastón (NORMAL, PRE\_ALERTA, ALERTA, REPOSO), **distancia** al obstáculo, **desnivel**, **uso** y **batería**, de `<TOPICO>/estado`.
- Un **aviso rojo** con la ubicación cuando llega una caída, con sonido, y "Alerta cancelada" cuando se cancela, de `<TOPICO>/alerta`.
- Un **mapa**: punto rojo en caída y azul en el resto, de `<TOPICO>/alerta` y `<TOPICO>/gps`.
- El panel **Mensajes** con los últimos JSON recibidos (reemplaza a MQTT Explorer).

Pasos:

1. Abre `tablero/tablero.html` con doble clic. Necesita internet: el mapa y la librería MQTT se cargan de la web.
2. En **Conexión**, escribe el mismo `TOPICO` del firmware y elige el broker: `broker.hivemq.com` (el del firmware) o `test.mosquitto.org` si cambiaste `MQTT_HOST`. Pulsa **Conectar**. El navegador recuerda estos datos para la próxima vez.
3. El indicador de arriba pasa a verde ("Conectado"). Con el bastón publicando, el estado aparece en menos de 5 s.
4. Haz un clic en cualquier parte de la página: los navegadores no dejan sonar la alarma hasta que el usuario interactúa con ella.

En la opción B, elige **Personalizado** y escribe `ws://localhost:9001`, usuario `baston` y su clave.

Si el profesor quiere verlo en otra laptop, basta con copiarle el archivo.

### Paso 6 — Prueba rápida con un solo sensor (opcional)

Si quieres ver el tablero funcionando antes de tener todo el bastón, `firmware/p07_mqtt_hcsr04/` usa solo el ESP32 y el HC-SR04 (Trig 5, Echo 18, el mismo cableado de la Fase 1A). Pon el nombre y la clave del hotspot y tu `TOPICO`, súbelo y abre el tablero: la distancia se actualiza cada segundo al acercar la mano. No hace falta Wokwi.

### En Wokwi

Con las banderas de la fila "5 en Wokwi", en el Monitor Serie aparece `wifi=1 mqtt=1` a los pocos segundos. Escribe `c`, espera la ALERTA y mira el tablero: llega la notificación y el punto rojo en UTEC. Al pulsar "Cancelar", el tablero muestra "Alerta cancelada" y el punto pasa a azul. **Este es tu plan B para la demo:** todo el flujo de la alerta funciona sin hardware.

### Criterio exacto para avanzar

- [ ] **(Wokwi)** `wifi=1 mqtt=1` en menos de 15 s y el tablero recibe `<TOPICO>/estado` cada 5 s.
- [ ] **(Wokwi)** Con `c`, el tablero muestra la caída en UTEC; al cancelar, muestra "Alerta cancelada".
- [ ] **(real)** Con el hotspot, `wifi=1 mqtt=1` en menos de 15 s tras el arranque.
- [ ] **(real)** nRF Connect ve **SafeCane** y recibe notificaciones.
- [ ] **(real)** Caída simulada al aire libre con GPS fijado: el tablero muestra la notificación y el punto rojo. Mide con cronómetro desde `PRE_ALERTA -> ALERTA` en el Monitor Serie hasta la notificación. En 10 repeticiones, el promedio es **menor a 5 s** (referencia: 4,71 s de Fitriandini et al.).
- [ ] **(real)** Con el hotspot apagado, el bastón sigue vibrando ante obstáculos y entra en PRE\_ALERTA/ALERTA con buzzer igual que en la Fase 3. Al volver el hotspot, una alerta aún activa llega al tablero.

## Fase 6 — Alimentación autónoma

**Para qué sirve:** que el bastón funcione sin cable, con una celda 18650 recargable y protegida. Es la fase donde es más fácil quemar algo: sigue el orden exacto.

**Qué usas:** celda 18650, portaceldas, TP4056 con protección, MT3608, interruptor, 2 resistencias de 100 kΩ, un capacitor de 100 nF y el multímetro.

### Paso 1 — Ajustar el MT3608 antes de conectar cargas

De fábrica puede entregar hasta 28 V: eso destruye el HC-SR04 y el ESP32 al instante.

1. Conecta la celda al TP4056: **B+** al positivo, **B−** al negativo.
2. Conecta **OUT+** del TP4056 → interruptor → **VIN+** del MT3608, y **OUT−** → **VIN−** del MT3608.
3. Sin nada en la salida del MT3608, mide VOUT+ / VOUT− con el multímetro.
4. Gira el potenciómetro multivuelta hasta leer **entre 5,0 y 5,1 V** (suelen hacer falta muchas vueltas en sentido antihorario).

### Paso 2 — Cadena de alimentación

| Desde | Hacia | Nota |
| --- | --- | --- |
| Celda (+) / (−) | TP4056 B+ / B− | Solo la celda va en B+/B− |
| TP4056 OUT+ | Interruptor → MT3608 VIN+ | Salida **protegida**: corta si la celda baja de ≈ 2,5 V |
| TP4056 OUT− | MT3608 VIN− y GND común |  |
| MT3608 VOUT+ (5,0 V) | Riel de 5 V: ESP32 **VIN**, HC-SR04, GPS, motor, buzzer, LED blanco |  |
| ESP32 pin **3V3** | Riel de 3,3 V: MPU6050, FC-51, LDR | No alimentes el pin 3V3 desde la celda |
| TP4056 OUT+ | 100 kΩ → nodo → GPIO 36; del nodo, 100 kΩ → GND y 100 nF → GND | 4,2 V de la celda quedan en 2,1 V en el pin; el capacitor estabiliza la lectura |

Después pon `#define USAR_BATERIA 1` en el firmware. Con esa bandera, si la celda baja de 3,3 V durante 10 s, el bastón da 3 pitidos largos y se apaga de forma ordenada, en vez de reiniciarse en bucle.

**Reglas de uso:**

- **Nunca conectes a la vez el USB del ESP32 y la batería encendida.** Para reprogramar, apaga el interruptor primero.
- **Carga con el interruptor apagado.** Si el bastón consume mientras carga, el TP4056 no detecta el final de la carga.

### Paso 3 — Medir el consumo y la autonomía

1. Carga la celda hasta que el LED del TP4056 pase de rojo a azul o verde.
2. Pon el multímetro como amperímetro **entre OUT+ y el interruptor**. Anota la corriente en REPOSO, en marcha con vibración y en ALERTA.
3. Deja el bastón funcionando frente a un obstáculo a 70 cm y mide cuánto dura hasta los 3 pitidos de batería baja.

El PDF estima 15 h con 170 mA promedio. Con la celda llena, `vbat` debe marcar 4,1–4,2 V.

**Pregunta abierta:** el objetivo dice "≥ 1 día" y la estimación es 15 h. Define en el informe si "1 día" es una jornada de uso (≈ 15 h) o 24 h continuas.

### En Wokwi

Con `USAR_BATERIA 1`, el potenciómetro hace de celda. Gíralo a dos tercios: `vbat` ≈ 4,4 V. Bájalo por debajo de la mitad: a los 10 s aparece `BATERIA BAJA`, suenan 3 pitidos y la simulación se detiene (el ESP32 entró en sueño profundo). Para volver, reinicia la simulación.

### Criterio exacto para avanzar

- [ ] **(Wokwi)** Con el potenciómetro bajo la mitad, aparecen `BATERIA BAJA` y los 3 pitidos.
- [ ] **(real)** El MT3608 marca 5,0–5,1 V **sin carga** y no baja de 4,8 V con todo conectado y el motor vibrando.
- [ ] **(real)** El bastón arranca con el interruptor, sin USB, y llega a `wifi=1 mqtt=1` (míralo en el tablero).
- [ ] **(real)** `vbat` difiere en menos de 0,1 V de lo que mide el multímetro en la celda.
- [ ] **(real)** Tienes anotadas las tres corrientes y la autonomía en horas.
- [ ] **(real)** Al conectar el cargador en el micro-USB del TP4056, con el interruptor apagado, el LED rojo indica carga.

## Fase 7 — Montaje en el bastón

**Para qué sirve:** pasar de la protoboard a un bastón que se pueda usar caminando en la demo, recargar y reprogramar sin abrirlo.

**Qué usas:** perfboard, espadines hembra, caja 3D, cable flexible, conectores JST, funda espiral, termorretráctil, bridas, velcro y tornillos M3.

### Distribución física

| Zona | Qué va ahí | Requisito |
| --- | --- | --- |
| Caja impresa junto a la empuñadura | ESP32, celda, TP4056, MT3608, perfboard, motor, buzzer | Micro-USB del ESP32 y del TP4056 accesibles desde fuera. El motor pegado a la pared de la caja que toca la mano |
| Tapa superior de la caja | **GPS**, antena cerámica hacia arriba | Nada metálico encima. Nunca dentro del tubo de aluminio |
| Frente de la caja | Pulsador con capuchón alto | Alcanzable con el pulgar |
| Base de la caja | MPU6050 | **Eje Z paralelo al tubo**; fijo con tornillos, no con velcro |
| Tubo, tercio superior, cara frontal | HC-SR04 | Al frente con ≈ 10° hacia abajo |
| Caja, cara superior | LED blanco y LDR | El LDR hacia arriba, lejos de la mano y del LED |
| Tubo, a 4–6 cm de la punta | **FC-51** en su soporte | Lentes a 2–4 cm del piso con el bastón en ángulo de uso |

### Montaje del FC-51

1. Imprime un soporte que abrace el tubo a 4–6 cm de la punta y sostenga el módulo con las lentes hacia el piso, inclinadas 20–30° hacia adelante.
2. Deja el potenciómetro accesible para recalibrar sin desmontar.
3. Comprueba que ni la contera ni las bridas entran en el campo de visión. Si ve su propia contera, quedará en `piso=1` siempre.
4. Protege la placa con termorretráctil o una tapa, sin tapar las lentes.
5. Repite la calibración de la Fase 1, paso 3, **ya montado** y en el piso de la demo.

### Cables a lo largo del bastón

- **Bastón rígido o telescópico (recomendado):** los cables del HC-SR04 (4 hilos) y del FC-51 (3 hilos) van por fuera, dentro de la funda espiral y sujetos con bridas cada 15 cm.
- **Si solo tienes uno plegable:** no pases cables por dentro (ahí va el elástico). Llévalos por fuera y pon un conector JST en cada articulación que cruce el cable. Para la demo, puedes fijar las articulaciones con cinta para que no se plieguen.
- Usa conectores JST entre la caja y cada cable: así desmontas un sensor sin desoldar.

### Orden de armado

1. Imprime la caja y el soporte, y verifica que encajan **antes** de soldar.
2. Pasa el circuito a perfboard con espadines hembra para cada módulo.
3. Tiende los cables con la funda espiral y los conectores.
4. Deja accesibles el micro-USB del ESP32 (reprogramar) y el del TP4056 (cargar).

### Criterio exacto para avanzar

- [ ] **(real)** 20 barridos y 10 golpes de la punta contra el piso sin que se suelte nada ni se pierda ninguna lectura.
- [ ] **(real)** Recalibraste el FC-51 ya montado y cumple de nuevo el paso 3 de la Fase 1.
- [ ] **(real)** 15 minutos de caminata sin reinicios (ningún `=== SafeCane` nuevo en el Monitor Serie ni hueco en el tablero).
- [ ] **(real)** Se recarga y se reprograma sin abrir la caja.
- [ ] **(real)** El GPS fija al aire libre con el bastón sujetado normalmente.

## Fase 8 — Validación final

**Para qué sirve:** reunir en una tabla los números medidos que van en la sección de resultados del informe, y declarar las limitaciones reales.

### Tabla de resultados (llena la columna Medido)

| Métrica | Objetivo | Medido | Viene de | Referencia |
| --- | --- | --- | --- | --- |
| Error de distancia (concreto) | < 3 cm |  | Fase 1, paso 1 | — |
| Alcance útil de detección | 2 m |  | Fase 1, paso 1 | — |
| Sensibilidad de caídas | ≥ 90 % |  | Fase 4 | — |
| Especificidad de caídas | — |  | Fase 4 | — |
| Falsas alarmas por hora de marcha | — |  | Fase 4 (`marcha_*`) | — |
| Detección de escalones que bajan | ≥ 90 % |  | Fase 4 (`escalon_*`) | Aporte propio |
| Detección de bordillos | ≥ 90 % |  | Fase 4 (`bordillo_*`) | Aporte propio |
| Falsos avisos de desnivel por minuto en plano | — |  | Fase 4 (`plano_01`) | — |
| Tiempo de primer fix GPS | < 2 min |  | Fase 1, paso 4 | — |
| Error de posición GPS | — |  | Fase 1, paso 4 | 4,63 m (Fitriandini et al.) |
| Latencia de alerta al tablero | < 5 s |  | Fase 5 | 4,71 s (Fitriandini et al.) |
| Autonomía | ≥ 1 día (definir) |  | Fase 6 | ≈ 50 h (ref. \[9\] del informe) |
| Lógica completa validada en simulación | Sí |  | Fases 0B–6 (Wokwi) | Enlace al proyecto de Wokwi |

### Limitaciones a declarar

Del informe original: mide distancia pero no clasifica el obstáculo; el cono de 15° no cubre obstáculos laterales ni a la altura de la cabeza; el ultrasonido falla con tela y vidrio; el GPS se degrada en interiores; es un prototipo académico, no un equipo médico; y las pruebas con un grupo reducido no son estadísticamente generalizables.

Nuevas, por el FC-51:

- **Detecta reflexión, no distancia:** las superficies negras, mojadas o brillantes reflejan poco infrarrojo y pueden parecer "vacío". El potenciómetro se calibra para el piso más oscuro previsto.
- **El sol directo puede cegar al receptor.**
- **Aviso tardío:** va junto a la punta, así que avisa cuando la punta llega al borde, no antes. Trabajo futuro: un sensor de distancia infrarrojo (Sharp GP2Y0A21, 10–80 cm) o ToF (VL53L0X) mirando 30–40 cm por delante.
- **Depende del ángulo de uso:** el aviso se desactiva fuera de 20–75°; con técnicas que levantan mucho el bastón puede perder escalones o dar avisos falsos.
- **Sin sensor de agarre, no distingue "se cayó el bastón" de "se cayó el usuario":** esos casos dependen de que el usuario cancele la pre-alerta.
- **Broker público (opción A):** sin autenticación ni cifrado; aceptable para la demo, no para un producto.

### Criterio exacto de cierre

- [ ] Todas las filas de la tabla tienen valor medido (o "no medido" con el motivo).
- [ ] Sensibilidad ≥ 90 % y latencia < 5 s alcanzadas, o explicadas en el informe.
- [ ] Las limitaciones nuevas están en el informe.
- [ ] Las secciones del informe que mencionaban el FSR se actualizaron: Tabla II (FSR → FC-51), III-E (reflexión infrarroja), V-D (uso activo = movimiento del MPU6050; aviso de desnivel), alcances, Figura 1 y la fila "Propuesta" de la Tabla I.

## Anexo A — Diagnóstico de fallos

### Hardware

| Síntoma | Causa más probable | Solución |
| --- | --- | --- |
| No aparece el puerto | Cable USB de solo carga | Cambia de cable antes que nada |
| `Connecting…_____` al subir | La placa no entra en modo de carga | Mantén BOOT presionado |
| Monitor Serie con basura | Velocidad incorrecta | 115200 baudios |
| Reinicios al activar el motor | Falta el diodo de rueda libre o el electrolítico | 1N4007 en el motor y electrolítico en el riel de 5 V |
| `piso` siempre 0 | Poco alcance, módulo muy alto o VCC/GND invertidos | Más alcance en el potenciómetro; lentes a 2–3 cm del piso; revisa la alimentación |
| `piso` siempre 1, incluso al aire | Demasiado alcance, o la contera o una brida en el campo de visión | Reduce el alcance; despeja el frente del sensor |
| `piso` cambia pero nunca aparece `DESNIVEL` | Bastón fuera del ángulo de uso (por ejemplo, MPU plano en la mesa) | Inclínalo unos 45°; revisa `incl` en el Monitor Serie |
| Avisos de desnivel al caminar en plano | Piso oscuro o bastón levantado entre toques | Más alcance o sube `T_DESNIVEL` |
| El MPU6050 no aparece en el escáner | SDA y SCL cruzados | Intercámbialos |
| `svm` clavada cerca de 2 g | Acelerómetro en ±2 g | Revisa `Wire.write(0x10)` en ACCEL\_CONFIG |
| La inclinación no sube al tumbar el bastón | Eje Z del MPU no está a lo largo del tubo | Reorienta el módulo o cambia `az` por el eje correcto |
| El GPS no fija nunca | Bajo techo o tapado por metal | Al aire libre, antena hacia arriba, fuera del tubo |
| El HC-SR04 devuelve -1 siempre | Alimentado a 3,3 V o Echo sin divisor | Riel de 5 V y divisor 1 kΩ / 2 kΩ |
| `wifi=0` permanente | Hotspot en 5 GHz o credenciales mal escritas | "Maximizar compatibilidad" (iPhone) o banda 2,4 GHz (Android) |
| `wifi=1` pero `mqtt=0` | Broker caído o IP incorrecta (opción B) | Prueba `test.mosquitto.org`; en la opción B revisa `ipconfig getifaddr en0` |
| El tablero no pasa a "Conectado" | Sin internet, broker caído o (opción B) falta el `listener 9001` | Prueba el otro broker en **Conexión**; revisa el `mosquitto.conf` |
| Tablero conectado pero sin datos | `TOPICO` distinto al del firmware (las mayúsculas cuentan) o broker distinto | Copia el tópico exacto de `safecane.ino`; usa el mismo broker que `MQTT_HOST` |
| La caída llega pero el mapa no muestra el punto | Sin fix: lat y lon llegan en 0 | Sal al aire libre; el aviso dice "sin ubicación GPS" |
| La alarma del tablero no suena | El navegador bloquea el audio hasta un clic | Haz un clic en la página antes de la demo |
| El buzzer no suena en el bastón | Quedó `#define WOKWI 1` | Ponlo en `0` |
| Se apaga con 3 pitidos recién cargado | `vbat` mal medido: divisor ausente o mal soldado | Revisa el divisor; mientras tanto, `USAR_BATERIA 0` |
| `uso=0` mientras camina | `UMBRAL_MOVIMIENTO` alto | Bájalo de 0,15 a 0,10 g |
| No arranca con algo conectado | Algo en un pin de arranque | Libera GPIO 0, 2, 12 y 15 |

### Wokwi

| Síntoma | Causa más probable | Solución |
| --- | --- | --- |
| `fatal error: TinyGPSPlus.h` (o PubSubClient / ArduinoJson) | Falta la librería en el proyecto | Pestaña Library Manager → **+** → agrégala |
| `NimBLEDevice.h: No such file` | Quedó `WOKWI 0` con `USAR_RED 1` | En Wokwi, `#define WOKWI 1` (Wokwi no simula BLE) |
| En reposo `svm` no es 1,00 (por ejemplo 4,00) | El MPU simulado no aplica el rango ±8 g | Solo en Wokwi: `LSB_8G` = 4096 × `svm` medida (4,00 → `16384.0`). Vuelve a 4096 para el bastón |
| La caída `c` no hace nada | El Monitor Serie no tenía el foco | Haz clic en la consola, escribe `c` y pulsa Enter |
| No suena el buzzer | El navegador tiene el audio bloqueado | Clic en el ícono de sonido de la simulación |
| Nunca aparece `DESNIVEL` | MPU vertical (incl = 0°) | accelX = 0,7 y accelZ = 0,7 |
| Pasa a REPOSO mientras pruebas | 2 min sin cambios en el MPU | Sube accelZ a 1,5 un instante |
| `wifi=1` pero `mqtt=0` | Broker público caído | Cambia `MQTT_HOST` a `test.mosquitto.org` |
| `ledcAttach` no compila | Wokwi con core 2.x | El código ya tiene la alternativa para 2.x; revisa que copiaste las funciones `motorInit` y `motor` |
| Las piezas aparecen pero sin cables | `diagram.json` pegado incompleto | Vuelve a copiar el archivo entero |

## Anexo B — Plan por semanas

Ajústalo a la fecha de la demo. Si tienes menos de 6 semanas, junta las semanas 4 y 5 y recorta el tablero (texto y mapa bastan), nunca la calibración.

1. **Semana 1:** compras. Mientras llegan, Fases 0B, 1, 3 y 5 **en Wokwi** (casillas Wokwi).
2. **Semana 2:** Fase 0 y Fase 1 en hardware (casillas reales).
3. **Semana 3:** Fases 2 y 3 en hardware.
4. **Semana 4:** Fase 4 (calibración) y Fase 5 en hardware.
5. **Semana 5:** Fases 6 y 7 (batería y montaje).
6. **Semana 6:** Fase 8, informe y ensayo de la demo (Anexo C).

## Anexo C — Guion y checklist de la demo

### Guion (7 minutos)

| Min. | Qué muestras | Qué dices |
| --- | --- | --- |
| 0–1 | El bastón y el tablero en la laptop | Problema, objetivo y arquitectura (sección 2) |
| 1–2 | Acercas una mano o una pared al HC-SR04 | Tres zonas de vibración según la distancia |
| 2–3 | Llevas la punta al borde de un escalón o de una tarima | El FC-51 deja de ver el piso: motor y buzzer pulsan juntos. Limitación: avisa al llegar al borde |
| 3–5 | Caída sobre colchoneta; luego otra cancelada con el pulsador | PRE\_ALERTA, 10 s para cancelar, ALERTA con buzzer; el tablero muestra la caída en el mapa |
| 5–6 | Tabla de resultados (Fase 8) | Sensibilidad, detección de escalones, latencia, autonomía |
| 6–7 | Proyecto de Wokwi abierto | Todo el firmware es verificable en simulación; limitaciones y trabajo futuro |

### Checklist

- [ ] Celda cargada (LED del TP4056 en azul o verde).
- [ ] FC-51 calibrado **en el piso del lugar de la demo**, ya montado. Lleva un escalón o una tarima de al menos 15 cm si el lugar no tiene.
- [ ] Hotspot del celular en 2,4 GHz encendido, con el nombre y la clave del firmware.
- [ ] El tablero (`tablero/tablero.html`) está en verde y recibe `<TOPICO>/estado` antes de empezar. Haz un clic en la página para habilitar el sonido.
- [ ] GPS con fix confirmado al aire libre (o explicar que dentro del aula no fija y mostrar la última posición).
- [ ] Colchoneta para la caída.
- [ ] Proyecto de Wokwi abierto en otra pestaña con `WOKWI 1`, `USAR_RED 1` (**plan B** si falla el hardware).
- [ ] Sketches de prueba a mano por si algo falla en vivo.
- [ ] Plan C sin red: mostrar la alerta local (buzzer + vibración).
