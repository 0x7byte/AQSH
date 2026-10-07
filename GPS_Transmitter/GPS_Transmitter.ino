#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include "config.h"
#include "simulation.h"

// GPS Packet struct matching the Receiver
struct GPSPacket
{
    uint32_t magic;      // 0x41515348 ('AQSH')
    uint16_t seqNumber;  // Sequence counter
    float latitude;      // GPS latitude
    float longitude;     // GPS longitude
};

RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
uint16_t packetSequence = 0;

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println("==================================================");
    Serial.println("   AquaShield - Simulated GPS Transmitter         ");
    Serial.println("   Hardware: nRF24L01+ Transmitter (Channel 108)  ");
    Serial.println("==================================================");

#if defined(LED_BUILTIN)
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
#endif

    SPI.begin();

    if (radio.begin())
    {
        radio.setChannel(NRF_CHANNEL);
        radio.setDataRate(RF24_250KBPS);
        radio.setPALevel(RF24_PA_LOW);
        radio.openWritingPipe(RADIO_PIPE_ADDRESS);
        radio.stopListening(); // Set as transmitter

        Serial.println("[OK] nRF24L01+ Transmitter initialized successfully!");
        Serial.print("     RF Channel: ");
        Serial.println(NRF_CHANNEL);
        Serial.print("     Interval:   ");
        Serial.print(TRANSMIT_INTERVAL_MS / 1000);
        Serial.println(" seconds per GPS point");
    }
    else
    {
        Serial.println("[ERROR] nRF24L01+ NOT found! Check 3.3V power, SPI wiring, and capacitor.");
    }

    startLocationSource();
}

void loop()
{
    GPSPoint currentPoint;
    int pointIndex = 0;

    if (readLocation(&currentPoint, &pointIndex))
    {
        packetSequence++;

        GPSPacket packet;
        packet.magic = RADIO_PACKET_MAGIC;
        packet.seqNumber = packetSequence;
        packet.latitude = currentPoint.latitude;
        packet.longitude = currentPoint.longitude;

#if defined(LED_BUILTIN)
        digitalWrite(LED_BUILTIN, HIGH);
#endif

        bool success = radio.write(&packet, sizeof(GPSPacket));

#if defined(LED_BUILTIN)
        digitalWrite(LED_BUILTIN, LOW);
#endif

        Serial.print("[TX #");
        Serial.print(packetSequence);
        Serial.print("] Route Point ");
        Serial.print(pointIndex + 1);
        Serial.print("/");
        Serial.print(NUMBER_OF_SIMULATED_POINTS);
        Serial.print(" | Lat: ");
        Serial.print(currentPoint.latitude, 6);
        Serial.print(" | Lon: ");
        Serial.print(currentPoint.longitude, 6);

        if (success)
        {
            Serial.println(" -> [OK Sent]");
        }
        else
        {
            Serial.println(" -> [FAIL / No ACK]");
        }
    }

    delay(20);
}

