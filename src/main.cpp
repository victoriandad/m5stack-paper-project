#include <Arduino.h>
#include <M5EPD.h>
#include <WiFi.h>
#include "wifi_manager.h"  // Include the WiFi manager header

// Define an enum for display rotation
enum DisplayRotation {
  ROTATION_VERTICAL = 0,
  ROTATION_HORIZONTAL = 1
};

// Define constants for canvas dimensions, text sizes, and update mode
const int CANVAS_WIDTH = 540;
const int CANVAS_HEIGHT = 960;
const int TEXT_SIZE_SMALL = 1;   // Small text size
const int TEXT_SIZE_MEDIUM = 2;  // Medium text size
const int TEXT_SIZE_LARGE = 3;   // Large text size
const int ORIGIN_X = 0;          // Canvas origin X-coordinate
const int ORIGIN_Y = 0;          // Canvas origin Y-coordinate
const m5epd_update_mode_t FULL_REFRESH_MODE = UPDATE_MODE_GC16;  // Full grayscale refresh mode

M5EPD_Canvas canvas(&M5.EPD);  // Create a canvas object tied to the display

// Mock calendar event struct
struct CalendarEvent {
    String title;
    String date;  // Add a date field (format: YYYY-MM-DD)
    int hour;
    int minute;
};

// Expanded fake event list with 25 items over a 3-month period
CalendarEvent events[] = {
    {"Team meeting (Sean)", "05/05/25", 9, 30},
    {"Lunch with Anna (Luigina)", "05/05/25", 12, 15},  // Same day as "Team meeting"
    {"Dentist appointment (Sean)", "07/05/25", 16, 0},
    {"Project deadline (Sean)", "08/05/25", 10, 0},
    {"Weekly review (Sean)", "10/05/25", 11, 0},
    {"Gym session (Luigina)", "12/05/25", 18, 0},
    {"Dinner with family (Sean)", "13/05/25", 19, 30},
    {"Doctor's appointment (Luigina)", "15/05/25", 8, 15},
    {"Client presentation (Sean)", "16/05/25", 13, 0},
    {"Pick up groceries (Luigina)", "18/05/25", 17, 15},
    {"Car service (Sean)", "01/06/25", 8, 0},
    {"Birthday party (Luca)", "05/06/25", 20, 0},
    {"Team outing (Sean)", "10/06/25", 18, 30},
    {"Weekend hike (Luca)", "15/06/25", 7, 0},
    {"Summer camp drop-off (Loris)", "01/07/25", 9, 0},
    {"Family picnic (All)", "04/07/25", 12, 0},
    {"Doctor's appointment (Luigina)", "07/07/25", 10, 30},
    {"Project presentation (Sean)", "10/07/25", 14, 0},
    {"Evening walk (Luigina)", "15/07/25", 18, 30},
    {"School meeting (Loris)", "20/07/25", 16, 0},
    {"Beach day (Family)", "25/07/25", 10, 0}
};

String getDayOfWeek(const String& date) {
    // Extract day, month, and year from the date string
    int day = date.substring(0, 2).toInt();
    int month = date.substring(3, 5).toInt();
    int year = date.substring(6, 8).toInt() + 2000;  // Convert YY to YYYY

    // Zeller's Congruence Algorithm to calculate the day of the week
    if (month < 3) {
        month += 12;
        year -= 1;
    }
    int k = year % 100;
    int j = year / 100;
    int dayOfWeek = (day + (13 * (month + 1)) / 5 + k + (k / 4) + (j / 4) - (2 * j)) % 7;

    // Map the result to the corresponding day name
    String days[] = {"Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
    return days[(dayOfWeek + 7) % 7];  // Ensure positive index
}

String getMonthName(const String& month) {
    // Map numeric month values to month names
    String months[] = {"January", "February", "March", "April", "May", "June", 
                       "July", "August", "September", "October", "November", "December"};
    int monthIndex = month.toInt() - 1;  // Convert to zero-based index
    if (monthIndex >= 0 && monthIndex < 12) {
        return months[monthIndex];
    }
    return "Unknown";  // Return "Unknown" for invalid input
}

void drawCalendar() {
    canvas.createCanvas(CANVAS_WIDTH, CANVAS_HEIGHT);
    canvas.setTextSize(TEXT_SIZE_MEDIUM);  // Use medium text size for better fit
    canvas.setTextDatum(TL_DATUM);
    canvas.drawString("Upcoming Events", 20, 20);
    canvas.drawLine(20, 60, 520, 60, 0, 1);  // Add a horizontal line for the header

    String currentMonth = "";  // Track the current month to group events
    String currentDay = "";    // Track the current day to group events
    int y = 80;

    for (const auto& event : events) {
        // Extract the month and year from the date
        String eventMonth = event.date.substring(3, 5);
        String eventYear = event.date.substring(6, 8);

        // Check if the event belongs to a new month
        if (eventMonth != currentMonth) {
            currentMonth = eventMonth;

            // Add a subsection header for the new month with the year
            canvas.setTextSize(TEXT_SIZE_LARGE);  // Larger text for month headers
            canvas.drawString(getMonthName(currentMonth) + " '" + eventYear, 20, y);  // Display month and year
            y += 30;  // Add spacing after the month header
            canvas.setTextSize(TEXT_SIZE_MEDIUM);  // Reset to medium text size
        }

        // Check if the event belongs to a new day
        if (event.date != currentDay) {
            currentDay = event.date;

            // Extract the day only (remove month and year)
            String eventDay = currentDay.substring(0, 2);

            // Add a subsection header for the new day with the day of the week
            String dayOfWeek = getDayOfWeek(currentDay);
            canvas.setTextSize(TEXT_SIZE_LARGE);  // Larger text for day headers
            canvas.drawString("  " + eventDay + " - " + dayOfWeek, 20, y);
            y += 30;  // Add spacing after the day header
            canvas.setTextSize(TEXT_SIZE_MEDIUM);  // Reset to medium text size
        }

        // Format and display the event time and title
        char timeStr[16];
        sprintf(timeStr, "%02d:%02d", event.hour, event.minute);
        canvas.drawString("    " + String(timeStr) + "  " + event.title, 40, y);
        y += 25;  // Reduced line spacing for compact display

        // Stop drawing if the content exceeds the screen height
        if (y > CANVAS_HEIGHT - 40) {
            break;
        }
    }

    canvas.pushCanvas(ORIGIN_X, ORIGIN_Y, FULL_REFRESH_MODE);
}

void setup() {
    M5.begin();
    Serial.begin(115200);
    M5.EPD.SetRotation(ROTATION_HORIZONTAL);

    drawCalendar();
    connectToWiFi(canvas);  // Call the function from wifi_manager.cpp
}

void loop() {
    // For now, nothing
}

