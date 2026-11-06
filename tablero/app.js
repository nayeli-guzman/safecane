// SafeCane: tablero. Recibe por MQTT lo que publica firmware/safecane/safecane.ino.
const UTEC = [-12.1352, -77.0222];
const SEG_SOSPECHOSO = 15;   // el bastón publica cada 5 s: más de 15 s sin datos es sospechoso
const TOPICO_DEFECTO = "safecane/grupo6";

const ESTADOS = {
  NORMAL:     { nombre: "Normal",     texto: "Avisa de obstáculos y desniveles.", c: "--ok" },
  PRE_ALERTA: { nombre: "Pre-alerta", texto: "Posible caída. El usuario tiene 10 s para cancelar con el pulsador.", c: "--warn" },
  ALERTA:     { nombre: "Alerta",     texto: "Caída confirmada. Suenan el buzzer y el LED.", c: "--bad" },
  REPOSO:     { nombre: "Reposo",     texto: "Bastón quieto. Los actuadores están apagados.", c: "--idle" },
};
// Mismos umbrales y patrones que tareaActuadores() en el firmware
const VIBRACION = {
  continua: "continua",
  rapida: "pulsos rápidos",
  lenta: "pulsos lentos",
  alerta: "patrón de pre-alerta",
  ninguna: "ninguna",
};

const $ = id => document.getElementById(id);
const body = document.body;
const reducirMovimiento = matchMedia("(prefers-reduced-motion: reduce)");
const hora = () => new Date().toLocaleTimeString();

// ---------- Preferencias (solo en este navegador) ----------
function leerPref() {
  try { return JSON.parse(localStorage.getItem("safecane") || "{}"); } catch { return {}; }
}
function guardarPref(p) {
  try { localStorage.setItem("safecane", JSON.stringify(p)); } catch {}
}
const pref = leerPref();
$("topico").value = pref.topico || TOPICO_DEFECTO;
$("broker").value = pref.broker || "wss://broker.hivemq.com:8884/mqtt";
$("url").value = pref.url || "";
$("usuario").value = pref.usuario || "";
$("broker").onchange = () => $("form").classList.toggle("personalizado", $("broker").value === "otro");
$("broker").onchange();

// ---------- Panel de conexión ----------
const panel = $("panelConexion"), abrir = $("abrirConexion");
function panelAbierto(si, instantaneo) {
  panel.classList.toggle("instantaneo", !!instantaneo);
  panel.classList.toggle("abierto", si);
  abrir.setAttribute("aria-expanded", si);
  if (si) $("topico").focus({ preventScroll: true });
}
abrir.onclick = e => { e.stopPropagation(); panelAbierto(!panel.classList.contains("abierto")); };
panel.onclick = e => e.stopPropagation();
document.addEventListener("click", () => panelAbierto(false));
document.addEventListener("keydown", e => {
  if (e.key !== "Escape") return;
  if ($("alerta").classList.contains("visible")) cerrarAlerta();
  else if (panel.classList.contains("abierto")) { panelAbierto(false, true); abrir.focus(); }
});

// ---------- Mapa ----------
let mapa = null, punto = null;
if (window.L) {
  mapa = L.map("mapa").setView(UTEC, 16);
  L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
    maxZoom: 19, attribution: "© OpenStreetMap"
  }).addTo(mapa);
  addEventListener("resize", () => mapa.invalidateSize());
} else {
  $("mapa").textContent = "No se pudo cargar el mapa. Revisa la conexión a internet.";
}

function ponerPunto(lat, lon, caida) {
  if (!lat && !lon) return false;                // sin fix: el firmware manda 0, 0
  if (!mapa) return true;
  const icono = L.divIcon({
    className: "", iconSize: [18, 18], iconAnchor: [9, 9],
    html: `<div class="pin${caida ? " caida" : ""}"><i></i></div>`,
  });
  if (!punto) punto = L.marker([lat, lon], { icon: icono, keyboard: false }).addTo(mapa);
  punto.setLatLng([lat, lon]).setIcon(icono);
  punto.bindTooltip(caida ? "SafeCane: CAÍDA" : "SafeCane");
  mapa.setView([lat, lon], Math.max(mapa.getZoom(), 17), { animate: !reducirMovimiento.matches });
  return true;
}
const coords = (lat, lon) => `${lat.toFixed(5)}, ${lon.toFixed(5)}`;

