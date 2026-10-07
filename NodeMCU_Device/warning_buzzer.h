#ifndef NODEMCU_WARNING_BUZZER_H
#define NODEMCU_WARNING_BUZZER_H

#include <Arduino.h>
#include "config.h"

unsigned long lastBuzzerToggleMillis = 0;
bool buzzerState = false;

inline void startBuzzer()
{
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // Short boot chirp to confirm buzzer is working
    digitalWrite(BUZZER_PIN, HIGH);
    delay(80);
    digitalWrite(BUZZER_PIN, LOW);
}

inline void updateBuzzer(int warningLevel, bool hasValidSignal)
{
    // If signal is lost or boat is in safe waters, keep buzzer silent
    if (!hasValidSignal || warningLevel == WARNING_SAFE)
    {
        digitalWrite(BUZZER_PIN, LOW);
        buzzerState = false;
        return;
    }

    unsigned long currentMillis = millis();

    if (warningLevel == WARNING_DANGER)
    {
        // Rapid urgent alarm: 150ms ON / 150ms OFF
        if (currentMillis - lastBuzzerToggleMillis >= DANGER_BEEP_INTERVAL_MS)
        {
            lastBuzzerToggleMillis = currentMillis;
            buzzerState = !buzzerState;
            digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
        }
    }
    else if (warningLevel == WARNING_CAUTION)
    {
        // Intermittent caution warning: 400ms ON / 400ms OFF
        if (currentMillis - lastBuzzerToggleMillis >= CAUTION_BEEP_INTERVAL_MS)
        {
            lastBuzzerToggleMillis = currentMillis;
            buzzerState = !buzzerState;
            digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
        }
    }
}

#endif

