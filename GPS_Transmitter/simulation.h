#ifndef TRANSMITTER_SIMULATION_H
#define TRANSMITTER_SIMULATION_H

#include <Arduino.h>
#include "config.h"

struct GPSPoint
{
    float latitude;
    float longitude;
};

// 16-point simulated boat route near St. Martin's Island / Bangladesh maritime boundary
// Starts SAFE (> 2000m), moves to WARNING (1000m - 2000m), enters DANGER (< 1000m), then returns to SAFE.
const GPSPoint simulatedRoute[] =
{
    {20.670400, 92.376200}, // Point 0:  SAFE (~2245m)
    {20.673050, 92.373550}, // Point 1:  SAFE (~2236m)
    {20.675700, 92.370900}, // Point 2:  SAFE (~2227m)
    {20.678350, 92.368250}, // Point 3:  SAFE (~2028m)
    {20.681000, 92.365600}, // Point 4:  WARNING (~1625m)
    {20.682700, 92.364067}, // Point 5:  WARNING (~1379m)
    {20.684400, 92.362533}, // Point 6:  WARNING (~1132m)
    {20.686100, 92.361000}, // Point 7:  DANGER (~886m)
    {20.687167, 92.360000}, // Point 8:  DANGER (~728m)
    {20.688233, 92.359000}, // Point 9:  DANGER (~571m)
    {20.689300, 92.358000}, // Point 10: DANGER (~413m - closest point)
    {20.688233, 92.359000}, // Point 11: DANGER (~571m)
    {20.686100, 92.361000}, // Point 12: DANGER (~886m)
    {20.682700, 92.364067}, // Point 13: WARNING (~1379m)
    {20.678350, 92.368250}, // Point 14: SAFE (~2028m)
    {20.673050, 92.373550}  // Point 15: SAFE (~2236m)
};

const int NUMBER_OF_SIMULATED_POINTS = sizeof(simulatedRoute) / sizeof(simulatedRoute[0]);

int simulatedPointIndex = 0;
unsigned long lastSimulationUpdateMillis = 0;

inline void startLocationSource()
{
    simulatedPointIndex = 0;
    lastSimulationUpdateMillis = 0;
}

inline bool readLocation(GPSPoint *currentGpsPoint, int *pointIdx)
{
    unsigned long currentMillis = millis();

    if (lastSimulationUpdateMillis == 0 || (currentMillis - lastSimulationUpdateMillis >= TRANSMIT_INTERVAL_MS))
    {
        lastSimulationUpdateMillis = currentMillis;
        *currentGpsPoint = simulatedRoute[simulatedPointIndex];
        *pointIdx = simulatedPointIndex;
        simulatedPointIndex = (simulatedPointIndex + 1) % NUMBER_OF_SIMULATED_POINTS;
        return true;
    }

    return false;
}

#endif

