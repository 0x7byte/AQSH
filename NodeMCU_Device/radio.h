#ifndef COAST_GUARD_RADIO_H
#define COAST_GUARD_RADIO_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include "config.h"

// Radio packet format shared with the Boat transmitter
struct RadioPacket
{
    uint32_t magic;         // 0x41515348 ('AQSH')
    char boatId[4];         // "01"
    float latitude;         // GPS latitude
    float longitude;        // GPS longitude
    float distanceMeters;   // Shortest boundary distance in metres
    uint8_t warningLevel;   // 0: SAFE, 1: WARNING, 2: DANGER
    uint8_t boundarySide;   // 0: BANGLADESH, 1: OTHER, 2: ON_BOUNDARY
    uint16_t seqNumber;     // Packet sequence counter
};

RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);

inline const char *getReceivedWarningLabel(uint8_t warningLevel)
{
    if (warningLevel == WARNING_DANGER)
    {
        return "DANGER";
    }
    else if (warningLevel == WARNING_CAUTION)
    {
        return "WARNING";
    }

    return "SAFE";
}

inline const char *getReceivedBoundarySideLabel(uint8_t boundarySide)
{
    if (boundarySide == BOUNDARY_SIDE_BANGLADESH)
    {
        return "BANGLADESH";
    }
    else if (boundarySide == BOUNDARY_SIDE_ON_BOUNDARY)
    {
        return "ON BOUNDARY";
    }

    return "OTHER";
}

inline bool startCoastGuardRadio()
{
    // NodeMCU Hardware SPI: SCK=D5(14), MISO=D6(12), MOSI=D7(13)
    SPI.begin();

    if (!radio.begin())
    {
        return false;
    }

    radio.setChannel(NRF_CHANNEL);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);
    radio.openReadingPipe(1, RADIO_PIPE_ADDRESS);
    radio.startListening(); // Set as receiver

    return true;
}

inline bool receiveBoatData(RadioPacket *receivedPacket)
{
    if (radio.available())
    {
        radio.read(receivedPacket, sizeof(RadioPacket));

        // Validate packet identifier
        if (receivedPacket->magic == RADIO_PACKET_MAGIC)
        {
            receivedPacket->boatId[sizeof(receivedPacket->boatId) - 1] = '\0';
            return true;
        }
    }

    return false;
}

#endif
