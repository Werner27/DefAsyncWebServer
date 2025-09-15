const char html_template[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Seite via Code</title>
    <style>
    body {
      font-family: Arial, sans-serif; margin: 0; padding: 0;
      background-color: #f4f4f4; color: #333; text-align: center;
    }
    nav { background-color: #333; overflow: hidden; }
    nav a {
      float: left; display: block; color: white; text-align: center;
      padding: 14px 16px; text-decoration: none;
    }
    nav a:hover { background-color: #ddd; color: black; }
    .dropdown { float: left; overflow: hidden; }
    .dropdown .dropbtn {
      font-size: 16px; border: none; outline: none; color: white;
      padding: 14px 16px; background-color: inherit; font-family: inherit; margin: 0;
    }
    nav a:hover, .dropdown:hover .dropbtn { background-color: #ddd; color: black; }
    .dropdown-content {
      display: none; position: absolute; background-color: #f9f9f9; min-width: 180px;
      box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2); z-index: 1;
    }
    .dropdown-content a {
      float: none; color: black; padding: 12px 16px; text-decoration: none; display: block; text-align: left;
    }
    .dropdown-content a:hover { background-color: #ddd; }
    .dropdown:hover .dropdown-content { display: block; }
    .content {
      background-color: #dfeb44ff; border: 1px solid #ddd; padding: 20px; text-align: center;
      max-width: 900px; margin: 16px auto;
    }
    </style>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <nav>
        <a href="/index">Home</a>
        <div class="dropdown">
            <button class="dropbtn">Menu01</button>
            <div class="dropdown-content">
                <a href="/submenu01">HTMLviaCode</a>
                <a href="/status">Status</a>
                <a href="/websocket">WebSocket</a>
            </div>
        </div>
        <a href="/config">Konfiguration</a>
        <a href="/update">OTA</a>
    </nav>

    <h1>Meine Seite via Code</h1>
    <h2>Template mit dynamischem Inhalt</h2>
    <p>Diese HTML-Seite wurde im Code definiert. Sie besteht aus einem Template, in das der Seiteninhalt eingefügt wird.</p>

    _BODY_CONTENT_
    <script src="/script.js"></script>
</body>
</html>
)rawliteral";
