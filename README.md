# safecane-

Bastón inteligente con ESP32. La guía completa está en [guia_safecane_infrarrojo.md](guia_safecane_infrarrojo.md).

## Modelo 3D: ejecución rápida

Todo se corre desde la raíz del repo. Requiere Python 3.12 y navegador con WebGL.

```bash
# 1. Una sola vez: crear el entorno e instalar CadQuery
python3 -m venv .venv-cad && .venv-cad/bin/pip install cadquery

# 2. Generar el modelo (escribe modelo3d/salida/*.stl, .step y .glb)
.venv-cad/bin/python modelo3d/safecane.py

# 3. Copiar el .glb al visor (hacerlo tras cada regeneración)
cp modelo3d/salida/safecane.glb modelo3d/visor/

# 4. Levantar el visor en segundo plano y abrirlo
cd modelo3d/visor && (nohup python3 -m http.server 8000 >/dev/null 2>&1 &)
```

Abrir en el navegador:

- Bastón: http://localhost:8000/
- Protoboard con las piezas reales: http://localhost:8000/protoboard.html

Parar el servidor sin matar la terminal:

```bash
kill $(ss -ltnp | grep ':8000' | grep -o 'pid=[0-9]*' | cut -d= -f2)
```

### Estructura

| Ruta | Qué es |
| --- | --- |
| `modelo3d/safecane.py` | Script CadQuery paramétrico. Las medidas están al inicio (`TUBO_D`, `MANGO_D`, `MANGO_L`, `VENTANA_IR`). |
| `modelo3d/salida/` | Salida generada: `mango_a.stl`, `mango_b.stl`, `contera.stl` (imprimir), `safecane.step` (FreeCAD) y `safecane.glb` (visor). |
| `modelo3d/visor/index.html` | Visor del bastón con panel de sensores, zoom al hacer clic y carcasa transparente. |
| `modelo3d/visor/protoboard.html` | Vista tipo protoboard con cables de colores y burbujas al pasar el mouse. |
| `modelo3d/visor/lib/` | three.js 0.160.0 copiado localmente (no necesita internet). |

### Problemas conocidos

- **`Address already in use` en el puerto 8000:** el servidor ya está corriendo. Abre la URL directamente.
- **No usar `pkill -f "http.server 8000"`:** también coincide con el propio comando de la terminal y la congela. Usa el `kill` de arriba.
- **Error `Error creating WebGL context`:** el navegador no tiene WebGL. Probar otro navegador o:
  `google-chrome --use-angle=swiftshader --enable-unsafe-swiftshader http://localhost:8000/`
- **El visor no muestra los cambios:** falta copiar el `.glb` (paso 3) y recargar con `Ctrl+F5`.
- **Abrir `index.html` con doble clic no funciona:** el navegador bloquea la carga del `.glb` desde `file://`. Siempre usar el servidor.
- **Nombres de piezas:** el visor busca los nodos del `.glb` por nombre (`esp32`, `hcsr04`, `mango_a`, ...). Si renombras una pieza en `safecane.py`, actualiza `INFO` en `visor/index.html`.
