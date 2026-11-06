// Datos de ejemplo para ver el tablero sin bastón: tablero.html?demo
// Pasan por el mismo recibir() que los mensajes de MQTT. No se conecta a ningún broker.
// Se puede borrar este archivo (y su <script> en tablero.html) sin afectar al tablero real.
(() => {
  const params = new URLSearchParams(location.search);
  if (!params.has("demo")) return;

  const { recibir, ponerConexion } = window.SafeCane;
  const T = "safecane/ejemplo";
  const POS = { lat: -12.13525, lon: -77.02215, sats: 8 };
  const DISTANCIAS = [240, 170, 130, 95, 70, 45, 30, 55, 110, 190, -1];
  const est = { estado: "NORMAL", dist: 240, desnivel: false, uso: true, vbat: 3.92 };
  let i = 0, tCaida = 0;

  const enviar = (sub, d, ret = false) => recibir(`${T}/${sub}`, JSON.stringify(d), ret);
  const publicar = () => enviar("estado", est);

  const acciones = {
    "Desnivel"() {
      est.desnivel = true; publicar();
      setTimeout(() => { est.desnivel = false; publicar(); }, 1500);
    },
    "Caída"() {
      clearTimeout(tCaida);
      est.estado = "PRE_ALERTA"; publicar();
      tCaida = setTimeout(() => {
        est.estado = "ALERTA"; publicar();
        enviar("alerta", { tipo: "caida", ...POS });
      }, 4000);   // el bastón real espera 10 s
    },
    "Cancelar"() {
      clearTimeout(tCaida);
      const habiaAlerta = est.estado === "ALERTA";
      est.estado = "NORMAL"; publicar();
      if (habiaAlerta) enviar("alerta", { tipo: "cancelada", ...POS });
    },
    "Reposo"() { clearTimeout(tCaida); est.estado = "REPOSO"; est.uso = false; publicar(); },
    "Normal"() { clearTimeout(tCaida); est.estado = "NORMAL"; est.uso = true; publicar(); },
  };

  const caja = document.getElementById("accionesDemo");
  for (const nombre of Object.keys(acciones)) {
    const b = document.createElement("button");
    b.className = "boton"; b.type = "button"; b.textContent = nombre;
    b.onclick = acciones[nombre];
    caja.append(b);
  }
  const salir = document.createElement("a");
  salir.className = "boton"; salir.textContent = "Salir"; salir.href = location.pathname;
  caja.append(salir);

  document.getElementById("franjaDemo").hidden = false;
  document.getElementById("topicoVisible").textContent = T;
  document.getElementById("enlaceDemo").hidden = true;
  ponerConexion("Datos de ejemplo", "warn");

  enviar("gps", { ...POS, hdop: 0.9 });
  publicar();
  setInterval(() => { est.dist = DISTANCIAS[++i % DISTANCIAS.length]; publicar(); }, 2500);

  // ?demo=Caída, ?demo=Reposo, etc. arrancan directamente en esa situación
  const inicial = params.get("demo");
  if (acciones[inicial]) acciones[inicial]();
})();
