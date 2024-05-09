#define BLYNK_TEMPLATE_ID "###"
#define BLYNK_TEMPLATE_NAME "###"
#define BLYNK_AUTH_TOKEN "###"

// Include the library files
#include <Wire.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <ArduinoJson.h>

// microcontroller pins with the sensor
#define TH 5        // temperature and humidty pin
#define Rain 36     // rain sensor pin
#define trig 12     // ultrasonic sensor pin
#define echo 13     // ultrasonic sensor pin
#define relay 33    // relay pin

//Ultrasonic sensor water tank measuring
//Enter your tank max value(CM)
int MaxLevel = 23;
int Level1 = (MaxLevel * 75) / 100;
int Level2 = (MaxLevel * 65) / 100;
int Level3 = (MaxLevel * 55) / 100;
int Level4 = (MaxLevel * 35) / 100;

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Blynk Auth token
char auth[] = "###";

// Call openweathermap API to Blynk
const char* ssid     = "###";         // SSID of local network
const char* password = "###";     // Password on the network
String APIKEY = "###";
String CityID = "###";                // Your City ID
WiFiClient client;
char servername[] = "api.openweathermap.org"; // remote server we will connect to
String result;

DHT dht(TH, DHT11);
BlynkTimer timer;
int rainPresence = 0;   // rain sensor alert variable

void connectToWiFi() {                      //a function that handles the process 
                                            //of connecting to a WiFi network.
  Serial.println(F("Connecting to WiFi"));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(F("Connecting to WiFi..."));
  }
  Serial.println(F("Connected to WiFi"));
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());
}

void sendWeatherDataToBlynk(String location, int temperature, int humidity, float windSpeed, float precipitation) {
  Serial.println("Location: " + location);
  Serial.println("Temperature: " + String(temperature) + " °C");
  Serial.println("Humidity: " + String(humidity) + " %");
  Serial.println("Wind Speed: " + String(windSpeed) + " m/s");
  Serial.println("Precipitation: " + String(precipitation) + " mm");

  Blynk.virtualWrite(V5, location);      // Virtual Pin V5 for location
  Blynk.virtualWrite(V6, temperature);   // Virtual Pin V6 for temperature
  Blynk.virtualWrite(V7, humidity);      // Virtual Pin V7 for humidity
  Blynk.virtualWrite(V8, windSpeed);     // Virtual Pin V8 for wind speed
  Blynk.virtualWrite(V9, precipitation); // Virtual Pin V9 for precipitation
}

void sendWeatherDataWrapper() {
  if (client.connect(servername, 80)) {
    client.println("GET /data/2.5/weather?id=" + CityID + "&units=metric&APPID=" + APIKEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println(F("Connection to openweathermap failed"));
  }

  while (client.connected() && !client.available())
    delay(1);

  while (client.connected() || client.available()) {
    char c = client.read();
    result = result + c;
  }

  client.stop();
  result.replace('[', ' ');
  result.replace(']', ' ');

  char jsonArray[result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, jsonArray);

  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  // Extract data
  String location = doc["name"];
  int temperature = doc["main"]["temp"];
  int humidity = doc["main"]["humidity"];
  float windSpeed = doc["wind"]["speed"];
  float precipitation = doc["rain"]["1h"];

  // Send data to Blynk and Serial Monitor
  sendWeatherDataToBlynk(location, temperature, humidity, windSpeed, precipitation);
}

void setup() {
  // Debug console
  Serial.begin(115200);
  connectToWiFi();
  Blynk.begin(auth, ssid, password, "blynk.cloud", 80);
  timer.setInterval(10000, sendWeatherDataWrapper);  // Send openapi data to Blynk every 10 seconds
  while (!Blynk.connected()) {
    delay(1000);
    Serial.println("Waiting for Blynk connection...");
  }

  dht.begin();

  lcd.begin(16, 2);  // Initialize the LCD
  lcd.init();
  pinMode(trig, OUTPUT);        // Ultrasonic sensor
  pinMode(echo, INPUT);         // Ultrasonic sensor
  pinMode(relay, OUTPUT);       // Relay
  digitalWrite(relay, HIGH);    // Relay

  lcd.backlight();
  pinMode(Rain, INPUT);
  analogReadResolution(12);

  lcd.setCursor(16, 0);
  lcd.print("System");
  lcd.setCursor(4, 1);
  lcd.print("Loading..");

  delay(4000);
  lcd.clear();
}

// Get the DHT11 sensor values
void DHT11sensor() {
  float t = dht.readTemperature();  //temp
  float h = dht.readHumidity();     //humidity

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V0, t); //temp
  Blynk.virtualWrite(V1, h); //humidity

  // Alert for high humidity
  if(h>=60){
    Blynk.logEvent("humidity_alert");
  }

  // Alert for high temperature
  if(t>=30){
    Blynk.logEvent("temperature_alert");
  }

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);

  lcd.setCursor(8, 0);
  lcd.print("H:");
  lcd.print(h);
}

