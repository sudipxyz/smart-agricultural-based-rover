// ESP32 Rover Code with GPS, MPU6050, Obstacle Avoidance, Web Dashboard

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Hardware setup
Adafruit_MPU6050 mpu;
TinyGPSPlus gps;
HardwareSerial GPSserial(1);
WebServer server(80);

// Motor pins
const int ENA = 14;
const int IN1 = 27;
const int IN2 = 26;
const int ENB = 25;
const int IN3 = 33;
const int IN4 = 32;

// Ultrasonic sensor
const int trigPin = 4;
const int echoPin = 2;

// MPU6050 data
float ax, ay, az;

// GPS data
double latitude = 0.0;
double longitude = 0.0;

void setup() {
  Serial.begin(115200);
  GPSserial.begin(9600, SERIAL_8N1, 16, 17);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  Wire.begin();
  mpu.begin();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  server.on("/", handleRoot);
  server.on("/data", []() {
    String json = "{";
    json += "\"lat\":" + String(latitude, 6) + ",";
    json += "\"lon\":" + String(longitude, 6) + ",";
    json += "\"ax\":" + String(ax, 2) + ",";
    json += "\"ay\":" + String(ay, 2) + ",";
    json += "\"az\":" + String(az, 2);
    json += "}";
    server.send(200, "application/json", json);
  });
  server.begin();
}

void loop() {
  server.handleClient();
  readMPU();
  readGPS();
  obstacleAvoidance();
}

void readMPU() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  ax = a.acceleration.x;
  ay = a.acceleration.y;
  az = a.acceleration.z;

  if (abs(ax) > 9.8 || abs(ay) > 9.8) {
    Serial.println("Fall detected!");
  }
}

void readGPS() {
  while (GPSserial.available()) {
    gps.encode(GPSserial.read());
    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
    }
  }
}

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH) * 0.034 / 2;
}

void obstacleAvoidance() {
  long distance = getDistance();
  if (distance < 15) {
    stopMotors(); delay(500);
    moveBackward(); delay(500);
    turnRight(); delay(500);
  }
  moveForward();
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>ESP32 Rover Dashboard</title>
      <style>
        body { font-family: Arial; text-align: center; margin-top: 30px; }
        #map { height: 300px; width: 100%; }
        canvas { max-width: 500px; }
      </style>
      <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.3/dist/leaflet.css" />
      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
      <script src="https://unpkg.com/leaflet@1.9.3/dist/leaflet.js"></script>
    </head>
    <body>
      <h2>ESP32 Rover Dashboard</h2>
      <div id="map"></div>
      <canvas id="mpuChart"></canvas>
      <script>
        var map = L.map('map').setView([0, 0], 15);
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
          attribution: 'Map data © OpenStreetMap contributors'
        }).addTo(map);
        var marker = L.marker([0, 0]).addTo(map);

        var mpuChart = new Chart(document.getElementById('mpuChart').getContext('2d'), {
          type: 'line',
          data: {
            labels: [],
            datasets: [
              { label: 'X', borderColor: 'red', data: [], fill: false },
              { label: 'Y', borderColor: 'green', data: [], fill: false },
              { label: 'Z', borderColor: 'blue', data: [], fill: false }
            ]
          },
          options: { responsive: true }
        });

        setInterval(() => {
          fetch('/data')
            .then(res => res.json())
            .then(data => {
              marker.setLatLng([data.lat, data.lon]);
              map.setView([data.lat, data.lon]);
              mpuChart.data.labels.push('');
              mpuChart.data.datasets[0].data.push(data.ax);
              mpuChart.data.datasets[1].data.push(data.ay);
              mpuChart.data.datasets[2].data.push(data.az);
              if (mpuChart.data.labels.length > 20) {
                mpuChart.data.labels.shift();
                mpuChart.data.datasets.forEach(ds => ds.data.shift());
              }
              mpuChart.update();
            });
        }, 1000);
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}
