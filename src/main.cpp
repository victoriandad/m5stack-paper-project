#include <Arduino.h>
#include <M5EPD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "wifi_manager.h"

#define DEBUG_ESP_SSL

// Web App URL from your Google Apps Script deployment
const char* calendarUrl = "https://script.google.com/macros/s/AKfycbyKx-l2S6gIDvvMzhOOGu-Hsarx-rtLdbjRACbiMJ_giWOVoNUCbOBgj18VFH7-BpBYSw/exec";

// Root CA Certificate
const char* rootCACertificate = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDrzCCApegAwIBAgIQB9Dktt8F8t5z3e1z5XzjEjANBgkqhkiG9w0BAQsFADBh\n"
"MQswCQYDVQQGEwJVUzETMBEGA1UEChMKR29vZ2xlIExMQzERMA8GA1UECxMIR29v\n"
"Z2xlIENBMRcwFQYDVQQDEw5Hb29nbGUgUm9vdCBDQTAeFw0yMTAyMDIxMzI1MDBa\n"
"Fw0zMTAyMDIxMzI1MDBaMGExCzAJBgNVBAYTAlVTMRMwEQYDVQQKEwpHb29nbGUg\n"
"TExDMREwDwYDVQQLEwhHb29nbGUgQ0ExFzAVBgNVBAMTDkdvb2dsZSBSb290IENB\n"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA7dY3k5Z4z+2dhz3e1z5X\n"
"zjEjANBgkqhkiG9w0BAQsFADBhMQswCQYDVQQGEwJVUzETMBEGA1UEChMKR29vZ2\n"
"xlIExMQzERMA8GA1UECxMIR29vZ2xlIENBMRcwFQYDVQQDEw5Hb29nbGUgUm9vdC\n"
"...\n"
"-----END CERTIFICATE-----\n";

// Display and layout constants
enum DisplayRotation { ROTATION_VERTICAL = 0, ROTATION_HORIZONTAL = 1 };
const int CANVAS_WIDTH = 540;
const int CANVAS_HEIGHT = 960;
const int TEXT_SIZE_SMALL = 1;
const int TEXT_SIZE_MEDIUM = 2;
const int TEXT_SIZE_LARGE = 3;
const int ORIGIN_X = 0;
const int ORIGIN_Y = 0;
const m5epd_update_mode_t FULL_REFRESH_MODE = UPDATE_MODE_GC16;

M5EPD_Canvas canvas(&M5.EPD);

// Event struct
struct CalendarEvent {
    String title;
    String date;  // format: DD/MM/YY
    int hour;
    int minute;
};

// Helper: Convert "DD/MM/YY" to weekday
String getDayOfWeek(const String& date) {
    int day = date.substring(0, 2).toInt();
    int month = date.substring(3, 5).toInt();
    int year = date.substring(6, 8).toInt() + 2000;

    if (month < 3) {
        month += 12;
        year -= 1;
    }
    int k = year % 100;
    int j = year / 100;
    int dayOfWeek = (day + (13 * (month + 1)) / 5 + k + (k / 4) + (j / 4) - 2 * j) % 7;

    String days[] = {"Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
    return days[(dayOfWeek + 7) % 7];
}

String getMonthName(const String& month) {
    String months[] = {"January", "February", "March", "April", "May", "June",
                       "July", "August", "September", "October", "November", "December"};
    int index = month.toInt() - 1;
    return (index >= 0 && index < 12) ? months[index] : "Unknown";
}

// Fetch and parse events from Google Apps Script
std::vector<CalendarEvent> fetchCalendarEvents(const String& url) {
    std::vector<CalendarEvent> result;

    WiFiClientSecure client;
    client.setCACert(rootCACertificate);  // Use the root CA certificate

    HTTPClient http;
    String currentUrl = url;
    int redirectCount = 0;
    const int maxRedirects = 5;  // Limit the number of redirects to prevent infinite loops

    while (redirectCount < maxRedirects) {
        http.begin(client, currentUrl);

        // 👇 Spoof browser user-agent
        http.setUserAgent("Mozilla/5.0 (ESP32 M5Stack)");

        int httpCode = http.GET();
        Serial.print("HTTP GET returned: ");
        Serial.println(httpCode);

        // Log all HTTP headers for debugging
        Serial.println("HTTP Headers:");
        for (int i = 0; i < http.headers(); i++) {
            Serial.print(http.headerName(i));
            Serial.print(": ");
            Serial.println(http.header(i));
        }

        if (httpCode == 302) {
            // Handle redirect
            String redirectUrl = http.header("Location");
            Serial.print("Redirecting to: ");
            Serial.println(redirectUrl);

            if (redirectUrl.isEmpty()) {
                Serial.println("Redirect URL is empty. Aborting.");
                break;
            }

            currentUrl = redirectUrl;  // Update the URL for the next request
            redirectCount++;
            http.end();  // Close the current connection before retrying
        } else if (httpCode == 200) {
            // Successfully fetched the data
            String payload = http.getString();
            Serial.println("Payload:");
            Serial.println(payload);

            StaticJsonDocument<4096> doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                for (JsonObject event : doc.as<JsonArray>()) {
                    CalendarEvent ev;
                    ev.title = event["title"].as<String>();
                    String start = event["start"].as<String>();
                    ev.date = start.substring(8, 10) + "/" +
                              start.substring(5, 7) + "/" +
                              start.substring(2, 4);
                    ev.hour = start.substring(11, 13).toInt();
                    ev.minute = start.substring(14, 16).toInt();
                    result.push_back(ev);
                }
            } else {
                Serial.println("Failed to parse JSON");
            }
            http.end();
            return result;  // Exit the loop after successfully fetching data
        } else {
            Serial.println("HTTP request failed");
            break;
        }
    }

    if (redirectCount >= maxRedirects) {
        Serial.println("Too many redirects. Aborting.");
    }

    http.end();
    return result;
}

