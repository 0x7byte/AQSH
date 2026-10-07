#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include "radio.h"
#include "display.h"

RadioPacket latestRadioPacket;
bool displayIsReady = false;
bool radioIsReady = false;
bool hasActiveAlert = false;
unsigned long lastPacketReceivedMillis = 0;
unsigned long lastRadioRetryMillis = 0;

// Non-blocking buzzer state
unsigned long lastBuzzerToggleMillis = 0;
bool buzzerIsOn = false;

inline void setBuzzerState(bool turnOn)
{
    if (BUZZER_ACTIVE_LOW)
    {
        digitalWrite(BUZZER_PIN, turnOn ? LOW : HIGH);
    }
    else
    {
        digitalWrite(BUZZER_PIN, turnOn ? HIGH : LOW);
    }
}

inline void setAlertLedState(bool turnOn)
{
    digitalWrite(ALERT_LED_PIN, turnOn ? HIGH : LOW);
}

void updateCoastGuardBuzzer(uint8_t warningLevel, bool alertActive)
{
    // Silent and LED OFF if no active issue or boat is in safe waters
    if (!alertActive || warningLevel == WARNING_SAFE)
    {
        setBuzzerState(false);
        setAlertLedState(false);
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
            setBuzzerState(buzzerIsOn);
            setAlertLedState(buzzerIsOn);
        }
    }
    else if (warningLevel == WARNING_CAUTION)
    {
        // Intermittent caution beep: 400ms ON / 400ms OFF
        if (currentMillis - lastBuzzerToggleMillis >= CAUTION_BEEP_INTERVAL_MS)
        {
            lastBuzzerToggleMillis = currentMillis;
            buzzerIsOn = !buzzerIsOn;
            setBuzzerState(buzzerIsOn);
            setAlertLedState(buzzerIsOn);
        }
    }
}

void setup()
{
    // Turn off ESP8266 WiFi completely to stop 2.4 GHz RF interference with nRF24L01+
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();

    // Initialize active buzzer on D4 (GPIO 2) and Alert LED on D3 (GPIO 0)
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(ALERT_LED_PIN, OUTPUT);
    setBuzzerState(false);
    setAlertLedState(false);

    // Short boot chirp & LED flash
    setBuzzerState(true);
    setAlertLedState(true);
    delay(80);
    setBuzzerState(false);
    setAlertLedState(false);

    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println("==================================================");
    Serial.println("   AquaShield - Coast Guard Base Station          ");
    Serial.println("   NodeMCU ESP8266 + nRF24L01 (ALWAYS ACTIVE)    ");
    Serial.println("==================================================");

    displayIsReady = startDisplay();
    radioIsReady = startCoastGuardRadio();

    if (displayIsReady)
    {
        Serial.println("[OK] 16x2 I2C LCD ready");
    }
    else
    {
        Serial.println("[ERROR] 16x2 I2C LCD not detected!");
    }

    if (radioIsReady)
    {
        Serial.println("[OK] nRF24L01+ receiver ready on Channel 108 (ALWAYS LISTENING)");
    }
    else
    {
        Serial.println("[ERROR] nRF24L01+ receiver not found! Check 3.3V, SPI wiring, and capacitor.");
    }
}

void loop()
{
    // Auto-reconnect nRF24 if disconnected or failed on boot
    if (!radioIsReady)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - lastRadioRetryMillis >= 2000)
        {
            lastRadioRetryMillis = currentMillis;
            Serial.println("[RETRY] Initializing nRF24L01+ receiver...");
            radioIsReady = startCoastGuardRadio();
            if (radioIsReady)
            {
                Serial.println("[OK] nRF24L01+ receiver recovered and now ALWAYS LISTENING!");
            }
        }
    }

    bool freshPacketReceived = false;

    // Check continuously if an issue packet arrived from the boat
    if (radioIsReady && receiveBoatData(&latestRadioPacket))
    {
        freshPacketReceived = true;
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
        freshPacketReceived = true;
        Serial.println("[INFO] Alert cleared - boat is either safe or out of range.");
    }

    // Sound buzzer if active issue
    updateCoastGuardBuzzer(latestRadioPacket.warningLevel, hasActiveAlert);

    // Update 16x2 LCD display (loops Screen 1 and Screen 2 every 3.5s)
    if (displayIsReady)
    {
        showBoatData(latestRadioPacket, hasActiveAlert, freshPacketReceived);
    }

    // High speed non-blocking loop: keeps nRF24 constantly polling at maximum rate
    yield();
    delay(2);
}