//Get the ultrasonic sensor values
void ultrasonic() {
  digitalWrite(trig, LOW);
  delayMicroseconds(4);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long t = pulseIn(echo, HIGH);
  int distance = t / 29 / 2;

  // Serial.println(distance);

  int blynkDistance = (distance - MaxLevel) * -1;
  if (distance <= MaxLevel) {
    Blynk.virtualWrite(V3, blynkDistance);        
  } else {
    Blynk.virtualWrite(V3, 0);
  }
  lcd.setCursor(16, 0);
  lcd.print("WLevel:");

  if (Level1 <= distance) {
    lcd.setCursor(24, 0);
    lcd.print("Very Low");
    // Serial.println("WLevel is very low");
  } else if (Level2 <= distance && Level1 > distance) {
    lcd.setCursor(24, 0);
    lcd.print("Low");
    lcd.print("      ");
    // Serial.println("WLevel is low");
  } else if (Level3 <= distance && Level2 > distance) {
    lcd.setCursor(24, 0);
    lcd.print("Medium");
    lcd.print("      ");
    // Serial.println("WLevel is medium");
  } else if (Level4 <= distance && Level3 > distance) {
    lcd.setCursor(24, 0);
    lcd.print("Full");
    lcd.print("      ");
    // Serial.println("WLevel is full");
  }  

  if(Level3 <= distance && Level2 > distance){
   Blynk.logEvent("water_level_alert");
  }
}

//Get the button value for relay to turn on and off water pump
BLYNK_WRITE(V4) {
  bool Relay = param.asInt();          // This function gets called when the state of the 
                                      //Blynk button widget changes
  Serial.print("Received Blynk command for relay: ");
  Serial.println(Relay);
  
  if (Relay == 1) {
    digitalWrite(relay, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Motor is ON ");
  } else {
    digitalWrite(relay, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Motor is OFF");
  }

  Serial.print("Relay state after Blynk command: ");
  Serial.println(digitalRead(relay));
  
}

// Get the rain sensor values
void rainSensor() {
  int Rvalue = analogRead(Rain);            // Read analog value from the rain sensor
  rainPresence = digitalRead(Rain);

  // The map function scales the analog value from the raw sensor range (assumed to be 0-4095) to a percentage range (0-100). 
  // This step is useful for normalizing the sensor readings to a more understandable range.
  Rvalue = map(Rvalue, 0, 4095, 0, 100);    // Map the analog value to a percentage

  // Adjust the rain sensor value
  // This adjustment might be specific to the behavior of the rain sensor used in the project.
  Rvalue = (Rvalue - 100) * -1;

  // Alert for rain presence
  if(rainPresence == 1){
    Blynk.logEvent("rain_alert");
  }

  // Alert for rain presence II
  if(Rvalue == 1){
    Blynk.logEvent("rain_alert");
  }


  // Send the adjusted rain sensor value to Blynk virtual pin V2
  Blynk.virtualWrite(V2, Rvalue);
}

void loop() {
  DHT11sensor();  // Call the DHT11sensor() function to read temperature and humidity from the DHT sensor
  rainSensor();   // Call the rainSensor() function to read data from the rain sensor
  ultrasonic();   // Call the ultrasonic() function to measure water tank level using the ultrasonic sensor
  Blynk.run();    // Run Blynk communication
  timer.run();  // Run BlynkTimer
  delay(1000);      // Introduce a small delay for stability
}
