/*
   ____  __  __  ____  _  _  _____       ___  _____  ____  _  _
  (  _ \(  )(  )(_  _)( \( )(  _  )___  / __)(  _  )(_  _)( \( )
   )(_) ))(__)(  _)(_  )  (  )(_)((___)( (__  )(_)(  _)(_  )  (
  (____/(______)(____)(_)\_)(_____)     \___)(_____)(____)(_)\_)
  Non-Official code for all ESP8266 boards            version 4.3
  Main .ino file

  JK Rolling
  - Wifi disabled
  - use USB-Serial communication just like Arduino
  - board library version 3.1.2

  Original credit
  The Duino-Coin Team & Community 2019-2024 © MIT Licensed
  https://duinocoin.com
  https://github.com/revoxhere/duino-coin

  If you don't know where to start, visit official website and navigate to
  the Getting Started page. Have fun mining!
*/

/* If optimizations cause problems, change them to -O0 (the default) */
#pragma GCC optimize("-Ofast")

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include "MiningJob.h"
#include "Settings.h"

#define CORE 1

void RestartESP() {
  ESP.reset();
}

// WDT Loop 
// See lwdtcb() and lwdtFeed() below
Ticker lwdTimer;

unsigned long lwdCurrentMillis = 0;
unsigned long lwdTimeOutMillis = LWD_TIMEOUT;

void ICACHE_RAM_ATTR lwdtcb(void) {
  if ((millis() - lwdCurrentMillis > LWD_TIMEOUT) || (lwdTimeOutMillis - lwdCurrentMillis != LWD_TIMEOUT))
    RestartESP();
}

void lwdtFeed(void) {
  lwdCurrentMillis = millis();
  lwdTimeOutMillis = lwdCurrentMillis + LWD_TIMEOUT;
}

MiningJob *job[CORE];

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    Serial.setTimeout(10000);
    while (!Serial)
      ;
    Serial.flush();
    pinMode(LED_BUILTIN, OUTPUT);
    
    // turn off wifi
    WiFi.mode(WIFI_OFF);

    job[0] = new MiningJob(0);

    // Start the WDT watchdog
    lwdtFeed();
    lwdTimer.attach_ms(LWD_TIMEOUT, lwdtcb);

    // Fastest clock mode for 8266s
    system_update_cpu_freq(160);
    os_update_cpu_frequency(160);
    // Feed the watchdog
    lwdtFeed();

    job[0]->blink(BLINK_SETUP_COMPLETE);
}

void single_core_loop() {
    job[0]->mine();
    lwdtFeed();
}

void loop() {
  single_core_loop();
  delay(10);
}
