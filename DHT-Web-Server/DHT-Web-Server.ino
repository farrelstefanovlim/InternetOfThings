#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// Konfigurasi Sensor DHT pada Wemos D1 R2
#define DHTPIN D2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Konfigurasi Wi-Fi Access Point (AP)
const char* ssid = "Wemos-Monitoring-DHT";
const char* password = "password123"; // Minimal 8 karakter

// Inisialisasi Web Server lokal pada port 80
ESP8266WebServer server(80);

// Halaman Web Dashboard (HTML + CSS + JS)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Monitoring Suhu & Kelembapan Wemos D1 R2</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }
    body { background-color: #0f172a; color: #f8fafc; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; padding: 20px; }
    .header { text-align: center; margin-bottom: 30px; }
    .header h1 { font-size: 1.8rem; color: #38bdf8; margin-bottom: 5px; }
    .header p { font-size: 0.9rem; color: #94a3b8; }
    .container { display: grid; grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); gap: 20px; width: 100%; max-width: 600px; }
    .card { background: #1e293b; border-radius: 16px; padding: 25px; text-align: center; box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.3); border: 1px solid #334155; }
    .card h2 { font-size: 1.1rem; color: #cbd5e1; margin-bottom: 15px; text-transform: uppercase; letter-spacing: 1px; }
    .value { font-size: 3rem; font-weight: bold; }
    .temp { color: #f97316; }
    .hum { color: #06b6d4; }
    .unit { font-size: 1.5rem; font-weight: normal; }
    .footer { margin-top: 30px; font-size: 0.85rem; color: #94a3b8; text-align: center; line-height: 1.7; }
    .status { display: inline-block; width: 10px; height: 10px; background-color: #22c55e; border-radius: 50%; margin-right: 6px; }
    .timestamp { font-weight: 600; color: #38bdf8; font-variant-numeric: tabular-nums; }
  </style>
</head>
<body>
  <div class="header">
    <h1>Wemos D1 R2 Environmental Monitor</h1>
    <p>Local Offline Web Server (Access Point Mode)</p>
  </div>

  <div class="container">
    <div class="card">
      <h2>Suhu (Temperature)</h2>
      <div class="value temp"><span id="temp">--</span><span class="unit">&deg;C</span></div>
    </div>
    <div class="card">
      <h2>Kelembapan (Humidity)</h2>
      <div class="value hum"><span id="hum">--</span><span class="unit">%</span></div>
    </div>
  </div>

  <div class="footer">
    <p><span class="status" id="status-dot"></span><span id="status-text">Menunggu data sensor...</span></p>
    <p>Pembaruan terakhir: <span class="timestamp" id="last-updated">--:--:--</span></p>
  </div>

  <script>
    function getFormattedTime() {
      const now = new Date();
      const jam = String(now.getHours()).padStart(2, '0');
      const menit = String(now.getMinutes()).padStart(2, '0');
      const detik = String(now.getSeconds()).padStart(2, '0');
      return `${jam}:${menit}:${detik}`;
    }

    function fetchSensorData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          const statusDot = document.getElementById('status-dot');
          const statusText = document.getElementById('status-text');
          const lastUpdated = document.getElementById('last-updated');
          
          if (data.status === "ok") {
            document.getElementById('temp').innerText = data.temperature.toFixed(1);
            document.getElementById('hum').innerText = data.humidity.toFixed(1);
            statusDot.style.backgroundColor = "#22c55e";
            statusText.innerText = "Terhubung - Diperbarui tiap 1 detik";
            lastUpdated.innerText = getFormattedTime();
          } else {
            statusDot.style.backgroundColor = "#ef4444";
            statusText.innerText = "Error: Gagal membaca sensor DHT!";
          }
        })
        .catch(() => {
          document.getElementById('status-dot').style.backgroundColor = "#ef4444";
          document.getElementById('status-text').innerText = "Koneksi ke Wemos terputus";
        });
    }

    fetchSensorData();
    setInterval(fetchSensorData, 1000);
  </script>
</body>
</html>
)rawliteral";

// Handler untuk halaman utama (/)
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// Handler untuk endpoint JSON (/data)
void handleData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  String json = "{";
  if (isnan(h) || isnan(t)) {
    json += "\"status\":\"error\"";
  } else {
    json += "\"status\":\"ok\",";
    json += "\"temperature\":" + String(t) + ",";
    json += "\"humidity\":" + String(h);
  }
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  Serial.println("\nMengaktifkan Access Point pada Wemos D1 R2...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Nama Wi-Fi (SSID): ");
  Serial.println(ssid);
  Serial.print("Alamat IP Dashboard: http://");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
  Serial.println("HTTP Web Server aktif!");
}

void loop() {
  server.handleClient();
}