// ---------- Sonido de alarma ----------
let audio = null, sirena = null;
document.addEventListener("click", () => {
  try {
    audio = audio || new (window.AudioContext || window.webkitAudioContext)();
    audio.resume();
    $("sonido").hidden = true;
  } catch {}
}, { once: true });

function pitido() {
  if (!audio) return;
  const o = audio.createOscillator(), g = audio.createGain();
  o.frequency.value = 880; g.gain.value = .25;
  o.connect(g).connect(audio.destination);
  o.start(); o.stop(audio.currentTime + .35);
}
function sonar(si) {
  clearInterval(sirena); sirena = null;
  if (si) { pitido(); sirena = setInterval(pitido, 700); }
}

// ---------- Avisos ----------
function avisar(texto) {
  const a = $("aviso");
  a.textContent = texto;
  a.classList.add("visible");
  clearTimeout(avisar.t);
  avisar.t = setTimeout(() => a.classList.remove("visible"), 5000);
}

// ---------- Actividad y mensajes ----------
function agregarEvento(texto, color) {
  const lista = $("actividad");
  lista.querySelector(".vacio")?.remove();
  const li = document.createElement("li");
  const punto = document.createElement("i");
  punto.style.setProperty("--c", `var(${color})`);
  const t = document.createElement("span"); t.textContent = texto;
  const h = document.createElement("time"); h.textContent = hora();
  li.append(punto, t, h);
  lista.prepend(li);
  while (lista.children.length > 30) lista.lastChild.remove();
}

function registrar(topico, texto, retenido) {
  const log = $("log");
  log.querySelector(".vacio")?.remove();
  const fila = document.createElement("div");
  const cab = document.createElement("span");
  cab.textContent = `${hora()} ${topico}${retenido ? " (retenido)" : ""}`;
  fila.append(cab, texto);
  log.prepend(fila);
  while (log.children.length > 40) log.lastChild.remove();
}

// ---------- Estado ----------
let ultimoDato = 0, estadoActual = null, desnivelActual = false;

function cambiarPalabra(texto) {
  const e = $("estado");
  if (e.textContent === texto) return;
  e.textContent = texto;
  if (!reducirMovimiento.matches && e.animate) {
    e.animate(
      [{ filter: "blur(4px)", opacity: .3 }, { filter: "blur(0)", opacity: 1 }],
      { duration: 240, easing: "cubic-bezier(0.23, 1, 0.32, 1)" }
    );
  }
}

function zonaDe(dist) {
  if (dist == null || dist <= 0) return { vibra: "ninguna", zona: 0, texto: "Sin eco" };
  if (dist < 50)  return { vibra: "continua", zona: 1, texto: "Menos de 50 cm" };
  if (dist < 100) return { vibra: "rapida",   zona: 2, texto: "Entre 50 y 100 cm" };
  if (dist < 200) return { vibra: "lenta",    zona: 3, texto: "Entre 100 y 200 cm" };
  return { vibra: "ninguna", zona: 0, texto: "Más de 2 m: sin aviso" };
}

function dibujarDistancia(dist, zona) {
  const CM_A_X = 1.7, ORIGEN = 175, MAX_CM = 250;
  const hay = dist != null && dist > 0 && dist <= MAX_CM;
  const cm = hay ? dist : MAX_CM;
  const x = ORIGEN + cm * CM_A_X;
  const obst = $("obstaculo");
  obst.style.transform = `translateX(${x}px)`;
  obst.classList.toggle("visible", hay);
  $("obstaculoTexto").textContent = hay ? `${Math.round(dist)} cm` : "";
  $("ondas").style.setProperty("--alcance", `${x - 172}px`);
  document.querySelectorAll(".zona").forEach((z, i) => z.classList.toggle("activa", i + 1 === zona));
}

