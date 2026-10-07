#include <Arduino.h>
#include <Wire.h>

// NodeMCU I2C pins:
// SDA = D2 (GPIO 4)
// SCL = D1 (GPIO 5)
const int SDA_PIN = 4;
const int SCL_PIN = 5;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- NodeMCU ESP8266 I2C Scanner ---");
    Wire.begin(SDA_PIN, SCL_PIN);
}

void loop()
{
    byte error, address;
    int nDevices = 0;

    Serial.println("Scanning I2C bus...");

    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("  [FOUND] I2C device at address 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            if (address == 0x27 || address == 0x3F)
            {
                Serial.print("  <-- Typically 16x2 I2C LCD!");
            }
            Serial.println();
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("  [ERROR] Unknown error at address 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
        }
    }

    if (nDevices == 0)
    {
        Serial.println("  No I2C devices found. Check wiring (SDA->D2, SCL->D1, VCC->5V/VIN, GND->GND)");
    }
    else
    {
        Serial.println("Done.\n");
    }

    delay(5000);
}

