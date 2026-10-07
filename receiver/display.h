#ifndef COAST_GUARD_DISPLAY_H
#define COAST_GUARD_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "radio.h"

// 16x2 I2C LCD instance
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

unsigned long lastDisplayRotateMillis = 0;
int displayScreenIndex = 0; // 0: Coordinates (Lat/Lon), 1: Danger & Distance from border (and Side)

inline void padAndPrintLcd(uint8_t row, String text)
{
    while (text.length() < LCD_COLS)
    {
        text += " ";
    }
    if (text.length() > LCD_COLS)
    {
        text = text.substring(0, LCD_COLS);
    }
    lcd.setCursor(0, row);
    lcd.print(text.c_str());
}

inline bool startDisplay()
{
    // Start I2C bus: SDA=D2 (GPIO 4), SCL=D1 (GPIO 5)
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // Verify I2C device presence
    Wire.beginTransmission(LCD_I2C_ADDRESS);
    uint8_t error = Wire.endTransmission();
    if (error != 0)
    {
        Serial.print("[I2C WARNING] No LCD found at 0x");
        Serial.println(LCD_I2C_ADDRESS, HEX);

        Wire.beginTransmission(0x3F);
        if (Wire.endTransmission() == 0)
        {
            Serial.println("[I2C HINT] Found LCD at 0x3F! Change LCD_I2C_ADDRESS to 0x3F in receiver/config.h.");
        }
        else
        {
            Serial.println("[I2C ERROR] No I2C device found! Check SDA=D2, SCL=D1, 5V power, and GND.");
        }
    }
    else
    {
        Serial.print("[I2C OK] LCD acknowledged at 0x");
        Serial.println(LCD_I2C_ADDRESS, HEX);
    }

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();

    padAndPrintLcd(0, "AquaShield pro");
    padAndPrintLcd(1, "Waiting Boat...");
    delay(1000);

    return (error == 0);
}

inline void showWaitingScreen()
{
    padAndPrintLcd(0, "AquaShield pro");
    padAndPrintLcd(1, "Waiting Boat...");
}

inline void resetDisplayToFirstScreen()
{
    displayScreenIndex = 0;
    lastDisplayRotateMillis = millis();
}

static int lastRenderedScreen = -1;
static uint16_t lastRenderedSeq = 0xFFFF;
static uint8_t lastRenderedWarn = 0xFF;

inline void showBoatData(const RadioPacket &packet, bool packetValid, bool forceRefresh = false)
{
    if (!packetValid)
    {
        if (lastRenderedScreen != -1 || forceRefresh)
        {
            showWaitingScreen();
            lastRenderedScreen = -1;
            lastRenderedSeq = 0xFFFF;
        }
        return;
    }

    // Rotate between Screen 0 and Screen 1 every SCREEN_SWITCH_INTERVAL_MS (3.75 seconds)
    unsigned long currentMillis = millis();
    bool screenRotated = false;
    if (currentMillis - lastDisplayRotateMillis >= SCREEN_SWITCH_INTERVAL_MS)
    {
        lastDisplayRotateMillis = currentMillis;
        displayScreenIndex = (displayScreenIndex + 1) % 2; // Loops between 0 and 1
        screenRotated = true;
    }

    // Only update LCD if screen rotated, new packet arrived, or forced refresh
    if (!screenRotated && lastRenderedScreen == displayScreenIndex &&
        lastRenderedSeq == packet.seqNumber && lastRenderedWarn == packet.warningLevel && !forceRefresh)
    {
        return;
    }

    lastRenderedScreen = displayScreenIndex;
    lastRenderedSeq = packet.seqNumber;
    lastRenderedWarn = packet.warningLevel;

    if (displayScreenIndex == 0)
    {
        // -------------------------------------------------------------
        // Screen 1 (3.75s):
        // Line 1: Lat: 20.681000
        // Line 2: Lon: 92.365600
        // -------------------------------------------------------------
        String latStr = "Lat: " + String(packet.latitude, 6);
        String lonStr = "Lon: " + String(packet.longitude, 6);
        padAndPrintLcd(0, latStr);
        padAndPrintLcd(1, lonStr);
    }
    else
    {
        // -------------------------------------------------------------
        // Screen 2 (3.75s):
        // Line 1: Danger level (ALERT: DANGER! / ALERT: WARNING / STATUS: SAFE)
        // Line 2: Distance from border & Side (Dist: 850m (BD))
        // -------------------------------------------------------------
        String dangerStr;
        if (packet.warningLevel == WARNING_DANGER)
        {
            dangerStr = "ALERT: DANGER!";
        }
        else if (packet.warningLevel == WARNING_CAUTION)
        {
            dangerStr = "ALERT: WARNING";
        }
        else
        {
            dangerStr = "STATUS: SAFE";
        }
        padAndPrintLcd(0, dangerStr);

        String sideTag;
        if (packet.boundarySide == BOUNDARY_SIDE_BANGLADESH)
        {
            sideTag = "(BD)";
        }
        else if (packet.boundarySide == BOUNDARY_SIDE_ON_BOUNDARY)
        {
            sideTag = "(BORDER)";
        }
        else
        {
            sideTag = "(OTHER)";
        }

        String distStr = "Dist: " + String((int)packet.distanceMeters) + "m " + sideTag;
        if (distStr.length() > LCD_COLS)
        {
            distStr = "D:" + String((int)packet.distanceMeters) + "m " + sideTag;
        }
        padAndPrintLcd(1, distStr);
    }
}

#endif
