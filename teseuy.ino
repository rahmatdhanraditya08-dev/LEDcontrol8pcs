#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Radit";
const char* password = "rahmatgaming";

WebServer server(80);

const int ledPins[] = {13, 12, 14, 27, 26, 25, 33, 32};
const int numLeds = 8;

String currentMode = "manual";
unsigned long lastAutoRun = 0;
int flipFlopState = 0;
int sequenceIndex = 0;
int speedDelay = 200;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>LED KONTROL | ESP32</title>
  <link href="https://fonts.googleapis.com/css2?family=Inter:opsz,wght@14..32,300;14..32,400;14..32,600;14..32,700;14..32,800&display=swap" rel="stylesheet">
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      font-family: 'Inter', sans-serif;
      background: radial-gradient(circle at 20% 30%, #0a0f1e, #03060c);
      min-height: 100vh;
      padding: 24px;
      color: #f0f3fa;
    }

    /* kontainer utama */
    .dashboard {
      max-width: 1300px;
      margin: 0 auto;
      backdrop-filter: blur(2px);
    }

    /* header */
    .header {
      text-align: center;
      margin-bottom: 40px;
      animation: fadeDown 0.8s ease;
    }
    .header h1 {
      font-size: 3.2rem;
      font-weight: 800;
      background: linear-gradient(135deg, #FFD966, #FF8C42, #FF3D6C);
      -webkit-background-clip: text;
      background-clip: text;
      color: transparent;
      letter-spacing: 2px;
      text-shadow: 0 0 15px rgba(255,140,66,0.3);
    }
    .header p {
      color: #9aaec5;
      font-weight: 400;
      margin-top: 8px;
      font-size: 1rem;
    }

    /* grid LED */
    .led-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(110px, 1fr));
      gap: 24px;
      margin-bottom: 48px;
    }
    .led-card {
      background: rgba(20, 30, 45, 0.55);
      backdrop-filter: blur(12px);
      border-radius: 32px;
      padding: 20px 12px;
      text-align: center;
      border: 1px solid rgba(255,255,255,0.15);
      transition: all 0.3s cubic-bezier(0.2, 0.9, 0.4, 1.1);
      box-shadow: 0 10px 20px -5px rgba(0,0,0,0.4);
    }
    .led-card:hover {
      transform: translateY(-8px);
      border-color: rgba(255,200,100,0.5);
      box-shadow: 0 20px 30px -8px rgba(0,0,0,0.6);
    }
    .led-label {
      font-weight: 700;
      font-size: 1rem;
      background: linear-gradient(120deg, #ffffff, #b9c8ff);
      -webkit-background-clip: text;
      background-clip: text;
      color: transparent;
      margin-bottom: 12px;
    }
    .led-indicator {
      width: 70px;
      height: 70px;
      margin: 12px auto;
      border-radius: 50%;
      background: #1e2a3a;
      box-shadow: inset 0 2px 5px rgba(0,0,0,0.5), 0 0 0 2px rgba(255,255,255,0.05);
      transition: all 0.2s ease;
    }
    .led-indicator.on {
      background: #ffbc3c;
      box-shadow: 0 0 25px #ffaa33, inset 0 0 8px #fff2c4;
      animation: pulseGlow 1.2s infinite;
    }
    @keyframes pulseGlow {
      0% { box-shadow: 0 0 5px #ffaa33; }
      50% { box-shadow: 0 0 25px #ffdd77; }
      100% { box-shadow: 0 0 5px #ffaa33; }
    }
    .toggle-btn {
      background: linear-gradient(145deg, #2c3e50, #1e2a3a);
      border: none;
      color: white;
      font-weight: 600;
      padding: 8px 18px;
      border-radius: 40px;
      font-size: 0.8rem;
      cursor: pointer;
      transition: 0.2s;
      box-shadow: 0 3px 6px rgba(0,0,0,0.3);
      width: 80%;
    }
    .toggle-btn:active {
      transform: scale(0.95);
    }

    /* tombol all */
    .action-bar {
      display: flex;
      justify-content: center;
      gap: 28px;
      margin: 20px 0 40px;
    }
    .action-btn {
      background: linear-gradient(145deg, #212d3f, #131c28);
      border: 1px solid rgba(255,215,120,0.4);
      padding: 14px 36px;
      border-radius: 60px;
      font-weight: 700;
      font-size: 1.1rem;
      color: #f5f9ff;
      cursor: pointer;
      transition: 0.2s;
      backdrop-filter: blur(8px);
      letter-spacing: 1px;
    }
    .action-btn.on {
      background: linear-gradient(145deg, #2b6e3c, #1f5530);
      box-shadow: 0 0 15px #2ecc71;
    }
    .action-btn.off {
      background: linear-gradient(145deg, #8b2c2c, #6b1f1f);
    }
    .action-btn:active {
      transform: scale(0.97);
    }

    /* mode panel */
    .mode-panel {
      background: rgba(0, 0, 0, 0.4);
      border-radius: 48px;
      padding: 24px 20px;
      margin: 30px 0;
      backdrop-filter: blur(8px);
      border: 1px solid rgba(255,255,255,0.1);
    }
    .mode-title {
      font-size: 1.3rem;
      font-weight: 600;
      margin-bottom: 20px;
      display: flex;
      align-items: center;
      gap: 10px;
      justify-content: center;
    }
    .mode-buttons {
      display: flex;
      flex-wrap: wrap;
      gap: 16px;
      justify-content: center;
    }
    .mode-btn {
      background: #1a2538;
      border: none;
      padding: 10px 28px;
      border-radius: 50px;
      font-weight: 600;
      color: #ccdeee;
      font-size: 0.9rem;
      cursor: pointer;
      transition: all 0.2s;
      box-shadow: 0 3px 6px black;
    }
    .mode-btn.active {
      background: linear-gradient(135deg, #f39c12, #e67e22);
      color: white;
      box-shadow: 0 0 12px #f39c12;
      border: 1px solid #ffd966;
    }

    /* speed slider */
    .speed-box {
      background: rgba(0,0,0,0.3);
      border-radius: 60px;
      padding: 16px 24px;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 20px;
      flex-wrap: wrap;
      margin-top: 20px;
    }
    .speed-box label {
      font-weight: 600;
    }
    input[type="range"] {
      width: 260px;
      height: 5px;
      -webkit-appearance: none;
      background: #2c3e66;
      border-radius: 5px;
      outline: none;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 20px;
      height: 20px;
      background: #ffb347;
      border-radius: 50%;
      cursor: pointer;
      box-shadow: 0 0 8px orange;
    }
    #speedValue {
      background: #0a0f1a;
      padding: 5px 18px;
      border-radius: 40px;
      font-family: monospace;
      font-weight: bold;
      font-size: 1rem;
    }
    footer {
      text-align: center;
      margin-top: 45px;
      font-size: 0.75rem;
      color: #5a6e8a;
    }

    @keyframes fadeDown {
      from {
        opacity: 0;
        transform: translateY(-30px);
      }
      to {
        opacity: 1;
        transform: translateY(0);
      }
    }
    @media (max-width: 700px) {
      .header h1 { font-size: 2rem; }
      .led-indicator { width: 55px; height: 55px; }
      .action-btn { padding: 8px 20px; font-size: 0.9rem; }
    }
  </style>
</head>
<body>
<div class="dashboard">
  <div class="header">
    <h1>🔮 LED KONTROL</h1>
    <p>✨ kendali lampu 8 channel | mode spektakuler | realtime ✨</p>
  </div>

  <div class="led-grid" id="ledGrid"></div>

  <div class="action-bar">
    <button class="action-btn on" id="allOnBtn">💡 ALL ON</button>
    <button class="action-btn off" id="allOffBtn">🌑 ALL OFF</button>
  </div>

  <div class="mode-panel">
    <div class="mode-title">🎛️ MODE CERDAS</div>
    <div class="mode-buttons">
      <button class="mode-btn" data-mode="manual">🖐️ MANUAL</button>
      <button class="mode-btn" data-mode="flipflop">🔄 FLIP FLOP</button>
      <button class="mode-btn" data-mode="sequence">🎬 SEQUENCE</button>
      <button class="mode-btn" data-mode="random">✨ RANDOM</button>
      <button class="mode-btn" data-mode="wave">🌊 WAVE</button>
    </div>
    <div class="speed-box">
      <label>⚡ KECEPATAN</label>
      <input type="range" id="speedSlider" min="50" max="600" value="200" step="10">
      <span id="speedValue">200 ms</span>
    </div>
  </div>
  <footer>Klik tombol TOGGLE pada setiap LED | mode otomatis akan berjalan di background</footer>
</div>

<script>
  async function fetchStatus() {
    const res = await fetch('/status');
    const data = await res.json();
    updateLeds(data.states);
    updateActiveMode(data.mode);
  }

  function updateLeds(states) {
    const grid = document.getElementById('ledGrid');
    grid.innerHTML = '';
    for (let i = 0; i < states.length; i++) {
      const card = document.createElement('div');
      card.className = 'led-card';
      card.innerHTML = `
        <div class="led-label">LED ${i}</div>
        <div class="led-indicator ${states[i] ? 'on' : ''}"></div>
        <button class="toggle-btn" data-idx="${i}">⚡ TOGGLE</button>
      `;
      grid.appendChild(card);
    }
    document.querySelectorAll('.toggle-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        const idx = btn.getAttribute('data-idx');
        toggleLed(idx);
      });
    });
  }

  function updateActiveMode(mode) {
    document.querySelectorAll('.mode-btn').forEach(btn => {
      if (btn.dataset.mode === mode) btn.classList.add('active');
      else btn.classList.remove('active');
    });
  }

  async function toggleLed(idx) {
    await fetch(`/toggle?index=${idx}`);
    fetchStatus();
  }

  async function allOn() {
    await fetch('/all/on');
    fetchStatus();
  }

  async function allOff() {
    await fetch('/all/off');
    fetchStatus();
  }

  async function setMode(mode) {
    await fetch(`/mode?name=${mode}`);
    fetchStatus();
  }

  async function setSpeed(val) {
    await fetch(`/speed?value=${val}`);
  }

  document.getElementById('allOnBtn').onclick = allOn;
  document.getElementById('allOffBtn').onclick = allOff;
  document.querySelectorAll('.mode-btn').forEach(btn => {
    btn.onclick = () => setMode(btn.dataset.mode);
  });
  const slider = document.getElementById('speedSlider');
  const speedSpan = document.getElementById('speedValue');
  slider.oninput = function() {
    let val = this.value;
    speedSpan.innerText = val + ' ms';
    setSpeed(val);
  };

  setInterval(fetchStatus, 800);
  fetchStatus();
</script>
</body>
</html>
)rawliteral";

void setAllLeds(uint8_t mask) {
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], (mask >> i) & 1);
  }
}

void runAutoMode() {
  if (currentMode == "manual") return;
  unsigned long now = millis();
  if (now - lastAutoRun < speedDelay) return;
  lastAutoRun = now;

  if (currentMode == "flipflop") {
    uint8_t pattern = flipFlopState ? 0b11110000 : 0b00001111;
    setAllLeds(pattern);
    flipFlopState = 1 - flipFlopState;
  } else if (currentMode == "sequence") {
    setAllLeds(1 << sequenceIndex);
    sequenceIndex = (sequenceIndex + 1) % numLeds;
  } else if (currentMode == "random") {
    setAllLeds(random(256));
  } else if (currentMode == "wave") {
    uint8_t mask = (1 << sequenceIndex) | (1 << ((sequenceIndex + 1) % numLeds));
    setAllLeds(mask);
    sequenceIndex = (sequenceIndex + 1) % numLeds;
  }
}

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleStatus() {
  String states = "[";
  for (int i = 0; i < numLeds; i++) {
    states += digitalRead(ledPins[i]) ? "1" : "0";
    if (i < numLeds - 1) states += ",";
  }
  states += "]";
  String json = "{\"states\":" + states + ",\"mode\":\"" + currentMode + "\"}";
  server.send(200, "application/json", json);
}

void handleToggle() {
  if (server.hasArg("index")) {
    int idx = server.arg("index").toInt();
    if (idx >= 0 && idx < numLeds) {
      digitalWrite(ledPins[idx], !digitalRead(ledPins[idx]));
      if (currentMode != "manual") currentMode = "manual";
      server.send(200, "text/plain", "OK");
      return;
    }
  }
  server.send(400, "text/plain", "Bad Request");
}

void handleAllOn() {
  for (int i = 0; i < numLeds; i++) digitalWrite(ledPins[i], HIGH);
  currentMode = "manual";
  server.send(200, "text/plain", "OK");
}

void handleAllOff() {
  for (int i = 0; i < numLeds; i++) digitalWrite(ledPins[i], LOW);
  currentMode = "manual";
  server.send(200, "text/plain", "OK");
}

void handleMode() {
  if (server.hasArg("name")) {
    String mode = server.arg("name");
    if (mode == "manual") currentMode = "manual";
    else if (mode == "flipflop") { currentMode = "flipflop"; flipFlopState = 0; }
    else if (mode == "sequence") { currentMode = "sequence"; sequenceIndex = 0; }
    else if (mode == "random") { currentMode = "random"; }
    else if (mode == "wave") { currentMode = "wave"; sequenceIndex = 0; }
    server.send(200, "text/plain", "OK");
    return;
  }
  server.send(400, "text/plain", "Bad Request");
}

void handleSpeed() {
  if (server.hasArg("value")) {
    int val = server.arg("value").toInt();
    if (val >= 20 && val <= 1000) speedDelay = val;
    server.send(200, "text/plain", "OK");
    return;
  }
  server.send(400, "text/plain", "Bad Request");
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/toggle", handleToggle);
  server.on("/all/on", handleAllOn);
  server.on("/all/off", handleAllOff);
  server.on("/mode", handleMode);
  server.on("/speed", handleSpeed);
  server.begin();
  randomSeed(analogRead(0));
}

void loop() {
  server.handleClient();
  runAutoMode();
}