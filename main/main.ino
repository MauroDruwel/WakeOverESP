#include <WiFi.h>
#include <WebServer.h>
#include "Adafruit_TinyUSB.h"

#if defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS) || defined(ARDUINO_NRF52840_CIRCUITPLAY)
const int pin = 4; // Left Button
bool activeState = true;

#elif defined(ARDUINO_FUNHOUSE_ESP32S2)
const int pin = BUTTON_DOWN;
bool activeState = true;

#elif defined PIN_BUTTON1
const int pin = PIN_BUTTON1;
bool activeState = false;

#elif defined(ARDUINO_ARCH_ESP32)
const int pin = 0;
bool activeState = false;

#elif defined(ARDUINO_ARCH_RP2040)
const int pin = D0;
bool activeState = false;
#else
const int pin = A0;
bool activeState = false;
#endif

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_MOUSE()
};

// USB HID object
Adafruit_USBD_HID usb_hid;

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
    // Manual begin() is required on core without built-in support e.g. mbed rp2040
    if (!TinyUSBDevice.isInitialized()) {
        TinyUSBDevice.begin(0);
    }

    Serial.begin(115200);
    // Set up button, pullup opposite to active state
    pinMode(pin, activeState ? INPUT_PULLDOWN : INPUT_PULLUP);

    // Set up HID
    usb_hid.setBootProtocol(HID_ITF_PROTOCOL_MOUSE);
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setStringDescriptor("TinyUSB Mouse");
    usb_hid.begin();

    // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
    if (TinyUSBDevice.mounted()) {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }
    connectToWiFi();

    // Setup HTTP server routes
    server.on("/wakeup", HTTP_GET, handleWakeup);
    server.begin();
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

void process_hid() {
  // Whether button is pressed
  bool btn_pressed = (digitalRead(pin) == activeState);

  // nothing to do if button is not pressed
  if (!btn_pressed) return;

  // Remote wakeup
  if (TinyUSBDevice.suspended()) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  if (usb_hid.ready()) {
    uint8_t const report_id = 0; // no ID
    int8_t const delta = 5;
    usb_hid.mouseMove(report_id, delta, delta); // right + down
  }
}

// HTTP handler for /wakeup route
void handleWakeup() {
    sendWakeUpSignal();
    server.send(200, "text/plain", "Wake-up signal sent!");
}

// Function to send wake-up signal using HID mouse
void sendWakeUpSignal() {
    process_hid();
}