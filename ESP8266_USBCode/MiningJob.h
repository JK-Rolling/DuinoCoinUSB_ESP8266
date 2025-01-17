#pragma GCC optimize("-Ofast")

#ifndef MINING_JOB_H
#define MINING_JOB_H

#include <Arduino.h>
#include <assert.h>
#include <string.h>
#include <Ticker.h>
#include <WiFiClient.h>

#include "DSHA1.h"
#include "Counter.h"
#include "Settings.h"

// https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TypeConversion.cpp
const char base36Chars[36] PROGMEM = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

const uint8_t base36CharValues[75] PROGMEM{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0,                                                                        // 0 to 9
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 0, 0, 0, 0, 0, 0, // Upper case letters
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35                    // Lower case letters
};

extern void lwdtFeed();

#define SPC_TOKEN ' '
#define END_TOKEN '\n'
#define SEP_TOKEN ','
#define IOT_TOKEN '@'

class MiningJob {

public:
    int core = 0;

    MiningJob(int core) {
        this->core = core;
        this->client_buffer = "";
        dsha1 = new DSHA1();
        dsha1->warmup();
    }

    void blink(uint8_t count, uint8_t pin = LED_BUILTIN) {
        #if defined(LED_BLINKING)
            uint8_t state = HIGH;

            for (int x = 0; x < (count << 1); ++x) {
                digitalWrite(pin, state ^= HIGH);
                delay(50);
            }
        #else
            digitalWrite(LED_BUILTIN, HIGH);
        #endif
    }

    bool max_micros_elapsed(unsigned long current, unsigned long max_elapsed) {
        static unsigned long _start = 0;

        if ((current - _start) > max_elapsed) {
            _start = current;
            return true;
        }
        return false;
    }

    void handleSystemEvents(void) {
        delay(10); // Required vTaskDelay by ESP-IDF
        lwdtFeed();
        yield();
    }

    void mine() {
        askForJob();
          
        dsha1->reset().write((const unsigned char *)getLastBlockHash().c_str(), getLastBlockHash().length());

        int start_time = micros();
        max_micros_elapsed(start_time, 0);
        #if defined(LED_BLINKING)
          digitalWrite(LED_BUILTIN, LOW);
        #endif
        for (Counter<10> counter; counter < difficulty; ++counter) {
            DSHA1 ctx = *dsha1;
            ctx.write((const unsigned char *)counter.c_str(), counter.strlen()).finalize(hashArray);
            
            #ifndef CONFIG_FREERTOS_UNICORE
                #define SYSTEM_TIMEOUT 500000 // 50ms for 8266 for same reason as above
                if (max_micros_elapsed(micros(), SYSTEM_TIMEOUT)) {
                    handleSystemEvents();
                } 
            #endif

            if (memcmp(getExpectedHash(), hashArray, 20) == 0) {
                unsigned long elapsed_time = micros() - start_time;

                #if defined(LED_BLINKING)
                    digitalWrite(LED_BUILTIN, HIGH);
                #endif

                submit(counter, elapsed_time);
                
                break;
            }
        }
    }

private:
    String client_buffer;
    uint8_t hashArray[20];
    String last_block_hash;
    String expected_hash_str;
    uint8_t expected_hash[20];
    DSHA1 *dsha1;
    String chipID = String(ESP.getChipId(), HEX);

    uint8_t *hexStringToUint8Array(const String &hexString, uint8_t *uint8Array, const uint32_t arrayLength) {
        assert(hexString.length() >= arrayLength * 2);
        const char *hexChars = hexString.c_str();
        for (uint32_t i = 0; i < arrayLength; ++i) {
            uint8Array[i] = (pgm_read_byte(base36CharValues + hexChars[i * 2] - '0') << 4) + pgm_read_byte(base36CharValues + hexChars[i * 2 + 1] - '0');
        }
        return uint8Array;
    }

    void submit(unsigned long counter, unsigned long elapsed_time) {
        // Clearing the receive buffer before sending the result.
        while (Serial.available()) Serial.read();
        
        Serial.print(String(counter, 2) 
                        + "," 
                        + String(elapsed_time, 2) 
                        + ",DUCOID"
                        + String(chipID)
                        + "\n");
    }

    bool parse() {
        // Create a non-constant copy of the input string
        char *job_str_copy = strdup(client_buffer.c_str());

        if (job_str_copy) {
            String tokens[3];
            char *token = strtok(job_str_copy, ",");
            for (int i = 0; token != NULL && i < 3; i++) {
                tokens[i] = token;
                token = strtok(NULL, ",");
            }

            last_block_hash = tokens[0];
            expected_hash_str = tokens[1];
            hexStringToUint8Array(expected_hash_str, expected_hash, 20);
            difficulty = tokens[2].toInt() * 100 + 1;

            // Free the memory allocated by strdup
            free(job_str_copy);

            return true;
        }
        else {
            // Handle memory allocation failure
            return false;
        }
    }

    void askForJob() {
        // Wait for serial data
        while (true) {
          if (Serial.available() > 0) {
            client_buffer = Serial.readStringUntil(END_TOKEN);
            if (client_buffer.length() == 1 && client_buffer[0] == END_TOKEN)
                client_buffer = "???\n"; // NOTE: Should never happen
            parse();
            // Clearing the receive buffer reading one job.
            while (Serial.available()) Serial.read();
            break;
          }
          
          if (max_micros_elapsed(micros(), 100000)) {
              handleSystemEvents();
          }
        }
    }

    const String &getLastBlockHash() const { return last_block_hash; }
    const String &getExpectedHashStr() const { return expected_hash_str; }
    const uint8_t *getExpectedHash() const { return expected_hash; }
    unsigned int getDifficulty() const { return difficulty; }
};

#endif
