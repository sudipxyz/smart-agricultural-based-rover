#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

#define DHTPIN D1
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

ESP8266WebServer server(80);

const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

int soilMoisture = 0;
int ldrValue = 0;
int rainValue = 0;
bool pumpOn = false;
String pumpStatus = "OFF";

float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;

#define SOIL_PIN A0
#define LDR_PIN A0
#define RAIN_PIN D6
#define PUMP_PIN D7

#define HISTORY_SIZE 20
float tempHistory[HISTORY_SIZE];
float humHistory[HISTORY_SIZE];
float soilHistory[HISTORY_SIZE];
float ldrHistory[HISTORY_SIZE];
float presHistory[HISTORY_SIZE];

int historyIndex = 0;

void updateSensorData() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  pressure = bmp.readPressure() / 100.0;
  soilMoisture = analogRead(SOIL_PIN);
  ldrValue = analogRead(LDR_PIN);
  rainValue = digitalRead(RAIN_PIN);

  tempHistory[historyIndex] = temperature;
  humHistory[historyIndex] = humidity;
  soilHistory[historyIndex] = soilMoisture;
  ldrHistory[historyIndex] = ldrValue;
  presHistory[historyIndex] = pressure;

  historyIndex = (historyIndex + 1) % HISTORY_SIZE;
}

void handleRoot() {
  String html = R"====(
  <!DOCTYPE html>
  <html lang=\"en\">
  <head>
    <meta charset=\"UTF-8\">
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
    <title>Agro Monitor</title>
    <script src=\"https://cdn.tailwindcss.com\"></script>
    <script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>
  </head>
  <body class=\"bg-gray-100 text-gray-800 min-h-screen flex flex-col items-center p-4\">
    <div class=\"w-full max-w-4xl\">
      <h1 class=\"text-3xl font-bold mb-4 text-center\">🌾 Smart Agriculture Dashboard</h1>

      <div class=\"grid grid-cols-1 md:grid-cols-2 gap-4\">
        <div class=\"bg-white rounded-2xl shadow p-4\">
          <p><strong>🌡 Temperature:</strong> )====" + String(temperature) + R"====( °C</p>
          <p><strong>💧 Humidity:</strong> )====" + String(humidity) + R"====( %</p>
          <p><strong>🎈 Pressure:</strong> )====" + String(pressure) + R"====( hPa</p>
          <p><strong>🌱 Soil Moisture:</strong> )====" + String(soilMoisture) + R"====(</p>
          <p><strong>☀️ Light Level (LDR):</strong> )====" + String(ldrValue) + R"====(</p>
          <p><strong>🌧 Rain:</strong> )====" + ((rainValue == 0) ? "Raining" : "Clear") + R"====(</p>
          <p><strong>🚰 Pump Status:</strong> )====" + pumpStatus + R"====(</p>

          <form method=\"POST\" action=\"/control\" class=\"mt-4 flex gap-2\">
            <button name=\"state\" value=\"on\" class=\"px-4 py-2 bg-green-500 hover:bg-green-600 text-white rounded-xl\">Turn Pump ON</button>
            <button name=\"state\" value=\"off\" class=\"px-4 py-2 bg-red-500 hover:bg-red-600 text-white rounded-xl\">Turn Pump OFF</button>
          </form>
        </div>

        <div class=\"bg-white rounded-2xl shadow p-4 overflow-x-auto\">
          <canvas id=\"chart\" width=\"400\" height=\"300\"></canvas>
        </div>
      </div>
    </div>

    <script>
      setInterval(() => location.reload(), 60000);

      fetch('/data')
        .then(res => res.json())
        .then(data => {
          new Chart(document.getElementById('chart'), {
            type: 'line',
            data: {
              labels: [...Array(data.temp.length).keys()],
              datasets: [
                { label: 'Temp', data: data.temp, borderColor: 'red', fill: false },
                { label: 'Humidity', data: data.hum, borderColor: 'blue', fill: false },
                { label: 'Soil', data: data.soil, borderColor: 'green', fill: false },
                { label: 'LDR', data: data.ldr, borderColor: 'orange', fill: false },
                { label: 'Pressure', data: data.pres, borderColor: 'purple', fill: false }
              ]
            },
            options: {
              responsive: true,
              maintainAspectRatio: false,
              scales: {
                y: { beginAtZero: true }
              }
            }
          });
        });
    </script>
  </body>
  </html>
  )====";

  server.send(200, "text/html", html);
}

void handleData() {
  StaticJsonDocument<1024> doc;
  JsonArray temp = doc.createNestedArray("temp");
  JsonArray hum = doc.createNestedArray("hum");
  JsonArray soil = doc.createNestedArray("soil");
  JsonArray ldr = doc.createNestedArray("ldr");
  JsonArray pres = doc.createNestedArray("pres");

  for (int i = 0; i < HISTORY_SIZE; i++) {
    int idx = (historyIndex + i) % HISTORY_SIZE;
    temp.add(tempHistory[idx]);
    hum.add(humHistory[idx]);
    soil.add(soilHistory[idx]);
    ldr.add(ldrHistory[idx]);
    pres.add(presHistory[idx]);
  }

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleControl() {
  if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "on") {
      digitalWrite(PUMP_PIN, HIGH);
      pumpOn = true;
      pumpStatus = "ON";
    } else {
      digitalWrite(PUMP_PIN, LOW);
      pumpOn = false;
      pumpStatus = "OFF";
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  bmp.begin();
  pinMode(RAIN_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/control", HTTP_POST, handleControl);

  server.begin();
}

void loop() {
  updateSensorData();
  server.handleClient();
  delay(10000);
}
