# smart-agricultural-based-rover
# 🤖 ESP32 2-Wheel Rover with GPS, Obstacle Avoidance & Web Control

This project showcases a feature-rich, two-wheeled autonomous rover using an **ESP32** microcontroller. The rover combines GPS-based tracking, real-time web dashboard, and obstacle avoidance with ultrasonic and IR sensors. Built for experimentation and low-cost robotics, it features a Google Maps interface and basic SLAM-style logic!

---

## 📦 Features

- 🌐 **Wi-Fi Web Interface** with control panel and live data
- 📍 **GPS Tracking** with Leaflet + OpenStreetMap
- 🧭 **Obstacle Avoidance** using Ultrasonic + IR sensors
- ⚙️ **Manual and Automatic Navigation**
- 🔁 **Expandable** for SLAM & Path Planning (A* Simulation)

---

## 🧠 Components

| Component                 | Quantity | Notes                            |
|--------------------------|----------|----------------------------------|
| ESP32 Dev Board          | 1        | Core controller                  |
| L298N Motor Driver       | 1        | For controlling DC motors        |
| Geared DC Motors + Wheels| 2        | Rover movement                   |
| Ultrasonic Sensor (HC-SR04) | 2     | Obstacle detection (front/side)  |
| IR Obstacle Sensor       | 2        | Rear object detection            |
| GPS Module (NEO-6M)      | 1        | Positioning                      |
| MPU6050 (Gyro + Accel)   | 1        | Orientation sensing              |
| Power Supply (Battery Bank) | 1     | 5V output preferred              |
| Chassis, Wires, etc.     | 1 set    | Assembly hardware                |

---

## 🔌 ESP32 Pin Mapping

| Component              | ESP32 GPIO | Notes                           |
|------------------------|-------------|---------------------------------|
| L298N IN1/IN2          | GPIO 13 / 12| Left motor direction            |
| L298N IN3/IN4          | GPIO 14 / 27| Right motor direction           |
| ENA / ENB (Motor PWM)  | GPIO 32 / 33| Speed control (optional)        |
| Ultrasonic TRIG / ECHO | GPIO 25 / 26| Front obstacle detection        |
| IR Sensor (Rear)       | GPIO 2      | Obstacle behind                 |
| GPS TX / RX            | GPIO 16 / 17| Serial2 port (GPS connection)   |
| MPU6050 SDA / SCL      | GPIO 21 / 22| I2C Communication               |

---

## 🌐 Web Interface

- Displays live GPS location on OpenStreetMap via Leaflet.js
- Provides Start/Stop buttons and motor control
- Periodically fetches `/gps` data for location updates

![Web UI Preview](https://user-images.githubusercontent.com/your-image-placeholder.png)

---

## 🚦 Behavior Logic

- If an obstacle is detected in front, the rover:
  - Stops
  - Moves backward briefly
  - Turns right to avoid collision

- Rear IR sensor prevents backward motion if blocked

---

## 🚀 Getting Started

1. **Clone the Repo** and upload the code to your ESP32 via Arduino IDE.
2. Modify your Wi-Fi credentials:
   ```cpp
   const char* ssid = "Your_SSID";
   const char* password = "Your_PASSWORD";
   # 🌾 Smart Agriculture Weather Station (Dual ESP8266 Nodes)

A fully functional **agricultural monitoring and automation system** using two ESP8266 microcontrollers. It wirelessly monitors environmental conditions and automatically controls irrigation via a water pump. Features real-time sensor updates, a mobile-friendly web dashboard, historical data charts, and Telegram alerts!

---

## 📦 Features

- 📡 **Two ESP8266 Nodes**: One for sensors, one for control
- 🌡️ Reads **temperature, humidity, pressure, light, rain, and soil moisture**
- 🔁 Automatic and manual **pump control**
- 🌐 Web dashboard (Chart.js + HTML5) for real-time and historical monitoring
- 🛜 Sends **alerts via Telegram**
- 🔧 Servo motor auto-inserts soil moisture probe

---

## ⚙️ System Architecture

### 🧠 ESP #1 – **Sensor Node (Field Scout)**

- Controls servo motor to insert soil sensor
- Reads data from:
  - 🌱 Soil Moisture (analog)
  - 🌡️ DHT11 (Temperature & Humidity)
  - 📉 BMP180 (Pressure)
  - ☀️ LDR (Light)
  - 🌧️ Rain Sensor (digital)
- Sends HTTP POST data to the Control Node every minute

### 💧 ESP #2 – **Control Node (Dashboard + Pump Control)**

- Receives sensor data via `/update`
- Web server shows:
  - Live readings and auto-refreshing dashboard
  - Pump status
  - Graphs of last 10 values per sensor (via Chart.js)
- Automatically or manually controls the pump
- Sends Telegram alerts when pump turns on/off

---

## 📊 Web Dashboard Preview

- View live readings (Temp, Humidity, Pressure, LDR, Soil, Rain)
- Line charts for each sensor (last 10 values)
- Buttons for manual pump control
- Auto-refresh every 60 seconds

---

## 🔌 Wiring Overview

### Sensor Node (ESP8266)

| Component      | Pin       |
|----------------|-----------|
| DHT11          | D2 (GPIO4)|
| BMP180 (I2C)   | SDA: D1, SCL: D2 |
| Soil Sensor    | A0        |
| LDR            | A0 (shared) |
| Rain Sensor    | D5 (GPIO14)|
| Servo Motor    | D6 (GPIO12)|

> Use external power for the servo, and voltage dividers or relays to multiplex A0 inputs if needed.

### Control Node (ESP8266)

| Component      | Pin       |
|----------------|-----------|
| Relay (Pump)   | D1 (GPIO5)|

---

## 📶 Communication Flow

- Both ESP8266s connect to the **same Wi-Fi network**
- Sensor Node sends data to the **Control Node’s IP** via HTTP POST (`/update`)
- Control Node processes, displays, and reacts to the data

---

## 🧪 Setup Instructions

1. Flash **Sensor Node code**:
   - Add your Wi-Fi credentials
   - Replace `CONTROL_ESP8266_IP` with the Control Node’s IP
2. Flash **Control Node code**:
   - Use same Wi-Fi credentials
   - Add Telegram bot token & chat ID
3. Power both ESP8266 boards
4. Visit the Control Node’s IP in a browser (e.g., `http://192.168.1.45`)
5. Monitor live dashboard and pump activity

---

## 📲 Telegram Alerts

Get notified when the pump changes state:

- ✅ Soil is fine. Pump turned OFF.
- 🚨 Soil is too dry! Pump turned ON 💧
- 🧠 Manual: Pump turned ON/OFF

### Setup

1. Create a bot via [@BotFather](https://t.me/BotFather)
2. Get your chat ID from [@userinfobot](https://t.me/userinfobot)
3. Add both values to the code:
   ```cpp
   String botToken = "YOUR_BOT_TOKEN";
   String chatID = "YOUR_CHAT_ID";
📂 SmartAgroWeatherStation/
├── sensor_node.ino        # Main field data collector (ESP #1)
├── control_node.ino       # Web server + pump logic (ESP #2)
├── README.md              # Project documentation

