// Settings.h
#ifndef SETTINGS_H
#define SETTINGS_H

// Comment out the line below if you wish to disable LED blinking
#define LED_BLINKING

// Comment out the line below if you wish to disable Serial printing
#define SERIAL_PRINTING

// Edit the line below if you wish to change the serial speed (low values may reduce performance but are less prone to interference)
#define SERIAL_BAUDRATE 115200

// ESP8266 WDT loop watchdog. Do not edit this value, but if you must - do not set it too low or it will falsely trigger during mining!
#define LWD_TIMEOUT 30000

#define LED_BUILTIN 2

#define BLINK_SETUP_COMPLETE 2

extern unsigned int difficulty = 0;
#endif  // End of SETTINGS_H