// Draw calendar using event list
void drawCalendar(const std::vector<CalendarEvent>& events) {
    canvas.createCanvas(CANVAS_WIDTH, CANVAS_HEIGHT);
    canvas.clear();
    canvas.setTextSize(TEXT_SIZE_MEDIUM);
    canvas.setTextDatum(TL_DATUM);
    canvas.drawString("Upcoming Events", 20, 20);
    canvas.drawLine(20, 60, 520, 60, 0, 1);

    // 🧪 No events? Show fallback message
    if (events.empty()) {
        canvas.setTextSize(TEXT_SIZE_MEDIUM);
        canvas.drawString("No events found.", 20, 100);
        canvas.pushCanvas(ORIGIN_X, ORIGIN_Y, FULL_REFRESH_MODE);
        return;
    }

    String currentMonth = "";
    String currentDay = "";
    int y = 80;

    for (const auto& event : events) {
        String eventMonth = event.date.substring(3, 5);
        String eventYear = event.date.substring(6, 8);

        if (eventMonth != currentMonth) {
            currentMonth = eventMonth;
            canvas.setTextSize(TEXT_SIZE_LARGE);
            canvas.drawString(getMonthName(currentMonth) + " '" + eventYear, 20, y);
            y += 30;
            canvas.setTextSize(TEXT_SIZE_MEDIUM);
        }

        if (event.date != currentDay) {
            currentDay = event.date;
            String eventDay = currentDay.substring(0, 2);
            String dayOfWeek = getDayOfWeek(currentDay);
            canvas.setTextSize(TEXT_SIZE_LARGE);
            canvas.drawString("  " + eventDay + " - " + dayOfWeek, 20, y);
            y += 30;
            canvas.setTextSize(TEXT_SIZE_MEDIUM);
        }

        char timeStr[16];
        sprintf(timeStr, "%02d:%02d", event.hour, event.minute);
        canvas.drawString("    " + String(timeStr) + "  " + event.title, 40, y);
        y += 25;

        if (y > CANVAS_HEIGHT - 40) {
            break;
        }
    }

    canvas.pushCanvas(ORIGIN_X, ORIGIN_Y, FULL_REFRESH_MODE);
}

// Main startup
void setup() {
    M5.begin();
    Serial.begin(115200);
    M5.EPD.SetRotation(ROTATION_HORIZONTAL);
    M5.EPD.Clear(true);  // Clear any ghosting

    connectToWiFi(canvas);

    std::vector<CalendarEvent> events = fetchCalendarEvents(calendarUrl);
    Serial.print("Total parsed events: ");
    Serial.println(events.size());

    drawCalendar(events);
}

void loop() {
    // Placeholder for future interaction or timed refresh
}
