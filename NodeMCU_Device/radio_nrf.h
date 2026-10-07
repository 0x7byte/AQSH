#ifndef NODEMCU_RADIO_NRF_H
#define NODEMCU_RADIO_NRF_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include "config.h"

// Radio packet shared between ESP32 Boat and NodeMCU ESP8266
struct RadioPacket
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

// nRF24L01 radio instance
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);

inline bool startNodeMCURadio()
{
    SPI.begin();

    if (!radio.begin())
    {
        return false;
    }

    radio.setChannel(NRF_CHANNEL);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);
    radio.openReadingPipe(1, RADIO_PIPE_ADDRESS);
    radio.startListening();

    return true;
}

inline bool checkRadioPacket(RadioPacket *receivedPacket)
{
    if (radio.available())
    {
        radio.read(receivedPacket, sizeof(RadioPacket));
        if (receivedPacket->magic == RADIO_PACKET_MAGIC)
        {
            return true;
        }
    }
    return false;
}

#endif

