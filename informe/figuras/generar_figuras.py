# generar_figuras.py: prepara las fotos y genera las figuras de resultados del informe.
# Uso (desde la carpeta informe):  python figuras/generar_figuras.py
# Las curvas de caida salen de una copia en Python de la logica de firmware/safecane/safecane.ino
# aplicada al perfil de caida que el propio firmware usa en Wokwi (funcion caidaSimulada).
import math
import os

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from PIL import Image, ImageDraw, ImageFont, ImageOps

AQUI = os.path.dirname(os.path.abspath(__file__))
FOTOS = os.path.join(AQUI, "..", "FOTOS SCAFENE")
SALIDA = AQUI

plt.rcParams.update({
    "font.family": "serif",
    "font.size": 8,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "axes.grid": True,
    "grid.alpha": 0.3,
    "savefig.bbox": "tight",
    "savefig.pad_inches": 0.02,
})

AZUL, NARANJA, ROJO, GRIS, VERDE = "#1f4e79", "#d9822b", "#b22222", "#6b6b6b", "#2e7d32"


# ---------------------------------------------------------------- fotos
def preparar_fotos():
    nombres = {
        "WhatsApp Image 2026-11-19 at 11.27.02 AM.jpeg": "foto_noche.jpg",
        "WhatsApp Image 2026-11-19 at 11.28.22 AM.jpeg": "foto_bateria.jpg",
        "WhatsApp Image 2026-11-19 at 11.28.22 AM (1).jpeg": "foto_mesa.jpg",
        "WhatsApp Image 2026-11-19 at 11.28.22 AM (2).jpeg": "foto_protoboard.jpg",
        "WhatsApp Image 2026-11-19 at 11.28.22 AM (3).jpeg": "foto_grosor.jpg",
        "WhatsApp Image 2026-11-19 at 11.28.23 AM.jpeg": "foto_baston_final.jpg",
        "WhatsApp Image 2026-11-19 at 11.32.52 AM.jpeg": "foto_primer_prototipo.jpg",
    }
    for origen, destino in nombres.items():
        im = ImageOps.exif_transpose(Image.open(os.path.join(FOTOS, origen))).convert("RGB")
        im.thumbnail((1100, 1100))
        im.save(os.path.join(SALIDA, destino), quality=85)

    # El diseño de la caja se separa en dos recortes: el bastón con la ubicación de los
    # sensores y las vistas de la caja. La tabla de pines del dibujo no se usa porque el
    # mapa definitivo es el del firmware (Tabla del informe).
    caja = Image.open(os.path.join(FOTOS, "WhatsApp Image 2026-11-19 at 11.30.46 AM.jpeg")).convert("RGB")
    # Título del panel central reescrito sin raya
    lapiz = ImageDraw.Draw(caja)
    lapiz.rectangle((628, 61, 1112, 86), fill=(1, 56, 113))
    fuente = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 19)
    lapiz.text((637, 73), "Vista interna: batería sobre el protoboard", font=fuente,
               fill="white", anchor="lm")
    caja.crop((0, 58, 233, 1018)).save(os.path.join(SALIDA, "diseno_baston.jpg"), quality=90)
    vistas = caja.crop((236, 58, 1536, 562))
    # Tapa el encabezado "Vistas de la caja" que queda cortado en la columna derecha
    fondo = caja.getpixel((1520, 470))
    ImageDraw.Draw(vistas).rectangle((1175 - 236, 474 - 58, vistas.width, vistas.height), fill=fondo)
    vistas.save(os.path.join(SALIDA, "diseno_caja.jpg"), quality=90)


# ---------------------------------------------------------------- lógica del firmware
SVM_CAIDA_LIBRE, SVM_IMPACTO, INCLIN_MIN = 0.5, 2.5, 60.0
VENTANA_IMPACTO, T_ASENTAR, INCLIN_SOSTENIDA, T_CANCELACION = 1000, 500, 2000, 10000


