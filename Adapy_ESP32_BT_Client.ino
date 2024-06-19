#include <Arduino.h>
#include "BluetoothSerial.h"

// Configuration parameters
const char* btServerName = "Adapy_BT_Server";
const unsigned long resendInterval = 340; // Resend interval in milliseconds

// Bluetooth Serial object
BluetoothSerial SerialBT;

// GPIO pins for all 8 buttons
constexpr int buttonPins[] = {32, 33, 21, 26, 18, 27, 4, 5};

// State tracking for buttons
bool buttonStates[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH}; // Assume buttons are pulled up
unsigned long lastSendTime[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // Track last send time for each button

void setup() {
    Serial.begin(19200);
    Serial.println("Starting Bluetooth Client");

    // Initialize button pins
    for (int i = 0; i < 8; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }

    // Initialize Bluetooth
    SerialBT.begin("ESP32_Client");
    while (!SerialBT.connect(btServerName)) {
        Serial.println("Failed to connect, retrying...");
        delay(1000);
    }
    Serial.println("Connected to server");
}

void loop() {
    unsigned long currentMillis = millis();

    if (!SerialBT.connected()) {
        Serial.println("Disconnected from Bluetooth server");
        while (!SerialBT.connect(btServerName)) {
            Serial.println("Reconnecting...");
            delay(1000);
        }
        Serial.println("Reconnected to server");
    }

    // Check button states
    for (int i = 0; i < 8; i++) {
        bool currentState = digitalRead(buttonPins[i]);
        if (currentState != buttonStates[i]) {
            buttonStates[i] = currentState;
            String message = (currentState == LOW) ? "DOWN_" + String(i) : "UP_" + String(i);
            Serial.println("Sending: " + message);
            SerialBT.println(message);
            lastSendTime[i] = currentMillis;
        } else if (currentState == LOW && (currentMillis - lastSendTime[i] >= resendInterval)) {
            // Button is held down, resend message
            String message = "DOWN_" + String(i);
            Serial.println("Resending: " + message);
            SerialBT.println(message);
            lastSendTime[i] = currentMillis;
        }
    }

    // Check if we receive anything from the Bluetooth server
    if (SerialBT.available()) {
        String message = SerialBT.readString(); // Read what we have received
        Serial.print("Received: ");
        Serial.println(message);
    }

    delay(20); // Delay to prevent overwhelming the ESP32
}
