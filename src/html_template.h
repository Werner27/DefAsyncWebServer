const char html_template[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 UI</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
  <nav>
      <a href="/index">Home</a>
      <div class="dropdown">
          <button class="dropbtn">Menu01</button>
          <div class="dropdown-content">
              <a href="/status">Status</a>
              <a href="/websocket">WebSocket</a>
              <a href="/test">Test</a>
          </div>
      </div>
      <a href="/config">Konfiguration</a>
      <a href="/update">OTA</a>
  </nav>
  <h1>ESP32 Web-UI</h1>
  <h2 class="h_color">Template mit dynamischen Inhalten</h2>
  <p>Diese Seite nutzt ein Template, in das der Seiteninhalt eingefügt wird.</p>
    _BODY_CONTENT_
  <script src="/script.js"></script>
</body>
</html>
)rawliteral";
