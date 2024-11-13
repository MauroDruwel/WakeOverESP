#include <WiFi.h>
#include <WebServer.h>
#include "hidmouse.h"

#if CFG_TUD_HID
HIDmouse mouse;

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// HTTP server
WebServer server(80);

// Function declarations
void connectToWiFi();
void handleWakeup();
void sendWakeUpSignal();

void setup() {
    Serial.begin(115200);
    connectToWiFi();

    // Setup HTTP server routes
    server.on("/wakeup", HTTP_GET, handleWakeup);
    server.begin();

    // Initialize HID mouse
    mouse.begin();
}

void loop() {
    // Handle incoming client requests
    server.handleClient();

    // Reconnect to WiFi if disconnected
    if (WiFi.status() != WL_CONNECTED) {
        connectToWiFi();
    }
}

// Function to connect to WiFi
void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");

    unsigned long startAttemptTime = millis();
    const unsigned long wifiTimeout = 30000; // 30 seconds timeout

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
        delay(1000);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
    } else {
        Serial.println("\nFailed to connect to WiFi");
    }
}

// HTTP handler for /wakeup route
void handleWakeup() {
    sendWakeUpSignal();
    server.send(200, "text/plain", "Wake-up signal sent!");
}

// Function to send wake-up signal using HID mouse
void sendWakeUpSignal() {
    mouse.pressLeft();
}

#endif