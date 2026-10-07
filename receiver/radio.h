#ifndef COAST_GUARD_RADIO_H
#define COAST_GUARD_RADIO_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include "config.h"

// Radio packet format shared with the Boat transmitter
struct __attribute__((packed)) RadioPacket
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
    pinMode(NRF_CE_PIN, OUTPUT);
    pinMode(NRF_CSN_PIN, OUTPUT);
    digitalWrite(NRF_CSN_PIN, HIGH);
    digitalWrite(NRF_CE_PIN, LOW);

    SPI.begin();
    delay(20);

    if (!radio.begin())
    {
        return false;
    }

    radio.setChannel(NRF_CHANNEL);
    radio.setDataRate(RF24_1MBPS);           // 1Mbps universal speed
    radio.setPALevel(RF24_PA_HIGH);          // High power reception
    radio.setAutoAck(NRF_ENABLE_AUTO_ACK);   // Match broadcast mode
    radio.openReadingPipe(1, RADIO_PIPE_ADDRESS);
    radio.startListening();                  // Set as receiver - ALWAYS LISTENING

    return true;
}

inline bool receiveBoatData(RadioPacket *receivedPacket)
{
    bool packetFound = false;

    // Read all available packets from FIFO so newest is processed
    while (radio.available())
    {
        RadioPacket tempPacket;
        radio.read(&tempPacket, sizeof(RadioPacket));

        // Validate packet identifier
        if (tempPacket.magic == RADIO_PACKET_MAGIC)
        {
            tempPacket.boatId[sizeof(tempPacket.boatId) - 1] = '\0';
            *receivedPacket = tempPacket;
            packetFound = true;
        }
    }

    return packetFound;
}

#endif
