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
    SPI.begin(NRF_SCK_PIN, NRF_MISO_PIN, NRF_MOSI_PIN);

    // Give power rail and crystal oscillator time to stabilize
    delay(100);

    bool radioReady = false;
    for (int attempt = 1; attempt <= 5; attempt++)
    {
        digitalWrite(NRF_CSN_PIN, HIGH);
        digitalWrite(NRF_CE_PIN, LOW);
        delay(20);

        if (radio.begin())
        {
            if (radio.isChipConnected())
            {
                radioReady = true;
                break;
            }
        }
        delay(50);
    }

    if (!radioReady)
    {
        return false;
    }

    radio.setChannel(NRF_CHANNEL);
    radio.setDataRate(RF24_1MBPS);           // 1Mbps is universal for genuine and clone nRF24 chips
    radio.setPALevel(RF24_PA_HIGH);          // High power for strong signal
    radio.setAutoAck(NRF_ENABLE_AUTO_ACK);   // Broadcast mode - guarantees instant transmit without waiting for ACK
    radio.openWritingPipe(RADIO_PIPE_ADDRESS);
    radio.stopListening();                   // Transmit mode

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

    // Send 3 quick bursts (15ms apart) so the receiver never misses a packet
    bool sent = false;
    for (int i = 0; i < 3; i++)
    {
        if (radio.write(&radioPacket, sizeof(RadioPacket)))
        {
            sent = true;
        }
        delay(15);
    }

    return sent;
}

#endif
