#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

// Pin Definitions
#define DHTPIN D4           // GPIO2
#define DHTTYPE DHT11
#define SERVOPIN D5         // GPIO14
#define RAINPIN D3          // GPIO0
#define SOILPIN A0          // Analog input

// WiFi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Control Node URL (Update this IP!)
const char* serverUrl = "http://192.168.1.150/update";

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
WiFiClient client;

void setup() {
  Serial.begin(115200);
  dht.begin();
  bmp.begin();
  
  pinMode(RAINPIN, INPUT);
  pinMode(SERVOPIN, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi. IP: " + WiFi.localIP().toString());
}

void loop() {
  insertSensor();  // Activate servo

  // Read Sensors
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float pressure = bmp.readPressure() / 100.0; // hPa
  int rainValue = digitalRead(RAINPIN);
  int soilMoisture = analogRead(SOILPIN);
  delay(100);  // Small gap for ADC
  int ldrValue = analogRead(SOILPIN);  // Same pin if using A0 for both

  // Debugging output
  Serial.println("Sending data...");
  Serial.printf("Temp: %.1f, Hum: %.1f, Pressure: %.1f, Soil: %d, LDR: %d, Rain: %d\n", 
                temperature, humidity, pressure, soilMoisture, ldrValue, rainValue);

  // Send via HTTP POST
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, serverUrl);  // ✅ Fixed: use WiFiClient
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = 
      "temp=" + String(temperature) +
      "&hum=" + String(humidity) +
      "&pres=" + String(pressure) +
      "&soil=" + String(soilMoisture) +
      "&ldr=" + String(ldrValue) +
      "&rain=" + String(rainValue);

    int httpCode = http.POST(postData);
    Serial.println("POST Status: " + String(httpCode));
    http.end();
  }

  delay(60000);  // Wait 60 seconds
}

void insertSensor() {
  analogWrite(SERVOPIN, 50);  // Forward
  delay(2000);
  analogWrite(SERVOPIN, 120); // Reverse
  delay(2000);
  analogWrite(SERVOPIN, 0);   // Stop
}
