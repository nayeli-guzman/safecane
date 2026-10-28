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
