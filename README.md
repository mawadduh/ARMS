# Artificial Roof Misting System

## Description
The Artificial Roof Misting System is a smart system designed to monitor and control environmental conditions such as temperature, humidity, and rain presence on rooftops. It utilizes various sensors and actuators to ensure optimal conditions for plants or other applications.

## Features
- Real-time monitoring of temperature, humidity, and rain presence.
- Automatic control of a misting system based on environmental conditions.
- Integration with the OpenWeatherMap API for weather data.
- Alerts for high humidity, high temperature, and rain presence.
- Ultrasonic sensor for water tank level measurement.
- LCD display for visual feedback.
- Remote monitoring and control through the Blynk platform.

## System Alerts
- **Humidity Alert**: Triggered when humidity levels are above a certain threshold.
- **Temperature Alert**: Triggered when temperature levels are above a certain threshold.
- **Rain Alert**: Triggered when rain is detected by the rain sensor.
- **Water Level Alert**: Triggered when the water level in the tank is below a certain threshold.

## Installation
1. Clone this repository to your local machine.
2. Open the Arduino IDE and upload the sketch to your ESP32 board.
3. Ensure all necessary libraries are installed.
4. Connect the sensors and actuators according to the specified pin configurations.

## Configuration
- Set your WiFi credentials and Blynk authentication token in the sketch.
- Adjust the tank max value and other sensor thresholds as needed.

## Credits
- Mawaddah Hasnan - Project Developer
- OpenWeatherMap - Weather Data Provider
- Blynk - IoT Platform
- Special thanks to SriTu Hobby for providing the initial codebase.
