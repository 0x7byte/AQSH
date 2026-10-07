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
int displayScreenIndex = 0; // 0: Coordinates & Danger, 1: Distance from border & Side

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
    lcd.print(text);
}

inline bool startDisplay()
{
    // Start I2C bus: SDA=D2 (GPIO 4), SCL=D1 (GPIO 5)
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();

    padAndPrintLcd(0, "Coast Guard BGD");
    padAndPrintLcd(1, "All Boats Safe");
    delay(1000);

    return true;
}

inline void showNoAlertScreen()
{
    padAndPrintLcd(0, "Coast Guard BGD");
    padAndPrintLcd(1, "All Boats Safe");
}

inline void showSignalLost()
{
    padAndPrintLcd(0, "Coast Guard BGD");
    padAndPrintLcd(1, "No Alert Active");
}

inline void resetDisplayToFirstScreen()
{
    displayScreenIndex = 0;
    lastDisplayRotateMillis = millis();
}

inline void showBoatData(const RadioPacket &packet, bool hasActiveAlert)
{
    // If no active issue has been received, display normal standby screen
    if (!hasActiveAlert)
    {
        showNoAlertScreen();
        return;
    }

    // Rotate between Screen 0 and Screen 1 in a loop until new data arrives
    unsigned long currentMillis = millis();
    if (currentMillis - lastDisplayRotateMillis >= SCREEN_SWITCH_INTERVAL_MS)
    {
        lastDisplayRotateMillis = currentMillis;
        displayScreenIndex = (displayScreenIndex + 1) % 2; // Loops between 0 and 1
    }

    if (displayScreenIndex == 0)
    {
        // -------------------------------------------------------------
        // Screen 1:
        // Line 1: X, Y Coordinate (Lat/Lon)
        // Line 2: Danger / Alert level
        // -------------------------------------------------------------
        String coordStr = "X:" + String(packet.latitude, 3) + " Y:" + String(packet.longitude, 2);
        padAndPrintLcd(0, coordStr);

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
        padAndPrintLcd(1, dangerStr);
    }
    else
    {
        // -------------------------------------------------------------
        // Screen 2:
        // Line 1: Distance from border
        // Line 2: Which side the boat is
        // -------------------------------------------------------------
        String distStr = "Border: " + String((int)packet.distanceMeters) + " m";
        padAndPrintLcd(0, distStr);

        String sideStr;
        if (packet.boundarySide == BOUNDARY_SIDE_BANGLADESH)
        {
            sideStr = "Side:BANGLADESH";
        }
        else if (packet.boundarySide == BOUNDARY_SIDE_ON_BOUNDARY)
        {
            sideStr = "Side: ON BORDER";
        }
        else
        {
            sideStr = "Side: OTHER SEA";
        }
        padAndPrintLcd(1, sideStr);
    }
}

#endif