def perfil_wokwi(t_ms, t0):
    """Copia de caidaSimulada(): reposo vertical, caida libre, impacto y baston tumbado."""
    if t_ms < t0:
        return 0.0, 0.0, 1.0
    t = t_ms - t0
    if t < 300:
        return 0.0, 0.0, 0.1
    if t < 400:
        return 2.0, 2.0, 2.0
    return 1.0, 0.0, 0.0


def simular_caida():
    t0 = 1000
    estado, t_pre = "NORMAL", None
    hubo_libre, t_libre, t_impacto = False, 0, 0
    filas, eventos = [], {}
    for ahora in range(0, 16000, 20):          # la IMU se lee cada 20 ms (50 Hz)
        ax, ay, az = perfil_wokwi(ahora, t0)
        svm = math.sqrt(ax * ax + ay * ay + az * az)
        incl = math.degrees(math.acos(max(-1, min(1, az / max(svm, 0.01)))))
        if svm < SVM_CAIDA_LIBRE:
            if not hubo_libre:
                eventos.setdefault("libre", ahora)
            hubo_libre, t_libre = True, ahora
        if hubo_libre and ahora - t_libre < VENTANA_IMPACTO and svm > SVM_IMPACTO:
            hubo_libre, t_impacto = False, ahora
            eventos.setdefault("impacto", ahora)
        if hubo_libre and ahora - t_libre >= VENTANA_IMPACTO:
            hubo_libre = False
        if t_impacto and estado == "NORMAL" and ahora - t_impacto >= T_ASENTAR:
            if incl < INCLIN_MIN:
                t_impacto = 0
            elif ahora - t_impacto >= T_ASENTAR + INCLIN_SOSTENIDA:
                t_impacto, estado, t_pre = 0, "PRE_ALERTA", ahora
                eventos["pre"] = ahora
        if estado == "PRE_ALERTA" and ahora - t_pre >= T_CANCELACION:
            estado = "ALERTA"
            eventos["alerta"] = ahora
        filas.append((ahora, svm, incl, estado))
    return filas, eventos


def figura_caida():
    filas, ev = simular_caida()
    t = [f[0] / 1000 for f in filas]
    svm = [f[1] for f in filas]
    incl = [f[2] for f in filas]
    nivel = {"NORMAL": 0, "PRE_ALERTA": 1, "ALERTA": 2}
    est = [nivel[f[3]] for f in filas]

    fig, ejes = plt.subplots(3, 1, figsize=(3.45, 3.6), sharex=True,
                             gridspec_kw={"height_ratios": [1.3, 1.0, 0.6]})
    a1, a2, a3 = ejes
    a1.plot(t, svm, color=AZUL, lw=1.1, drawstyle="steps-post")
    a1.axhline(SVM_CAIDA_LIBRE, color=NARANJA, ls="--", lw=0.8)
    a1.axhline(SVM_IMPACTO, color=ROJO, ls="--", lw=0.8)
    a1.text(15.8, SVM_CAIDA_LIBRE + 0.08, "0,5 g", color=NARANJA, ha="right", fontsize=7)
    a1.text(15.8, SVM_IMPACTO + 0.08, "2,5 g", color=ROJO, ha="right", fontsize=7)
    a1.set_ylabel("SVM (g)")
    a1.set_ylim(0, 3.9)
    a1.annotate("caída libre", xy=(ev["libre"] / 1000 + 0.15, 0.12), xytext=(2.6, 1.6),
                fontsize=7, arrowprops=dict(arrowstyle="->", lw=0.6))
    a1.annotate("impacto 3,46 g", xy=(ev["impacto"] / 1000 + 0.05, 3.46), xytext=(2.6, 3.2),
                fontsize=7, arrowprops=dict(arrowstyle="->", lw=0.6))

    a2.plot(t, incl, color=VERDE, lw=1.1, drawstyle="steps-post")
    a2.axhline(INCLIN_MIN, color=ROJO, ls="--", lw=0.8)
    a2.text(15.8, INCLIN_MIN + 4, "60°", color=ROJO, ha="right", fontsize=7)
    ti = ev["impacto"] / 1000
    a2.axvspan(ti + 0.5, ti + 2.5, color=GRIS, alpha=0.15, lw=0)
    a2.text(ti + 1.5, 20, "2 s", ha="center", fontsize=7)
    a2.set_ylabel("Inclinación (°)")
    a2.set_ylim(0, 100)

    a3.plot(t, est, color="black", lw=1.1, drawstyle="steps-post")
    a3.set_yticks([0, 1, 2])
    a3.set_yticklabels(["NORMAL", "PRE", "ALERTA"], fontsize=6.5)
    a3.set_ylim(-0.3, 2.4)
    a3.axvspan(ev["pre"] / 1000, ev["alerta"] / 1000, color=NARANJA, alpha=0.12, lw=0)
    a3.text((ev["pre"] + ev["alerta"]) / 2000, 1.35, "10 s para cancelar", ha="center", fontsize=7)
    a3.set_xlabel("Tiempo (s)")
    a3.set_xlim(0, 16)
    fig.align_ylabels(ejes)
    fig.savefig(os.path.join(SALIDA, "resultado_caida.pdf"))
    plt.close(fig)
    return ev


