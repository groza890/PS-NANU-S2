#pragma once
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 Dashboard</title>
  <script>
    async function fetchTemp() {
  const r = await fetch('/get_temperature');
  if (!r.ok) throw new Error(`HTTP ${r.status}`);
  const j = await r.json();
  document.getElementById('temp').innerText = j.temperature.toFixed(2) + ' °C';
}

    async function toggleLED() {
      let btn = document.getElementById('btnLed');
      let state = btn.innerText === 'ON' ? 'OFF' : 'ON';
      await fetch('/led_control', {
        method: 'POST',
        headers: {'Content-Type':'application/x-www-form-urlencoded'},
        body: 'state=' + state 
      });
      btn.innerText = state;
    }
    async function loadMessages() {
      let r = await fetch('/messages');
      let arr = await r.json();
      let list = document.getElementById('msgs');
      list.innerHTML = arr.map(m => '<li>' + m + '</li>').join('');
    }
    async function loadFloods() {
      let r = await fetch('/floods');
      let arr = await r.json();
      let list = document.getElementById('floods');
      list.innerHTML = arr.map((f,i) =>
        `<li>${f} <button onclick="delFlood(${i})">X</button></li>`
      ).join('');
    }
    async function delFlood(i) {
      await fetch('/floods/delete?idx=' + i, {method:'DELETE'});
      loadFloods();
    }
    setInterval(fetchTemp, 2000);
    setInterval(loadFloods, 5000);
    window.onload = () => {
      fetchTemp();
      loadMessages();
      loadFloods();
    };
  </script>
</head>
<body>
  <h1>ESP32 Dashboard</h1>
  <p>Temperatura: <span id="temp">–</span></p>
  <button id="btnLed" onclick="toggleLED()">ON</button>
  <h2>Mesaje</h2>
  <ul id="msgs"></ul>
  <h2>Flood Alerts</h2>
  <ul id="floods"></ul>
</body>
</html>
)rawliteral";