function mostrarEstado(d) {
  ultimoDato = Date.now();
  body.dataset.fresco = "si";
  reiniciarFrescura();

  const info = ESTADOS[d.estado];
  if (d.estado !== estadoActual) {
    if (estadoActual || info) agregarEvento(info ? info.nombre : String(d.estado), info ? info.c : "--idle");
    estadoActual = d.estado;
  }
  body.dataset.estado = info ? d.estado : "";
  cambiarPalabra(info ? info.nombre : String(d.estado ?? "—"));
  $("descripcion").textContent = info ? info.texto : "Estado desconocido.";

  const z = zonaDe(d.dist);
  const normal = d.estado === "NORMAL";
  const desnivel = !!d.desnivel;
  let vibra = "ninguna";
  if (d.estado === "PRE_ALERTA") vibra = "alerta";
  else if (normal) vibra = desnivel ? "rapida" : z.vibra;
  body.dataset.vibra = vibra;
  body.dataset.desnivel = desnivel ? "si" : "no";
  dibujarDistancia(d.dist, normal ? z.zona : 0);

  $("leyenda").textContent = desnivel && normal
    ? "Desnivel delante: motor y buzzer pulsan juntos"
    : d.estado === "ALERTA"
      ? "Vibración: apagada. Buzzer y LED intermitentes"
      : `Vibración: ${VIBRACION[vibra]}`;

  $("dist").textContent = d.dist == null || d.dist < 0 ? "Sin lectura" : `${Math.round(d.dist)} cm`;
  $("zona").textContent = z.texto;
  $("desnivel").textContent = desnivel ? "¡Delante!" : "No";
  $("datoDesnivel").dataset.tono = desnivel ? "warn" : "";
  if (desnivel && !desnivelActual) agregarEvento("Desnivel detectado", "--warn");
  desnivelActual = desnivel;
  $("uso").textContent = d.uso ? "En uso" : "Quieto";

  const hayBateria = d.vbat > 0;
  $("vbat").textContent = hayBateria ? `${d.vbat.toFixed(2)} V` : "Sin medir";
  $("datoBateria").dataset.tono = hayBateria && d.vbat < 3.4 ? "bad" : "";
  // 3,3 V es el corte del firmware y 4,2 V la celda llena
  const nivel = hayBateria ? Math.min(1, Math.max(0, (d.vbat - 3.3) / .9)) : 0;
  $("celda").style.transform = `scaleX(${nivel})`;
}

function reiniciarFrescura() {
  const f = $("frescura");
  if (reducirMovimiento.matches || !f.animate) return;
  f.getAnimations().forEach(a => a.cancel());
  f.animate([{ transform: "scaleX(1)" }, { transform: "scaleX(0)" }],
            { duration: SEG_SOSPECHOSO * 1000, easing: "linear", fill: "forwards" });
}

// ---------- Alerta ----------
let alertaDesde = 0, focoAntes = null;

function abrirAlerta() {
  const a = $("alerta");
  if (!a.classList.contains("visible")) focoAntes = document.activeElement;
  a.classList.add("visible");
  $("entendido").focus({ preventScroll: true });
}
function cerrarAlerta() {
  $("alerta").classList.remove("visible");
  sonar(false);
  alertaDesde = 0;
  if (focoAntes?.focus) focoAntes.focus({ preventScroll: true });
  focoAntes = null;
}
$("entendido").onclick = cerrarAlerta;

function mostrarAlerta(d, retenido) {
  const hayGps = ponerPunto(d.lat, d.lon, d.tipo === "caida");
  const lugar = hayGps ? coords(d.lat, d.lon) : "";
  const donde = hayGps ? `en ${lugar} (${d.sats} satélites)` : "sin ubicación GPS";
  if (d.tipo === "caida") {
    $("alertaDonde").textContent = hayGps ? `Posición ${lugar}, con ${d.sats} satélites.` : "Sin ubicación GPS.";
    const enlace = $("alertaMapa");
    enlace.hidden = !hayGps;
    if (hayGps) enlace.href = `https://www.google.com/maps?q=${d.lat},${d.lon}`;
    alertaDesde = retenido ? 0 : Date.now();
    actualizarTiempoAlerta(retenido);
    abrirAlerta();
    sonar(true);
    agregarEvento(`Caída ${donde}`, "--bad");
  } else if (d.tipo === "cancelada") {
    cerrarAlerta();
    if (!retenido) {
      avisar("Alerta cancelada por el usuario");
      agregarEvento("Alerta cancelada por el usuario", "--ok");
    }
  }
}
function actualizarTiempoAlerta(retenido) {
  if (retenido) { $("alertaHora").textContent = "Alerta activa recibida al conectar."; return; }
  if (!alertaDesde) return;
  const s = Math.round((Date.now() - alertaDesde) / 1000);
  $("alertaHora").textContent = s < 2 ? "Ahora mismo" : `Hace ${s} s`;
}

