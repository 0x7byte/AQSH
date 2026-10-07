#ifndef BOAT_SIMULATION_H
#define BOAT_SIMULATION_H

#include "config.h"

struct GPSPoint
{
    float latitude;
    float longitude;
};

const GPSPoint simulatedRoute[] =
{
    {20.670400, 92.376200},
    {20.673050, 92.373550},
    {20.675700, 92.370900},
    {20.678350, 92.368250},
    {20.681000, 92.365600},
    {20.682700, 92.364067},
    {20.684400, 92.362533},
    {20.686100, 92.361000},
    {20.687167, 92.360000},
    {20.688233, 92.359000},
    {20.689300, 92.358000},
    {20.688233, 92.359000},
    {20.686100, 92.361000},
    {20.682700, 92.364067},
    {20.678350, 92.368250},
    {20.673050, 92.373550}
};

const int NUMBER_OF_SIMULATED_POINTS = sizeof(simulatedRoute) / sizeof(simulatedRoute[0]);

int simulatedPointIndex = 0;
unsigned long lastSimulationUpdateMillis = 0;
bool firstLocationRead = true;

void startLocationSource()
{
    simulatedPointIndex = 0;
    lastSimulationUpdateMillis = 0;
    firstLocationRead = true;
}

bool readLocation(GPSPoint *currentGpsPoint)
{
    unsigned long currentMillis = millis();

    if (firstLocationRead)
    {
        *currentGpsPoint = simulatedRoute[simulatedPointIndex];
        lastSimulationUpdateMillis = currentMillis;
        firstLocationRead = false;
        return true;
    }

    if (currentMillis - lastSimulationUpdateMillis >= SIMULATION_INTERVAL_MS)
    {
        lastSimulationUpdateMillis = currentMillis;
        simulatedPointIndex++;

        if (simulatedPointIndex >= NUMBER_OF_SIMULATED_POINTS)
        {
            simulatedPointIndex = 0;
        }

        *currentGpsPoint = simulatedRoute[simulatedPointIndex];
        return true;
    }

    return false;
}

#endif
