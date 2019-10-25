// Example usage for LPD8806 library by Adafruit.

#include "LPD8806.h"

// Initialize objects from the lib
LPD8806 lPD8806;

void setup() {
    // Call functions on initialized library objects that require hardware
    lPD8806.begin();
}

void loop() {
    // Use the library's initialized objects and functions
    lPD8806.process();
}
