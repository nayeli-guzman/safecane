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
