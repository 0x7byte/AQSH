#include <Arduino.h>
#include "config.h"
#include "radio.h"
#include "display.h"

RadioPacket latestRadioPacket;
bool displayIsReady = false;
bool radioIsReady = false;
bool hasActiveAlert = false;
unsigned long lastPacketReceivedMillis = 0;

// Non-blocking buzzer state
unsigned long lastBuzzerToggleMillis = 0;
bool buzzerIsOn = false;

void updateCoastGuardBuzzer(uint8_t warningLevel, bool alertActive)
{
    // Silent if no active issue or boat is in safe waters
    if (!alertActive || warningLevel == WARNING_SAFE)
    {
        digitalWrite(BUZZER_PIN, LOW);
        buzzerIsOn = false;
        return;
    }

    unsigned long currentMillis = millis();

    if (warningLevel == WARNING_DANGER)
    {
        // Rapid urgent alarm: 150ms ON / 150ms OFF
        if (currentMillis - lastBuzzerToggleMillis >= DANGER_BEEP_INTERVAL_MS)
        {
            lastBuzzerToggleMillis = currentMillis;
            buzzerIsOn = !buzzerIsOn;
            digitalWrite(BUZZER_PIN, buzzerIsOn ? HIGH : LOW);
        }
    }
    else if (warningLevel == WARNING_CAUTION)
    {
        // Intermittent caution beep: 400ms ON / 400ms OFF
        if (currentMillis - lastBuzzerToggleMillis >= CAUTION_BEEP_INTERVAL_MS)
        {
            lastBuzzerToggleMillis = currentMillis;
            buzzerIsOn = !buzzerIsOn;
            digitalWrite(BUZZER_PIN, buzzerIsOn ? HIGH : LOW);
        }
    }
}

void setup()
{
    // Initialize active buzzer on D0 (GPIO 16)
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // Short boot chirp
    digitalWrite(BUZZER_PIN, HIGH);
    delay(80);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println("==================================================");
    Serial.println("   AquaShield - Coast Guard Base Station          ");
    Serial.println("   NodeMCU ESP8266 + nRF24L01 + 16x2 I2C LCD     ");
    Serial.println("==================================================");

    displayIsReady = startDisplay();
    radioIsReady = startCoastGuardRadio();

    if (displayIsReady)
    {
        Serial.println("[OK] 16x2 I2C LCD ready (Address 0x27)");
    }
    else
    {
        Serial.println("[ERROR] 16x2 I2C LCD not detected!");
    }

    if (radioIsReady)
    {
        Serial.println("[OK] nRF24L01+ receiver ready on Channel 108");
    }
    else
    {
        Serial.println("[ERROR] nRF24L01+ receiver not found! Check 3.3V, SPI wiring, and capacitor.");
    }
}

void loop()
{
    // Check if a new issue packet arrived from the boat
    if (radioIsReady && receiveBoatData(&latestRadioPacket))
    {
        lastPacketReceivedMillis = millis();
        hasActiveAlert = (latestRadioPacket.warningLevel != WARNING_SAFE);

        // Reset rotation so fresh incoming data starts on Screen 1 immediately
        resetDisplayToFirstScreen();

        Serial.print("[ALERT RX #");
        Serial.print(latestRadioPacket.seqNumber);
        Serial.print("] Boat: ");
        Serial.print(latestRadioPacket.boatId);
        Serial.print(" | Lat: ");
        Serial.print(latestRadioPacket.latitude, 6);
        Serial.print(" | Lon: ");
        Serial.print(latestRadioPacket.longitude, 6);
        Serial.print(" | Dist: ");
        Serial.print(latestRadioPacket.distanceMeters, 0);
        Serial.print("m | Status: ");
        Serial.print(getReceivedWarningLabel(latestRadioPacket.warningLevel));
        Serial.print(" | Side: ");
        Serial.println(getReceivedBoundarySideLabel(latestRadioPacket.boundarySide));
    }

    // If no alert packet is received for 35s, clear the alert
    if (hasActiveAlert && (millis() - lastPacketReceivedMillis > PACKET_TIMEOUT_MS))
    {
        hasActiveAlert = false;
        latestRadioPacket.warningLevel = WARNING_SAFE;
        Serial.println("[INFO] Alert cleared - boat is either safe or out of range.");
    }

    // Sound buzzer if active issue
    updateCoastGuardBuzzer(latestRadioPacket.warningLevel, hasActiveAlert);

    // Update 16x2 LCD display (loops Screen 1 and Screen 2 every 3.5s)
    if (displayIsReady)
    {
        showBoatData(latestRadioPacket, hasActiveAlert);
    }

    delay(20);
}