function mostrarGps(d) {
  const enCaida = $("alerta").classList.contains("visible") || estadoActual === "ALERTA";
  if (ponerPunto(d.lat, d.lon, enCaida))
    $("gpsTexto").textContent = `${coords(d.lat, d.lon)} · ${d.sats} satélites · hdop ${d.hdop}`;
}

// ---------- Mensajes recibidos ----------
function recibir(topico, texto, retenido) {
  registrar(topico, texto, retenido);
  let d;
  try { d = JSON.parse(texto); } catch { return; }
  if (topico.endsWith("/estado")) mostrarEstado(d);
  else if (topico.endsWith("/alerta")) mostrarAlerta(d, retenido);
  else if (topico.endsWith("/gps")) mostrarGps(d);
}

// ---------- MQTT por WebSocket ----------
let cliente = null;

function ponerConexion(texto, tono) {
  $("conexion").textContent = texto;
  $("conexion").dataset.tono = tono;
}

function conectar() {
  const topico = $("topico").value.trim().replace(/\/+$/, "");
  const broker = $("broker").value;
  const url = broker === "otro" ? $("url").value.trim() : broker;
  guardarPref({ topico, broker, url: $("url").value.trim(), usuario: $("usuario").value.trim() });
  if (!topico || !url) return avisar("Falta el tópico o la URL del broker");
  $("topicoVisible").textContent = topico;
  if (!window.mqtt) {
    ponerConexion("Sin librería MQTT", "bad");
    return avisar("No se pudo cargar la librería MQTT. Revisa la conexión a internet.");
  }

  if (cliente) cliente.end(true);
  ponerConexion("Conectando…", "warn");
  const opciones = { clientId: "tablero-" + Math.random().toString(16).slice(2, 10), reconnectPeriod: 3000, connectTimeout: 8000 };
  if (broker === "otro" && $("usuario").value.trim()) {
    opciones.username = $("usuario").value.trim();
    opciones.password = $("clave").value;
  }
  cliente = mqtt.connect(url, opciones);

  cliente.on("connect", () => {
    ponerConexion("Conectado", "ok");
    cliente.subscribe(`${topico}/#`);
  });
  cliente.on("reconnect", () => ponerConexion("Reconectando…", "warn"));
  cliente.on("offline", () => ponerConexion("Desconectado", "bad"));
  cliente.on("error", err => { ponerConexion("Error", "bad"); avisar("Error MQTT: " + err.message); });
  cliente.on("message", (t, buf, paquete) => recibir(t, buf.toString(), paquete.retain));
}

$("form").onsubmit = e => { e.preventDefault(); panelAbierto(false); conectar(); };

// ---------- Cada segundo ----------
setInterval(() => {
  if (alertaDesde) actualizarTiempoAlerta(false);
  if (!ultimoDato) return;
  const s = Math.round((Date.now() - ultimoDato) / 1000);
  const fresco = s < SEG_SOSPECHOSO;
  const u = $("ultimo");
  u.textContent = fresco ? `Datos hace ${s} s` : `Sin datos hace ${s} s`;
  u.dataset.tono = fresco ? "ok" : "bad";
  body.dataset.fresco = fresco ? "si" : "no";
  if (!fresco) $("descripcion").textContent = "Sin datos recientes: esto es lo último que se recibió.";
}, 1000);

// Con ?demo no se conecta al broker: los datos los manda demo.js
window.SafeCane = { recibir, ponerConexion };
if (!new URLSearchParams(location.search).has("demo")) conectar();
