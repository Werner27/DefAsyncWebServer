
const char root_content[] PROGMEM = R"rawliteral(
    <div class="content">
        <h1>Willkommen auf meiner Hauptseite!</h1>
        <p>Dies ist die Startseite. Benutze das Menü oben, um zu den anderen Seiten zu navigieren.</p>
    </div>
)rawliteral";

const char submenu01_content[] PROGMEM = R"rawliteral(
    <div class="content">
        <h2>Dieser Block ist der Inhalt</h2>
        <p>Zählerstand: <span id="counterValue">%COUNTER_VALUE%</span></p>
        <button onclick="counterReset()">Reset Zähler</button>

        <p>Text für EEPROM speichern:</p>
        <input type="text" id="eepromText" value="%EEPROM_TEXT%">
        <button onclick="saveToEEPROM()">Speichern</button><br><br>
        <button onclick="window.history.back()">Zurück</button>
    </div>
    <script>
        function updateCounter() {
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("counterValue").innerHTML = this.responseText;
                }
            };
            xhr.open("GET", "/counter", true);
            xhr.send();
        }
        function counterReset() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/counterReset", true);
            xhr.send();
        }
        function saveToEEPROM() {
            var text = document.getElementById("eepromText").value;
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/save_eeprom", true);
            xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            xhr.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    alert("Text gespeichert!");
                }
            };
            xhr.send("data=" + text);
        }
        setInterval(updateCounter, 1000);
    </script>
)rawliteral";

const char status_content[] PROGMEM = R"rawliteral(
<div class="form-container">
  <h2>Status</h2>

  <fieldset class="block">
    <legend>Text auf EEPROM speichern</legend>
    <div class="form-group"><input type="text" id="eepromText" value="%EEPROM_TEXT%"></div>
    <div class="form-actions"><button class="btn01" onclick="saveToEEPROM()">Speichern</button></div>
  </fieldset>

  <fieldset class="block">
    <legend>Zählerstand</legend>
    <p>Zählerstand: <span id="counterValue">%COUNTER_VALUE%</span></p>
    <div class="form-actions"><button class="btn01" onclick="counterReset()">Reset Zähler</button></div>
  </fieldset>

  <fieldset class="block">
    <legend>Zählersteuerung</legend>
    <div class="form-group"><label>Set Counter:</label><input type="text" id="set_counter" value="%SET_COUNTER%"></div>
    <div class="form-group"><label>Aktueller Zählerstand:</label><input type="number" id="cur_counter" value="%CUR_COUNTER%" disabled></div>
    <div class="form-actions"><button class="btn01" onclick="setCounter()">Neu setzen</button></div>
  </fieldset>

  <fieldset class="block">
    <legend>JSON Status</legend>
    <div class="form-group"><label>Uptime (s):</label><input id="uptime" readonly></div>
    <div class="form-group"><label>Counter:</label><input id="counter" readonly></div>
    <div class="form-group"><label>Modus:</label><input id="mode" readonly></div>
    <div class="form-group"><label>SSID:</label><input id="ssid" readonly></div>
    <div class="form-group"><label>IP:</label><input id="ip" readonly></div>
    <div class="form-group"><label>Subnetz:</label><input id="subnet" readonly></div>
    <div class="form-group"><label>Free Heap:</label><input id="heap" readonly></div>
    <div class="form-actions"><button class="btn01" id="toggleBtn" onclick="toggleJson()">JSON anzeigen</button></div>
  </fieldset>
</div>

<script>
function updateCounter() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("counterValue").innerHTML = this.responseText;
            document.getElementById("cur_counter").value = this.responseText;
        }
    };
    xhr.open("GET", "/counter", true);
    xhr.send();
}
function counterReset() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/counter_reset", true);
    xhr.send();
}
function saveToEEPROM() {
    var text = document.getElementById("eepromText").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/save_eeprom", true);
    xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            alert("Text gespeichert!");
        }
    };
    xhr.send("data=" + text);
}
function setCounter() {
    var value = document.getElementById("set_counter").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/set_counter", true);
    xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
        }
    };
    xhr.send("data=" + value);
}

setInterval(updateCounter, 1000);

let intervalId = null;
function fetchStatus() {
  fetch('/status.json')
    .then(r => r.json())
    .then(d => {
      uptime.value = d.uptime_sec;
      counter.value = d.counter;
      mode.value = d.wifi_mode;
      ssid.value = d.wifi_ssid;
      ip.value = d.ip;
      subnet.value = d.subnet;
      heap.value = d.free_heap;
    });
}
function toggleJson() {
  const btn = document.getElementById('toggleBtn');
  if (intervalId) {
    clearInterval(intervalId); intervalId = null; btn.textContent = 'JSON anzeigen';
  } else {
    fetchStatus(); intervalId = setInterval(fetchStatus, 1000); btn.textContent = 'Stop';
  }
}
</script>
)rawliteral";
