#include <WiFi.h>
#include "UbidotsESPMQTT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define WIFISSID "YourWiFiName"
#define PASSWORD "YourWiFiPassword"
#define TOKEN "Your_Ubidots_Token" // Replace with your Ubidots token
#define DEVICE_LABEL "gasdetector" // Your device name in Ubidots
#define VARIABLE_GAS "gas_level"
#define VARIABLE_ALERT "gas_alert"

#define ANALOG_GAS_PIN 34      // MQ-6 analog output
#define BUZZER_PIN 4
#define LED_PIN 2

LiquidCrystal_I2C lcd(0x27, 16, 2);
Ubidots ubidots(TOKEN);

void callback(char* topic, byte* payload, unsigned int length) {
  // Not needed now
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  WiFi.begin(WIFISSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");

  delay(1000);
  lcd.clear();

  ubidots.setCallback(callback);
  ubidots.wifiConnection(WIFISSID, PASSWORD);
  ubidots.begin();
}

void loop() {
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }
  ubidots.loop();

  int gasValue = analogRead(ANALOG_GAS_PIN); // MQ-6 analog read
  float voltage = gasValue * (3.3 / 4095.0); // Optional: voltage mapping
  bool gasDetected = gasValue > 1500; // Adjust threshold based on calibration

  Serial.print("Gas Value: ");
  Serial.println(gasValue);

  lcd.setCursor(0, 0);
  lcd.print("Gas: ");
  lcd.print(gasValue);

  if (gasDetected) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Status: LEAK!");
    ubidots.add(VARIABLE_ALERT, 1);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Status: Safe   ");
    ubidots.add(VARIABLE_ALERT, 0);
  }

  // Send data to Ubidots
  ubidots.add(VARIABLE_GAS, gasValue);
  ubidots.publish(DEVICE_LABEL);

  delay(3000); // Read every 3 seconds
}
