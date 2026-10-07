#ifndef NODEMCU_DISPLAY_LCD_H
#define NODEMCU_DISPLAY_LCD_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "distance.h"

// 16x2 I2C LCD instance
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

unsigned long lastScreenRotateMillis = 0;
int currentScreenMode = 0; // 0: Coordinates, 1: Distance & Status, 2: Boat ID & Territory

inline void padAndPrintLine(uint8_t row, String text)
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

inline bool startLcdDisplay()
{
    // Start I2C bus with configured NodeMCU pins:
    // SDA = D2 (GPIO 4)
    // SCL = D1 (GPIO 5)
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();

    padAndPrintLine(0, "AquaShield BGD");
    padAndPrintLine(1, "Init System...");
    delay(1000);

    return true;
}

inline void showWaitingForGps()
{
    padAndPrintLine(0, "AquaShield Ready");
    padAndPrintLine(1, "Waiting Boat RX.");
}

inline void showSignalLost()
{
    padAndPrintLine(0, "SIGNAL TIMEOUT! ");
    padAndPrintLine(1, "Boat RF Lost   ");
}

inline void updateLcdDisplay(const char *boatId, float latitude, float longitude,
                             float distanceMeters, int warningLevel, int boundarySide,
                             bool hasValidSignal, bool receivedFirstPacket)
{
    if (!receivedFirstPacket)
    {
        showWaitingForGps();
        return;
    }

    if (!hasValidSignal)
    {
        showSignalLost();
        return;
    }

    unsigned long currentMillis = millis();
    if (currentMillis - lastScreenRotateMillis >= SCREEN_SWITCH_INTERVAL_MS)
    {
        lastScreenRotateMillis = currentMillis;
        currentScreenMode = (currentScreenMode + 1) % 3;
    }

    if (currentScreenMode == 0)
    {
        // Screen 0: GPS Coordinates
        padAndPrintLine(0, "Lt: " + String(latitude, 6));
        padAndPrintLine(1, "Ln: " + String(longitude, 6));
    }
    else if (currentScreenMode == 1)
    {
        // Screen 1: Boundary Distance & Warning Status
        String distStr = "Dist: " + String((int)distanceMeters) + "m";
        String statusStr;
        if (warningLevel == WARNING_DANGER)
        {
            statusStr = "STAT: DANGER!";
        }
        else if (warningLevel == WARNING_CAUTION)
        {
            statusStr = "STAT: WARNING";
        }
        else
        {
            statusStr = "STAT: SAFE";
        }

        padAndPrintLine(0, distStr);
        padAndPrintLine(1, statusStr);
    }
    else
    {
        // Screen 2: Boat ID & Maritime Territory
        String boatStr = "Boat ID: " + String(boatId);
        String sideStr;
        if (boundarySide == BOUNDARY_SIDE_BANGLADESH)
        {
            sideStr = "Side:BANGLADESH";
        }
        else if (boundarySide == BOUNDARY_SIDE_ON_BOUNDARY)
        {
            sideStr = "Side: ON BORDER";
        }
        else
        {
            sideStr = "Side: OTHER SEA";
        }

        padAndPrintLine(0, boatStr);
        padAndPrintLine(1, sideStr);
    }
}

#endif

