#ifndef BOAT_RADIO_H
#define BOAT_RADIO_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <string.h>
#include "config.h"

// Radio packet shared between ESP32 Boat and NodeMCU ESP8266
struct __attribute__((packed)) RadioPacket
{
    uint32_t magic;         // 0x41515348 ('AQSH')
    char boatId[4];         // "01"
    float latitude;         // GPS latitude
    float longitude;        // GPS longitude
    float distanceMeters;   // Distance in metres to boundary
    uint8_t warningLevel;   // 0: SAFE, 1: WARNING, 2: DANGER
    uint8_t boundarySide;   // 0: BANGLADESH, 1: OTHER, 2: ON_BOUNDARY
    uint16_t seqNumber;     // Packet sequence counter
};

RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
uint16_t boatPacketSeq = 0;

inline bool startBoatRadio()
{
    // Explicitly configure CE and CSN as GPIO pins
    pinMode(NRF_CE_PIN, OUTPUT);
    pinMode(NRF_CSN_PIN, OUTPUT);
    digitalWrite(NRF_CSN_PIN, HIGH);
    digitalWrite(NRF_CE_PIN, LOW);

    // Initialize SPI on ESP32 VSPI pins (SCK, MISO, MOSI)
    // NOTE: Do NOT pass NRF_CSN_PIN to SPI.begin on ESP32,
    // otherwise hardware SPI locks CSN and RF24 library cannot toggle it!
    SPI.begin(NRF_SCK_PIN, NRF_MISO_PIN, NRF_MOSI_PIN);

    delay(20);

    if (!radio.begin())
    {
        return false;
    }

    radio.setChannel(NRF_CHANNEL);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);
    radio.openWritingPipe(RADIO_PIPE_ADDRESS);
    radio.stopListening(); // Transmit mode

    return true;
}

inline bool sendBoatData(float latitude, float longitude, float minimumDistance, int warningLevel,
                         int boundarySide)
{
    boatPacketSeq++;
    RadioPacket radioPacket;
    memset(&radioPacket, 0, sizeof(RadioPacket));

    radioPacket.magic = RADIO_PACKET_MAGIC;
    strncpy(radioPacket.boatId, BOAT_ID, sizeof(radioPacket.boatId) - 1);
    radioPacket.latitude = latitude;
    radioPacket.longitude = longitude;
    radioPacket.distanceMeters = minimumDistance;
    radioPacket.warningLevel = (uint8_t)warningLevel;
    radioPacket.boundarySide = (uint8_t)boundarySide;
    radioPacket.seqNumber = boatPacketSeq;

    return radio.write(&radioPacket, sizeof(RadioPacket));
}

#endif
