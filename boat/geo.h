#ifndef BOAT_GEO_H
#define BOAT_GEO_H

#include <math.h>
#include "config.h"

const float EARTH_RADIUS_METERS = 6371000.0;
const float PI_VALUE = 3.14159265;

float degreesToRadians(float degrees)
{
    return degrees * PI_VALUE / 180.0;
}

float convertLongitudeToLocalX(float longitude)
{
    float longitudeDifference = degreesToRadians(longitude - MAP_ORIGIN_LONGITUDE);
    float originLatitudeRadians = degreesToRadians(MAP_ORIGIN_LATITUDE);

    return longitudeDifference * cos(originLatitudeRadians) * EARTH_RADIUS_METERS;
}

float convertLatitudeToLocalY(float latitude)
{
    float latitudeDifference = degreesToRadians(latitude - MAP_ORIGIN_LATITUDE);

    return latitudeDifference * EARTH_RADIUS_METERS;
}

#endif
