#ifndef __G4_TESTING__
#define __G4_TESTING__

// To add new tests create a separate file (see led_blink.c) and it to the enum here
#define TEST_BLINKY 0
#define TEST_FDCAN  1
#define TEST_SPI    2

// Change this define to set the test compiled
#define G4_TESTING_CHOSEN TEST_SPI

#endif // __G4_TESTING__
