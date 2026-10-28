# Lista de compras MVP — SafeCane (FC-51, demo)

Actualizada: 2026-10-26. Base: `guia_safecane_infrarrojo.md` (versión FC-51 + Wokwi). Para la parte en Wokwi no hay que comprar nada.

Estados: `Pendiente` · `Comprado` · `Tengo`. Cambia la columna **Estado** a medida que compres.

## Procesamiento y sensores

| # | Componente | Especificación | Cant. | Para qué | Estado | Nota |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | ESP32-WROOM-32 DevKit V1 | 38 pines; no la variante "U" | 1 | Cerebro del bastón | Pendiente | |
| 2 | HC-SR04 | 5 V, 2–400 cm | 1 | Obstáculos | Pendiente | |
| 3 | MPU6050 (GY-521) | Acelerómetro I2C | 1 | Caídas e inclinación | Pendiente | |
| 4 | FC-51 | VCC, GND, OUT; potenciómetro | 1 | Escalones que bajan y bordillos | Tengo | VCC a 3V3. Va a 4–6 cm de la punta |
| 5 | GPS NEO-6M (GY-NEO6MV2) | Antena cerámica | 1 | Ubicación | Pendiente | |
| 6 | LDR GL5528 | 8–20 kΩ a 10 lux | 1 | Luz ambiente | Pendiente | |
| 7 | Pulsador + capuchón alto | 12 mm, normalmente abierto | 1 | Cancelar alerta | Pendiente | |

## Actuadores

| # | Componente | Especificación | Cant. | Para qué | Estado | Nota |
| --- | --- | --- | --- | --- | --- | --- |
| 8 | Motor vibrador | 5 V | 1 | Aviso háptico | Tengo | Riel de 5 V, base con 470 Ω |
| 9 | Buzzer activo | 5 V, ≈85 dB | 1 | Alarma sonora | Pendiente | Que diga "activo" (suena con solo darle 5 V) |
| 10 | LED blanco alto brillo | 5 mm | 1 | Visibilidad nocturna | Pendiente | |

## Alimentación

| # | Componente | Especificación | Cant. | Para qué | Estado | Nota |
| --- | --- | --- | --- | --- | --- | --- |
| 11 | Celda 18650 | 3000 mAh reales (Samsung 30Q, LG MJ1, Sony VTC) | 1 | Energía | Pendiente | Evita las de 6000–9800 mAh: son falsas |
| 12 | Portaceldas 18650 | Con cables | 1 | Sujetar la celda | Pendiente | |
| 13 | TP4056 con protección | Chips DW01A + FS8205A, pads OUT+/OUT− | 1 | Carga y protección | Pendiente | |
| 14 | MT3608 | Elevador ajustable a 5,0 V | 1 | Riel de 5 V | Pendiente | |
| 15 | Interruptor deslizante | 3 A | 1 | Encendido | Pendiente | |
| 16 | Cable micro-USB de datos | 2 m, no solo de carga | 1 | Programar y grabar la Fase 4 caminando | Pendiente | |

## Pasivos y discretos

| # | Componente | Valor | Cant. | Dónde va | Estado |
| --- | --- | --- | --- | --- | --- |
| 17 | Resistencia | 10 kΩ | 4 | Divisor del LDR (1) + 3 de reserva | Pendiente |
| 18 | Resistencia | 100 kΩ | 2 | Divisor de batería | Pendiente |
| 19 | Resistencia | 1 kΩ | 3 | Divisor del Echo (1) + bases del buzzer y del LED (2) | Pendiente |
| 20 | Resistencia | 470 Ω | 1 | Base del transistor del motor | Pendiente |
| 21 | Resistencia | 2 kΩ (o 2 × 1 kΩ) | 1 | Divisor del Echo | Pendiente |
| 22 | Resistencia | 100 Ω | 1 | LED blanco | Pendiente |
| 23 | Transistor NPN 2N2222 | 600 mA | 3 | Motor, buzzer y LED | Pendiente |
| 24 | Diodo 1N4007 | — | 2 | Rueda libre en motor y buzzer | Pendiente |
| 25 | Capacitor electrolítico | 100–470 µF / 10 V | 2 | Rieles de 5 V y 3,3 V | Pendiente |
| 26 | Capacitor cerámico | 100 nF | 7 | HC-SR04, MPU6050, GPS, FC-51, ESP32, bornes del motor, divisor de batería | Pendiente |

## Mecánica y cableado

| # | Componente | Especificación | Cant. | Para qué | Estado | Nota |
| --- | --- | --- | --- | --- | --- | --- |
| 27 | Bastón rígido o telescópico | Contera desmontable | 1 | Estructura | Pendiente | Si es plegable: cables por fuera y JST en las articulaciones |
| 28 | Protoboard | 830 puntos | 1 | Fases 1–5 | Pendiente | |
| 29 | Perfboard | 7 × 9 cm | 2 | Montaje final | Pendiente | |
| 30 | Jumpers Dupont | M-M, M-H, H-H | 3 juegos | Conexiones | Pendiente | |
| 31 | Espadines | Macho y hembra, tira de 40 | 2 | Zócalos de módulos | Pendiente | |
| 32 | Cable flexible AWG 26–28 | 4 hilos, ≈1,5 m | 1 | HC-SR04 y FC-51 | Pendiente | |
| 33 | Conectores JST-XH | 3 y 4 pines, macho + hembra | 4 pares | Sensores desmontables | Pendiente | |
| 34 | Funda espiral para cables | — | 1 m | Cables a lo largo del tubo | Pendiente | |
| 35 | Caja 3D en PLA | Caja de empuñadura (GPS arriba) + soporte del FC-51 | 2 piezas | Alojamiento | Pendiente | Confirma acceso a impresora 3D |
| 36 | Termorretráctil, bridas, velcro, tornillos M3 | Surtido | 1 juego | Acabado | Pendiente | |

## Herramientas (si no las tienes)

| # | Herramienta | Estado | Nota |
| --- | --- | --- | --- |
| 37 | Multímetro | Pendiente | Indispensable: ajustar el MT3608, medir OUT del FC-51 y el consumo |
| 38 | Cautín 30–40 W + estaño con flux | Pendiente | |
| 39 | Pelacables y pinza fina | Pendiente | |

## Repuestos recomendados (opcionales)

| Componente | Por qué |
| --- | --- |
| 1 ESP32 DevKit extra | Es lo que más se quema en la Fase 6; sin él no hay demo |
| 1 MT3608 extra | Si sale de fábrica a 28 V y lo conectas antes de ajustarlo, se lleva lo demás |

## Ya no se compra

| Componente | Motivo |
| --- | --- |
| FSR 402 y almohadilla | Reemplazado por el FC-51 |
| TCRT5000 | Solo ve a 1–2 cm; el FC-51 sí sirve |
| Motor vibrador de 3 V | Ya tienes uno de 5 V |
| Disco piezoeléctrico, 1 MΩ, 1N4148, cable apantallado, epóxico | El uso activo ahora lo mide el MPU6050 |
| Sharp GP2Y0A21 / VL53L0X | No hace falta para la demo; queda como trabajo futuro |

## Antes de pagar

1. TP4056: que diga "con protección" y tenga pads OUT+/OUT−.
2. NEO-6M: con antena cerámica.
3. Buzzer: "activo", no "pasivo".
4. Celda 18650: de marca verificable.
