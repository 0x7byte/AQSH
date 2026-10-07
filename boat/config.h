#ifndef BOAT_CONFIG_H
#define BOAT_CONFIG_H

#include <Arduino.h>

// Boat identity sent in every radio packet
const char BOAT_ID[] = "01";

// Local map origin used when GPS coordinates are converted to metres
const float MAP_ORIGIN_LATITUDE = 20.704389;
const float MAP_ORIGIN_LONGITUDE = 92.368667;

// ESP32 connections for LEDs and active buzzer
const int GREEN_LED_PIN = 26;
const int RED_LED_PIN = 27;
const int BUZZER_PIN = 25;

// ESP32 VSPI connections for nRF24L01+ module
const int NRF_SCK_PIN  = 18;
const int NRF_MISO_PIN = 19;
const int NRF_MOSI_PIN = 23;
const int NRF_CSN_PIN  = 5;
const int NRF_CE_PIN   = 4;

// nRF24L01+ radio settings (2.4 GHz)
const uint8_t NRF_CHANNEL = 108;                     // 2508 MHz (avoids 2.4GHz WiFi channels)
const uint64_t RADIO_PIPE_ADDRESS = 0xF0F0F0F0E1LL; // 5-byte address pipe
const unsigned long RADIO_PACKET_MAGIC = 0x41515348UL;

// Timing values used by the boat simulation and warning outputs
const unsigned long SIMULATION_INTERVAL_MS = 15000; // New simulated GPS point every 15 seconds
const unsigned long RADIO_SEND_INTERVAL_MS = 15000; // Transmit evaluation interval
const unsigned long SLOW_WARNING_INTERVAL_MS = 500;
const unsigned long FAST_WARNING_INTERVAL_MS = 150;


// Warning distances in metres
const float WARNING_DISTANCE_METERS = 2000.0;
const float DANGER_DISTANCE_METERS = 1000.0;

// Warning levels
#define WARNING_SAFE 0
#define WARNING_CAUTION 1
#define WARNING_DANGER 2

// Maritime boundary constants
#define BOUNDARY_SIDE_BANGLADESH 0
#define BOUNDARY_SIDE_OTHER 1
#define BOUNDARY_SIDE_ON_BOUNDARY 2
const bool BANGLADESH_IS_RIGHT_OF_ORDERED_BOUNDARY = true;
const float BOUNDARY_SIDE_TOLERANCE_METERS = 25.0;

#endif
