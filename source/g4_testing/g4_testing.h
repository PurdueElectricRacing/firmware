#ifndef __G4_TESTING__
#define __G4_TESTING__

// To add new tests create a separate file (see led_blink.c) and it to the enum here
#define TEST_BLINKY 0
#define TEST_FDCAN  1
#define TEST_USART  2
#define TEST_SPI     3
#define TEST_NUCLEO  4

// Change this define to set the test compiled
#define G4_TESTING_CHOSEN TEST_NUCLEO

#endif // __G4_TESTING__
