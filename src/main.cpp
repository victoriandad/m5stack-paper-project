#include <Arduino.h>

// // put function declarations here:
// int myFunction(int, int);

// void setup() {
//   // put your setup code here, to run once:
//   int result = myFunction(2, 3);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
// }

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }

#include <M5EPD.h>

M5EPD_Canvas canvas(&M5.EPD);  // Create a canvas object tied to the display

void setup() {
  M5.begin();
  M5.EPD.SetRotation(1);        // Optional: set rotation to horizontal
  canvas.createCanvas(540, 960); // Full screen resolution
  canvas.setTextSize(3);        // Adjust text size
  canvas.drawString("Hello, M5Paper!", 100, 100);
  canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);  // GC16 = full grayscale refresh
}

void loop() {
  // Nothing yet
}

