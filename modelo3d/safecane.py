# SafeCane 3D paramétrico (CadQuery). Ejecutar: python safecane.py
# Eje Z = a lo largo del bastón. Frente (HC-SR04, pulsador) = -Y. Palma (TCRT5000) = +Y.
import os
import cadquery as cq

# ---------- Parámetros (mm) ----------
TUBO_D = 22.0          # diámetro exterior del tubo de aluminio
TUBO_L = 600.0         # largo de la maqueta del tubo
PARED = 3.0            # pared de la carcasa
MANGO_D = 44.0         # diámetro exterior de la empuñadura
MANGO_L = 170.0
VENTANA_IR = (6.0, 10.0)   # ventana TCRT5000 (ancho x alto)
PULSADOR_D = 12.0
CONTERA_L = 40.0

def cilindro(d, l, z0=0.0):
    return cq.Workplane("XY").workplane(offset=z0).circle(d / 2).extrude(l)

def caja(x, y, z, cx, cy, cz):
    return cq.Workplane("XY").box(x, y, z).translate((cx, cy, cz))

# ---------- Empuñadura (dos mitades, split en plano YZ) ----------
z_m0 = CONTERA_L + TUBO_L
mango = cilindro(MANGO_D, MANGO_L, z_m0)
hueco = cilindro(MANGO_D - 2 * PARED, MANGO_L - 2 * PARED, z_m0 + PARED)
mango = mango.cut(hueco)
mango = mango.cut(cilindro(TUBO_D, PARED + 20, z_m0 - 1))               # entrada del tubo
z_palma = z_m0 + MANGO_L * 0.55
mango = mango.cut(caja(VENTANA_IR[0], 30, VENTANA_IR[1], 0, MANGO_D / 2, z_palma))   # ventana IR (+Y)
z_boton = z_m0 + MANGO_L * 0.8
mango = mango.cut(cq.Workplane("XZ").workplane(offset=0).center(0, z_boton)
                  .circle(PULSADOR_D / 2).extrude(MANGO_D))             # pulsador (-Y)
mango = mango.cut(caja(12, 30, 8, 0, -MANGO_D / 2, z_m0 + MANGO_L * 0.3))  # acceso USB (-Y)
mitad_a = mango.intersect(caja(MANGO_D, MANGO_D, MANGO_L, MANGO_D / 4, 0, z_m0 + MANGO_L / 2))
mitad_b = mango.intersect(caja(MANGO_D, MANGO_D, MANGO_L, -MANGO_D / 4, 0, z_m0 + MANGO_L / 2))

# ---------- Tubo y contera ----------
tubo = cilindro(TUBO_D, TUBO_L, CONTERA_L).cut(cilindro(TUBO_D - 2, TUBO_L, CONTERA_L))
contera = cilindro(TUBO_D + 4, CONTERA_L).cut(cilindro(TUBO_D - 2, CONTERA_L - 5, 5))

# ---------- Módulos (cajas con dimensiones típicas) ----------
zi = z_m0 + PARED
modulos = {
    "esp32":    caja(28, 12, 51, 0, 0, zi + 100),
    "bateria":  cilindro(18.6, 65, zi + 5).translate((0, 0, 0)),
    "tp4056":   caja(17, 4, 26, 0, 6, zi + 40),
    "mt3608":   caja(17, 5, 36, 0, -6, zi + 140),
    "mpu6050":  caja(16, 3, 21, 0, 0, zi + 12),
    "tcrt5000": caja(10, 6, 5.5, 0, MANGO_D / 2 - PARED - 3, z_palma),
    "pulsador": cq.Workplane("XZ").center(0, z_boton).circle(PULSADOR_D / 2).extrude(10) \
                .translate((0, -MANGO_D / 2 + 4, 0)),
    "hcsr04":   caja(45, 20, 15, 0, -TUBO_D / 2 - 10, z_m0 - 120),
    "led":      cilindro(5, 8, z_m0 - 200).translate((0, TUBO_D / 2 + 2, 0)),
    "ldr":      cilindro(5, 4, z_m0 - 230).translate((0, TUBO_D / 2 + 2, 0)),
    "gps":      caja(25, 25, 6, 0, 0, z_m0 - 40),
    "piezo":    cilindro(27, 1, CONTERA_L - 8),
}
# El HC-SR04 se inclina 10° hacia abajo
modulos["hcsr04"] = modulos["hcsr04"].rotate((0, -TUBO_D / 2, z_m0 - 120), (1, -TUBO_D / 2, z_m0 - 120), 10)

# ---------- Exportar ----------
piezas = {"mango_a": mitad_a, "mango_b": mitad_b, "tubo": tubo, "contera": contera, **modulos}
salida = os.path.join(os.path.dirname(os.path.abspath(__file__)), "salida")
os.makedirs(salida, exist_ok=True)
for nombre in ("mango_a", "mango_b", "contera"):                       # piezas a imprimir
    cq.exporters.export(piezas[nombre], os.path.join(salida, f"{nombre}.stl"))
colores = {"mango_a": (0.1, 0.1, 0.1), "mango_b": (0.1, 0.1, 0.1), "tubo": (0.75, 0.75, 0.8),
           "contera": (0.1, 0.1, 0.1), "esp32": (0.1, 0.3, 0.7), "bateria": (0.2, 0.7, 0.3),
           "tcrt5000": (0.9, 0.1, 0.1), "hcsr04": (0.1, 0.5, 0.9), "pulsador": (0.9, 0.7, 0.1)}
ens = cq.Assembly(name="safecane")
for n, p in piezas.items():
    ens.add(p, name=n, color=cq.Color(*colores.get(n, (0.6, 0.6, 0.6))))
ens.save(os.path.join(salida, "safecane.step"))
ens.save(os.path.join(salida, "safecane.glb"))
print("OK ->", salida)
