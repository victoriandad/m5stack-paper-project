#include "wifi_manager.h"
#include "secrets.h"
#include "constants.h"  // Include shared constants

void connectToWiFi(M5EPD_Canvas& canvas) {
    // Create a smaller canvas for the WiFi status at the bottom
    canvas.createCanvas(CANVAS_WIDTH, 100);  // Height of 100 pixels for the status area
    canvas.setTextSize(TEXT_SIZE_MEDIUM);    // Use medium text size for status
    canvas.setTextDatum(TL_DATUM);           // Top-left alignment

    // Invert the canvas colors (white text on black background)
    canvas.setTextColor(15, 0);  // Set white text on black background

    // Display "Connecting to WiFi..." message
    canvas.fillCanvas(0);  // Clear the canvas with a black background
    canvas.drawString("Connecting to WiFi...", 20, 10);  // Initial status
    canvas.pushCanvas(ORIGIN_X, CANVAS_HEIGHT - 100, FULL_REFRESH_MODE);  // Push to bottom area

    // Attempt to connect to WiFi
    WiFi.begin(Config::SSID, Config::PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Display "WiFi connected!" and IP address
    canvas.fillCanvas(0);  // Clear the canvas with a black background
    canvas.drawString("WiFi connected!", 20, 10);  // Update status
    canvas.drawString("IP: " + WiFi.localIP().toString(), 20, 40);  // Display IP address
    canvas.pushCanvas(ORIGIN_X, CANVAS_HEIGHT - 100, FULL_REFRESH_MODE);  // Push to bottom area

    Serial.println("\nWiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Restore the canvas to normal colors (optional, if needed later)
    canvas.setTextColor(0, 15);  // Restore black text on white background
}