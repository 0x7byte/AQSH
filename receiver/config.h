#ifndef COAST_GUARD_CONFIG_H
#define COAST_GUARD_CONFIG_H

#include <Arduino.h>

// ==========================================
// NodeMCU ESP8266 Pin Connections
// ==========================================

// 16x2 I2C LCD Connections
// Connect LCD VCC to 5V (VIN), GND to GND
const int I2C_SDA_PIN = 4;            // NodeMCU D2 (GPIO 4)
const int I2C_SCL_PIN = 5;            // NodeMCU D1 (GPIO 5)
const uint8_t LCD_I2C_ADDRESS = 0x27; // Change to 0x3F if 0x27 does not show characters
const uint8_t LCD_COLS = 16;
const uint8_t LCD_ROWS = 2;

// Active Buzzer Connection
// Connect Buzzer (+) to D0, (-) to GND
// D0 (GPIO 16) is chosen because it never conflicts with ESP8266 boot-mode strapping
const int BUZZER_PIN = 16;            // NodeMCU D0 (GPIO 16)

// Alert LED Connection
// Connect LED Anode (+) through a 220Ω-330Ω resistor to D3, Cathode (-) to GND
const int ALERT_LED_PIN = 0;          // NodeMCU D3 (GPIO 0)

// nRF24L01+ Radio Connections (SPI)
// Connect nRF24 VCC to 3.3V (3V3) ONLY! GND to GND
// Hardware HSPI: SCK=D5(14), MISO=D6(12), MOSI=D7(13)
const int NRF_SCK_PIN  = 14;          // NodeMCU D5 (GPIO 14)
const int NRF_MISO_PIN = 12;          // NodeMCU D6 (GPIO 12)
const int NRF_MOSI_PIN = 13;          // NodeMCU D7 (GPIO 13)
const int NRF_CSN_PIN  = 15;          // NodeMCU D8 (GPIO 15)
const int NRF_CE_PIN   = 2;           // NodeMCU D4 (GPIO 2)

// ==========================================
// nRF24L01+ Global Radio Settings (Matches Boat)
// ==========================================
const uint8_t NRF_CHANNEL = 108;      // 2508 MHz (avoids 2.4GHz WiFi interference)
const uint64_t RADIO_PIPE_ADDRESS = 0xF0F0F0F0E1LL; // 5-byte RF pipe address
const unsigned long RADIO_PACKET_MAGIC = 0x41515348UL;   // 'AQSH'
const bool NRF_ENABLE_AUTO_ACK = false;                  // Broadcast mode: eliminates "no ACK" issues

// Warning levels
#define WARNING_SAFE 0
#define WARNING_CAUTION 1
#define WARNING_DANGER 2

// Boundary territory constants
#define BOUNDARY_SIDE_BANGLADESH 0
#define BOUNDARY_SIDE_OTHER 1
#define BOUNDARY_SIDE_ON_BOUNDARY 2

// Timings
const unsigned long PACKET_TIMEOUT_MS         = 35000; // Reset alert if no new issue received for 35 seconds
const unsigned long SCREEN_SWITCH_INTERVAL_MS = 3500;  // Alternate between Screen 1 and Screen 2 every 3.5s
const unsigned long CAUTION_BEEP_INTERVAL_MS  = 400;   // 400ms ON / 400ms OFF
const unsigned long DANGER_BEEP_INTERVAL_MS   = 150;   // 150ms ON / 150ms OFF


#endif
