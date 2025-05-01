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

// Initialize the M5EPD object
M5EPD_Canvas canvas(&M5.EPD);

void setup() {
  // Initialize M5EPD
  M5.begin();
  M5.EPD.Clear(true);

  // Set up the canvas
  canvas.createCanvas(540, 960);
  canvas.setTextSize(3);
  canvas.drawString("Hello, M5Paper!", 10, 10);

  // Push the canvas to the display
  canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
}

void loop() {
  // Nothing to do here for now
}
