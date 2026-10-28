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
