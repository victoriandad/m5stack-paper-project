#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <M5EPD.h>

// Canvas dimensions
const int CANVAS_WIDTH = 540;
const int CANVAS_HEIGHT = 960;

// Text sizes
const int TEXT_SIZE_SMALL = 1;   // Small text size
const int TEXT_SIZE_MEDIUM = 2;  // Medium text size
const int TEXT_SIZE_LARGE = 3;   // Large text size

// Canvas origin
const int ORIGIN_X = 0;          // Canvas origin X-coordinate
const int ORIGIN_Y = 0;          // Canvas origin Y-coordinate

// Update mode
const m5epd_update_mode_t FULL_REFRESH_MODE = UPDATE_MODE_GC16;  // Full grayscale refresh mode

#endif