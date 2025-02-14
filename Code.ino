#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

// LED Matrix Configuration
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define DATA_PIN 23  // ESP32 data pin (connect to DIN of matrix)
#define CLK_PIN 18   // ESP32 clock pin (connect to CLK of matrix)
#define CS_PIN 5     // ESP32 chip select pin (connect to CS/LOAD of matrix)

// Create Parola Display Object
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// WiFi Access Point Settings
const char* apSSID = "ESP32_Matrix";    // Name of the WiFi network
const char* apPassword = "12345678";    // Password for the network (at least 8 characters)

// Web Server on port 80
AsyncWebServer server(80);

// Variables to store user input
String displayText = "Afghan Robotic Team!";
int scrollSpeed = 50; // Default scroll speed

// HTML Web Page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 LED Matrix</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; }
        input, select, button { font-size: 16px; margin: 10px; padding: 8px; }
    </style>
</head>
<body>
    <h2>ESP32 LED Matrix Control</h2>
    <form action="/update" method="GET">
        <label>Enter Text:</label>
        <input type="text" name="text" placeholder="Enter message" required>
        <br>
        <label>Scroll Speed:</label>
        <select name="speed">
            <option value="30">Fast</option>
            <option value="50" selected>Medium</option>
            <option value="80">Slow</option>
        </select>
        <br>
        <button type="submit">Update Display</button>
    </form>
</body>
</html>
)rawliteral";

// Function to Update Display
void updateDisplay() {
  myDisplay.displayClear();
  myDisplay.displayScroll(displayText.c_str(), PA_CENTER, PA_SCROLL_LEFT, scrollSpeed);
}

void setup() {
  Serial.begin(115200);

  // Create Access Point
  WiFi.softAP(apSSID, apPassword);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Access Point IP: ");
  Serial.println(myIP);

  // Start Display
  myDisplay.begin();
  myDisplay.setIntensity(5);
  updateDisplay();  // Start with default message

  // Handle Web Page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  // Handle Text & Speed Updates
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("text")) {
      displayText = request->getParam("text")->value();
    }
    if (request->hasParam("speed")) {
      scrollSpeed = request->getParam("speed")->value().toInt();
    }
    updateDisplay();
    request->send(200, "text/plain", "Updated Successfully! Go back.");
  });

  // Start Server
  server.begin();
}

void loop() {
  if (myDisplay.displayAnimate()) {
    updateDisplay(); // Continue animation until text fully scrolls
  }
}
