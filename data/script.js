// WebSocket Client (für websocket.html)
(function(){
  try {
    const socket = new WebSocket((location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/ws');

    socket.onopen = () => console.log("WebSocket verbunden");

    socket.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        if (typeof data.uptime !== 'undefined') {
          const el = document.getElementById("uptime");
          if (el) el.innerText = data.uptime;
        }
        if (typeof data.led !== 'undefined') {
          const el = document.getElementById("led");
          if (el) el.innerText = data.led ? "AN" : "AUS";
        }
      } catch (e) {
        // Ignorieren, wenn andere JSONs kommen (z.B. Status)
      }
    };

    window.sendLED = function(state) {
      const msg = { led: !!state, blink: false };
      socket.send(JSON.stringify(msg));
    };
    window.sendBlink = function() {
      const msg = { led: false, blink: true };
      socket.send(JSON.stringify(msg));
    };
  } catch(e) {
    console.error("WebSocket init Fehler:", e);
  }
})();
