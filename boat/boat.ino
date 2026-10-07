#include <Arduino.h>
#include "config.h"
#include "segments.h"
#include "geo.h"
#include "distance.h"
#include "warning.h"
#include "radio.h"
#include "simulation.h"

GPSPoint currentGpsPoint;
float currentMinimumDistance = 0.0;
int currentWarningLevel = WARNING_SAFE;
int currentBoundarySide = BOUNDARY_SIDE_OTHER;
bool radioIsReady = false;

void updateBoatPosition()
{
    float localX = convertLongitudeToLocalX(currentGpsPoint.longitude);
    float localY = convertLatitudeToLocalY(currentGpsPoint.latitude);

    BoundaryMeasurement boundaryMeasurement = measureBoundary(localX, localY);
    currentMinimumDistance = boundaryMeasurement.distanceMeters;
    currentBoundarySide = determineBoundarySide(boundaryMeasurement);
    currentWarningLevel = determineWarningLevel(currentMinimumDistance);

    Serial.println("--------------------------------------------------");
    Serial.print("[GPS 15s] Lat: ");
    Serial.print(currentGpsPoint.latitude, 6);
    Serial.print(" | Lon: ");
    Serial.print(currentGpsPoint.longitude, 6);
    Serial.print(" | Distance: ");
    Serial.print(currentMinimumDistance, 0);
    Serial.print(" m | Status: ");
    Serial.print(getWarningLabel(currentWarningLevel));
    Serial.print(" | Side: ");
    Serial.println(getBoundarySideLabel(currentBoundarySide));
}

void setup()
{
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println("==================================================");
    Serial.println("   AquaShield - Boat Unit (ESP32)                 ");
    Serial.println("   GPS Simulation: 15s | Sends ONLY if issue      ");
    Serial.println("==================================================");

    startLocationSource();

    if (readLocation(&currentGpsPoint))
    {
        updateBoatPosition();
    }

    radioIsReady = startBoatRadio();

    if (radioIsReady)
    {
        Serial.println("[OK] nRF24L01+ transmitter ready on Channel 108");
    }
    else
    {
        Serial.println("[ERROR] nRF24L01+ transmitter not found! Check 3.3V power and wiring.");
    }
}

void loop()
{
    // Read new simulated GPS position (advances every 15 seconds)
    if (readLocation(&currentGpsPoint))
    {
        // Calculate distance and warning level
        updateBoatPosition();

        // ONLY send data if there is an issue (WARNING or DANGER)
        if (currentWarningLevel != WARNING_SAFE)
        {
            if (radioIsReady)
            {
                bool packetSent = sendBoatData(currentGpsPoint.latitude, currentGpsPoint.longitude,
                                               currentMinimumDistance, currentWarningLevel,
                                               currentBoundarySide);

                if (packetSent)
                {
                    Serial.print("[ALERT TX] Issue detected (");
                    Serial.print(getWarningLabel(currentWarningLevel));
                    Serial.println(")! Packet transmitted to Coast Guard.");
                }
                else
                {
                    Serial.println("[ERROR] Radio transmission failed. Check nRF24 module.");
                }
            }
            else
            {
                Serial.println("[ERROR] Radio is NOT ready! Retrying nRF24 initialization...");
                radioIsReady = startBoatRadio();
                if (radioIsReady)
                {
                    Serial.println("[OK] nRF24L01+ transmitter recovered! Sending packet...");
                    sendBoatData(currentGpsPoint.latitude, currentGpsPoint.longitude,
                                 currentMinimumDistance, currentWarningLevel,
                                 currentBoundarySide);
                }
                else
                {
                    Serial.println("[ERROR] nRF24L01+ not responding! Check 3.3V power, GND, and SPI wiring.");
                }
            }
        }
        else
        {
            Serial.println("[SAFE] Boat is in safe waters (>2000m). No issue to transmit.");
        }
    }

    // Control local boat LEDs and buzzer
    updateWarningOutputs(currentWarningLevel);

    delay(20);
}
