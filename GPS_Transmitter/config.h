#ifndef TRANSMITTER_CONFIG_H
#define TRANSMITTER_CONFIG_H

#include <Arduino.h>

// ==========================================
// Board Selection & Pinout for Transmitter
// ==========================================
// Select your transmitting board pins:
//
// For Arduino Uno / Nano:
//   CE  = Pin 9
//   CSN = Pin 10
//   SCK = Pin 13, MISO = Pin 12, MOSI = Pin 11
//
// For ESP8266 (NodeMCU / D1 mini):
//   CE  = D4 (GPIO 2)
//   CSN = D8 (GPIO 15)
//   SCK = D5, MISO = D6, MOSI = D7
//
// For ESP32:
//   CE  = GPIO 4
//   CSN = GPIO 5
//   SCK = GPIO 18, MISO = GPIO 19, MOSI = GPIO 23

#if defined(ESP8266)
const int NRF_CE_PIN  = 2;   // D4
const int NRF_CSN_PIN = 15;  // D8
#elif defined(ESP32)
const int NRF_CE_PIN  = 4;
const int NRF_CSN_PIN = 5;
#else
// Default for Arduino Uno / Nano / Pro Mini
const int NRF_CE_PIN  = 9;
const int NRF_CSN_PIN = 10;
#endif

// ==========================================
// nRF24L01+ Radio Settings (Must match Receiver)
// ==========================================
const uint8_t NRF_CHANNEL = 108;      // 2508 MHz
const uint64_t RADIO_PIPE_ADDRESS = 0xF0F0F0F0E1LL;
const uint32_t RADIO_PACKET_MAGIC = 0x41515348UL;   // 'AQSH'

// Transmission interval in milliseconds
const unsigned long TRANSMIT_INTERVAL_MS = 3000; // Send new simulated GPS point every 3 seconds

#endif

