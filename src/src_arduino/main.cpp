// Minimal Arduino sketch for BlackPill F411CE
// On many BlackPill boards, the user LED is on PC13 (active LOW)
#include <Arduino.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN PC13
#endif

// For clarity
constexpr uint32_t BLINK_MS = 500;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    // On BlackPill PC13 LED is often active-low: start OFF
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);
    delay(100);
    Serial.println("\nSTM32F411 (BlackPill) | Arduino core | Blink + Serial ready");
}

void loop()
{
    // Toggle LED
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.println("tick");
    delay(BLINK_MS);
}
