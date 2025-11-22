#ifndef __F4_TESTING__
#define __F4_TESTING__

// To add new tests create a separate file (see led_blink.c) and it to the enum here
#define TEST_LED_BLINK     0
#define TEST_FREERTOS_DEMO 1
#define TEST_ONBOARDING_26 2
#define TEST_PWM 3

// Change this define to set the test compiled
#define F4_TESTING_CHOSEN TEST_PWM

#endif // __F4_TESTING__
