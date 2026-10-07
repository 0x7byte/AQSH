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

// nRF24L01+ Global Radio settings (Universal for all genuine & clone chips)
const uint8_t NRF_CHANNEL = 76;                      // 2476 MHz (Universal RF24 channel, clean & antenna resonant)
const uint8_t RADIO_PIPE_ADDRESS[6] = "AQSH1";       // 5-byte RF pipe address
const unsigned long RADIO_PACKET_MAGIC = 0x41515348UL;
const bool NRF_ENABLE_AUTO_ACK = false;              // Broadcast mode: eliminates "no ACK" issues

// Timing values used by the boat simulation and warning outputs
const unsigned long SIMULATION_INTERVAL_MS = 15000;  // New simulated GPS point every 15 seconds
const unsigned long RADIO_SEND_INTERVAL_MS = 2000;   // Telemetry broadcast interval: every 2 seconds continuous
const unsigned long CAUTION_BEEP_INTERVAL_MS = 400;  // 400ms ON / 400ms OFF (matches receiver)
const unsigned long DANGER_BEEP_INTERVAL_MS = 150;   // 150ms ON / 150ms OFF (matches receiver)

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

// Fix: With boundary segments directed south/south-west into the Bay of Bengal,
// Bangladeshi waters (Saint Martin's Island) lie on the left (positive cross product).
const bool BANGLADESH_IS_RIGHT_OF_ORDERED_BOUNDARY = false;
const float BOUNDARY_SIDE_TOLERANCE_METERS = 25.0;

#endif
