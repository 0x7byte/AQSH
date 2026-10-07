#ifndef BOAT_WARNING_H
#define BOAT_WARNING_H

#include "config.h"

#ifndef WARNING_SAFE
#define WARNING_SAFE 0
#define WARNING_CAUTION 1
#define WARNING_DANGER 2
#endif

unsigned long lastWarningOutputChangeMillis = 0;
bool warningOutputIsOn = false;

int determineWarningLevel(float minimumDistance)
{
    if (minimumDistance <= DANGER_DISTANCE_METERS)
    {
        return WARNING_DANGER;
    }
    else if (minimumDistance <= WARNING_DISTANCE_METERS)
    {
        return WARNING_CAUTION;
    }

    return WARNING_SAFE;
}

const char *getWarningLabel(int warningLevel)
{
    if (warningLevel == WARNING_DANGER)
    {
        return "DANGER";
    }
    else if (warningLevel == WARNING_CAUTION)
    {
        return "WARNING";
    }

    return "SAFE";
}

void updateWarningOutputs(int warningLevel)
{
    unsigned long currentMillis = millis();

    // In SAFE mode: Green LED is ALWAYS ON! Red LED and Buzzer are OFF.
    if (warningLevel == WARNING_SAFE)
    {
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, LOW);
        warningOutputIsOn = false;
        return;
    }

    // In WARNING or DANGER: Green LED is ALWAYS OFF!
    digitalWrite(GREEN_LED_PIN, LOW);

    if (warningLevel == WARNING_DANGER)
    {
        // Rapid urgent alarm: 150ms ON / 150ms OFF (matches Receiver!)
        if (currentMillis - lastWarningOutputChangeMillis >= DANGER_BEEP_INTERVAL_MS)
        {
            lastWarningOutputChangeMillis = currentMillis;
            warningOutputIsOn = !warningOutputIsOn;
            digitalWrite(RED_LED_PIN, warningOutputIsOn ? HIGH : LOW);
            digitalWrite(BUZZER_PIN, warningOutputIsOn ? HIGH : LOW);
        }
    }
    else if (warningLevel == WARNING_CAUTION)
    {
        // Intermittent caution beep: 400ms ON / 400ms OFF (matches Receiver!)
        if (currentMillis - lastWarningOutputChangeMillis >= CAUTION_BEEP_INTERVAL_MS)
        {
            lastWarningOutputChangeMillis = currentMillis;
            warningOutputIsOn = !warningOutputIsOn;
            digitalWrite(RED_LED_PIN, warningOutputIsOn ? HIGH : LOW);
            digitalWrite(BUZZER_PIN, warningOutputIsOn ? HIGH : LOW);
        }
    }
}

#endif
