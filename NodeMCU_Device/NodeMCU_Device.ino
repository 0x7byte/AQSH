#include <Arduino.h>
#include "config.h"
#include "segments.h"
#include "geo.h"
#include "distance.h"
#include "display_lcd.h"
#include "warning_buzzer.h"
#include "radio_nrf.h"

// Global state
RadioPacket receivedPacket;
char currentBoatId[8] = "01";
float currentLatitude = 0.0;
float currentLongitude = 0.0;
float currentMinimumDistance = 0.0;
int currentWarningLevel = WARNING_SAFE;
int currentBoundarySide = BOUNDARY_SIDE_OTHER;

bool hasReceivedFirstPacket = false;
bool hasValidSignal = false;
unsigned long lastPacketReceivedMillis = 0;

void processReceivedPacket(const RadioPacket &packet)
{
    // Copy boat ID safely
    memset(currentBoatId, 0, sizeof(currentBoatId));
    strncpy(currentBoatId, packet.boatId, sizeof(currentBoatId) - 1);

    currentLatitude = packet.latitude;
    currentLongitude = packet.longitude;

    // If the transmitter already calculated distance and warning, use them
    // Otherwise calculate locally from segments.h
    if (packet.distanceMeters > 0.0f)
    {
        currentMinimumDistance = packet.distanceMeters;
        currentWarningLevel = packet.warningLevel;
        currentBoundarySide = packet.boundarySide;
    }
    else
    {
        float localX = convertLongitudeToLocalX(currentLongitude);
        float localY = convertLatitudeToLocalY(currentLatitude);
        BoundaryMeasurement measurement = measureBoundary(localX, localY);
        currentMinimumDistance = measurement.distanceMeters;
        currentBoundarySide = determineBoundarySide(measurement);
        currentWarningLevel = determineWarningLevel(currentMinimumDistance);
    }

    // Print to Serial Monitor
    Serial.print("[RX #");
    Serial.print(packet.seqNumber);
    Serial.print("] Boat: ");
    Serial.print(currentBoatId);
    Serial.print(" | Lat: ");
    Serial.print(currentLatitude, 6);
    Serial.print(" | Lon: ");
    Serial.print(currentLongitude, 6);
    Serial.print(" | Dist: ");
    Serial.print(currentMinimumDistance, 0);
    Serial.print("m | Status: ");
    Serial.print(getWarningLabel(currentWarningLevel));
    Serial.print(" | Side: ");
    Serial.println(getBoundarySideLabel(currentBoundarySide));
}

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println("==================================================");
    Serial.println("   AquaShield - Maritime Monitoring Station       ");
    Serial.println("   Hardware: NodeMCU ESP8266 + nRF24L01 + 16x2 LCD");
    Serial.println("==================================================");

    // 1. Initialize Active Buzzer on D0
    startBuzzer();
    Serial.println("[OK] Active Buzzer initialized on D0 (GPIO 16)");

    // 2. Initialize 16x2 I2C LCD on D2 (SDA) / D1 (SCL)
    if (startLcdDisplay())
    {
        Serial.println("[OK] 16x2 I2C LCD initialized at address 0x27");
    }

    // 3. Initialize nRF24L01+ Radio Receiver on HSPI (D5/D6/D7) + D4(CE) + D8(CSN)
    if (startNodeMCURadio())
    {
        Serial.println("[OK] nRF24L01+ Receiver ready (Channel 108, 250kbps)");
        padAndPrintLine(0, "AquaShield Ready");
        padAndPrintLine(1, "Waiting Boat RX.");
    }
    else
    {
        Serial.println("[ERROR] nRF24L01+ NOT detected! Check 3.3V power, SPI pins, and capacitor.");
        padAndPrintLine(0, "nRF24 NOT FOUND!");
        padAndPrintLine(1, "Check 3V3/Wiring");
    }
}

void loop()
{
    // Check if a radio packet arrived from the Boat
    if (checkRadioPacket(&receivedPacket))
    {
        lastPacketReceivedMillis = millis();
        hasReceivedFirstPacket = true;
        hasValidSignal = true;

        processReceivedPacket(receivedPacket);
    }

    // Check for RF signal timeout
    if (hasReceivedFirstPacket && (millis() - lastPacketReceivedMillis > PACKET_TIMEOUT_MS))
    {
        if (hasValidSignal)
        {
            hasValidSignal = false;
            Serial.println("[WARNING] Boat RF Signal Lost! Waiting for reconnect...");
        }
    }

    // Non-blocking buzzer alarm: Silent in SAFE, beeping in WARNING, rapid pulsing in DANGER
    updateBuzzer(currentWarningLevel, hasValidSignal);

    // Multi-screen display update for 16x2 LCD
    updateLcdDisplay(currentBoatId, currentLatitude, currentLongitude, currentMinimumDistance,
                     currentWarningLevel, currentBoundarySide, hasValidSignal,
                     hasReceivedFirstPacket);

    delay(20);
}

