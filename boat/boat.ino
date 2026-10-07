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

unsigned long lastRadioSendMillis = 0;

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
    Serial.println("   GPS Simulation: 15s | Telemetry: 2s Continuous ");
    Serial.println("==================================================");

    startLocationSource();

    if (readLocation(&currentGpsPoint))
    {
        updateBoatPosition();
    }

    radioIsReady = startBoatRadio();

    if (radioIsReady)
    {
        Serial.print("[OK] nRF24L01+ transmitter ready on Channel ");
        Serial.println(NRF_CHANNEL);

        // Send initial packet at boot so receiver confirms connection immediately
        sendBoatData(currentGpsPoint.latitude, currentGpsPoint.longitude,
                     currentMinimumDistance, currentWarningLevel,
                     currentBoundarySide);
        Serial.println("[TX #1] Initial connection packet broadcasted to Coast Guard Base.");
        lastRadioSendMillis = millis();
    }
    else
    {
        Serial.println("[ERROR] nRF24L01+ transmitter not found! Check 3.3V power and SPI wiring.");
    }
}

void loop()
{
    // Read new simulated GPS position (advances every 15 seconds)
    if (readLocation(&currentGpsPoint))
    {
        updateBoatPosition();
    }

    // Transmit telemetry packet every RADIO_SEND_INTERVAL_MS (2 seconds continuous)
    unsigned long currentMillis = millis();
    if (currentMillis - lastRadioSendMillis >= RADIO_SEND_INTERVAL_MS)
    {
        lastRadioSendMillis = currentMillis;

        if (radioIsReady)
        {
            sendBoatData(currentGpsPoint.latitude, currentGpsPoint.longitude,
                         currentMinimumDistance, currentWarningLevel,
                         currentBoundarySide);

            if (currentWarningLevel != WARNING_SAFE)
            {
                Serial.print("[ALERT TX #");
                Serial.print(boatPacketSeq);
                Serial.print("] Issue detected (");
                Serial.print(getWarningLabel(currentWarningLevel));
                Serial.print(")! Dist: ");
                Serial.print(currentMinimumDistance, 0);
                Serial.println("m -> Transmitted to Coast Guard.");
            }
            else
            {
                Serial.print("[TX #");
                Serial.print(boatPacketSeq);
                Serial.print("] Safe telemetry packet sent. Dist: ");
                Serial.print(currentMinimumDistance, 0);
                Serial.println("m");
            }
        }
        else
        {
            // Auto-retry radio initialization if it failed earlier
            Serial.println("[ERROR] Radio is NOT ready! Retrying nRF24 initialization...");
            radioIsReady = startBoatRadio();
            if (radioIsReady)
            {
                Serial.println("[OK] nRF24L01+ transmitter recovered! Resuming transmission...");
            }
        }
    }

    // Control local boat LEDs and buzzer
    updateWarningOutputs(currentWarningLevel);

    delay(20);
}