def duty_obstaculo(d):
    """Intensidad media del motor según tareaActuadores()."""
    if 0 < d < 50:
        return 255
    if 0 < d < 100:
        return 200 * 0.5            # pulsos de 100 ms encendido y 100 ms apagado
    if 0 < d < 200:
        return 120 * 0.5            # pulsos de 300 ms encendido y 300 ms apagado
    return 0


def figura_vibracion():
    d = [x / 2 for x in range(0, 501)]
    pct = [duty_obstaculo(x) / 255 * 100 for x in d]
    fig, ax = plt.subplots(figsize=(3.45, 1.7))
    ax.fill_between(d, pct, step="post", color=AZUL, alpha=0.18, lw=0)
    ax.plot(d, pct, color=AZUL, lw=1.2, drawstyle="steps-post")
    for x0, x1, txt in ((0, 50, "continua"), (50, 100, "rápida\n100/100 ms"),
                        (100, 200, "lenta\n300/300 ms"), (200, 250, "sin aviso")):
        ax.text((x0 + x1) / 2, 104, txt, ha="center", va="bottom", fontsize=6.5)
    ax.set_xlim(0, 250)
    ax.set_ylim(0, 125)
    ax.set_xlabel("Distancia al obstáculo (cm)")
    ax.set_ylabel("Intensidad media (%)")
    fig.savefig(os.path.join(SALIDA, "resultado_vibracion.pdf"))
    plt.close(fig)


def figura_consumo():
    partes = [("ESP32 con\nWiFi y BLE", 80), ("GPS\nNEO-6M", 45), ("Sensores\ny LED", 20),
              ("Motor y\nbuzzer", 25)]
    fig, ax = plt.subplots(figsize=(3.45, 1.45))
    izq = 0
    colores = [AZUL, NARANJA, VERDE, GRIS]
    for (nombre, ma), c in zip(partes, colores):
        ax.barh(0, ma, left=izq, color=c, height=0.5, label=nombre.replace("\n", " "))
        ax.text(izq + ma / 2, 0, f"{ma}", ha="center", va="center", color="white", fontsize=7)
        izq += ma
    ax.legend(loc="lower center", bbox_to_anchor=(0.5, 0.98), ncol=2, frameon=False, fontsize=6.5,
              handlelength=1.2, columnspacing=1.0)
    ax.set_xlim(0, 175)
    ax.set_ylim(-0.4, 0.4)
    ax.set_yticks([])
    ax.set_xlabel("Corriente promedio estimada (mA), total 170 mA")
    ax.spines["left"].set_visible(False)
    ax.grid(axis="y", visible=False)
    fig.savefig(os.path.join(SALIDA, "resultado_consumo.pdf"))
    plt.close(fig)


if __name__ == "__main__":
    preparar_fotos()
    ev = figura_caida()
    figura_vibracion()
    figura_consumo()
    print("eventos (ms):", ev)
    print("listo")
