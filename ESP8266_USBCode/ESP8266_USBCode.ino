/*
  ,------.          ,--.                       ,-----.       ,--.
  |  .-.  \ ,--.,--.`--',--,--,  ,---. ,-----.'  .--./ ,---. `--',--,--,
  |  |  \  :|  ||  |,--.|      \| .-. |'-----'|  |    | .-. |,--.|      \
  |  '--'  /'  ''  '|  ||  ||  |' '-' '       '  '--'\' '-' '|  ||  ||  |
  `-------'  `----' `--'`--''--' `---'         `-----' `---' `--'`--''--'
  Non-Official code for ESP8266 boards                       version 2.7.5

  JK Rolling
  - Wifi disabled
  - use USB-Serial communication just like Arduino

  Original credit
  Duino-Coin Team & Community 2019-2021 © MIT Licensed
  https://duinocoin.com
  https://github.com/revoxhere/duino-coin

  If you don't know where to start, visit official website and navigate to
  the Getting Started page. Have fun mining!
*/

#define LEDON LOW
#define LEDOFF HIGH

/* If during compilation the line below causes a
  "fatal error: Crypto.h: No such file or directory"
  message to occur; it means that you do NOT have the
  latest version of the ESP8266/Arduino Core library.
  To install/upgrade it, go to the below link and
  follow the instructions of the readme file:
  https://github.com/esp8266/Arduino */
#include <Crypto.h>  // experimental SHA1 crypto library
using namespace experimental::crypto;

#include <Ticker.h>
#include <ESP8266WiFi.h>

String chipID = "";

// Loop WDT... please don't feed me...
// See lwdtcb() and lwdtFeed() below
Ticker lwdTimer;
#define LWD_TIMEOUT   60000

unsigned long lwdCurrentMillis = 0;
unsigned long lwdTimeOutMillis = LWD_TIMEOUT;

#define END_TOKEN  '\n'
#define SEP_TOKEN  ','

// Our new WDT to help prevent freezes
// code concept taken from https://sigmdel.ca/michel/program/esp8266/arduino/watchdogs2_en.html
void ICACHE_RAM_ATTR lwdtcb(void)
{
  if ((millis() - lwdCurrentMillis > LWD_TIMEOUT) || (lwdTimeOutMillis - lwdCurrentMillis != LWD_TIMEOUT))
    ESP.reset();
}

void lwdtFeed(void) {
  lwdCurrentMillis = millis();
  lwdTimeOutMillis = lwdCurrentMillis + LWD_TIMEOUT;
}

void handleSystemEvents(void) {
  yield();
}

bool max_micros_elapsed(unsigned long current, unsigned long max_elapsed) {
  static unsigned long _start = 0;

  if ((current - _start) > max_elapsed) {
    _start = current;
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10000);
  while (!Serial)
    ;
  Serial.flush();
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LEDOFF);

  // turn off wifi
  WiFi.mode(WIFI_OFF);

  chipID = String(ESP.getChipId(), HEX);

  lwdtFeed();
  lwdTimer.attach_ms(LWD_TIMEOUT, lwdtcb);
}

void loop() {
  // 1 minute watchdog
  lwdtFeed();

  // Wait for serial data
  if (Serial.available() > 0) {
    digitalWrite(LED_BUILTIN, LEDON);
    String last_block_hash = Serial.readStringUntil(SEP_TOKEN);
    String expected_hash = Serial.readStringUntil(SEP_TOKEN);
    unsigned int difficulty = strtoul(Serial.readStringUntil(SEP_TOKEN).c_str(), NULL, 10) * 100 + 1;

    // Clearing the receive buffer reading one job.
    while (Serial.available()) Serial.read();
    expected_hash.toUpperCase();

    float start_time = micros();
    max_micros_elapsed(start_time, 0);

    for (unsigned int duco_numeric_result = 0; duco_numeric_result < difficulty; duco_numeric_result++) {
        // Difficulty loop
        String result = SHA1::hash(last_block_hash + String(duco_numeric_result));

        if (result == expected_hash) {
        // If result is found
        unsigned long elapsed_time = micros() - start_time;

        // Clearing the receive buffer before sending the result.
        while (Serial.available()) Serial.read();

        Serial.print(String(duco_numeric_result, 2) 
                        + "," 
                        + String(elapsed_time, 2) 
                        + ",DUCOID"
                        + String(chipID)
                        + "\n");
                        
        digitalWrite(LED_BUILTIN, LEDOFF);
        break;
        }
    }
  }
  if (max_micros_elapsed(micros(), 250000))
    handleSystemEvents();
}
