#ifndef NODEMCU_DISTANCE_H
#define NODEMCU_DISTANCE_H

#include <math.h>
#include "segments.h"
#include "config.h"

struct BoundaryMeasurement
{
    float distanceMeters;
    int nearestSegmentIndex;
    float signedCrossProduct;
};

inline float calculateDistanceBetweenPoints(float firstX, float firstY, float secondX, float secondY)
{
    float xDifference = firstX - secondX;
    float yDifference = firstY - secondY;

    return sqrt((xDifference * xDifference) + (yDifference * yDifference));
}

inline float calculateDistanceToSegment(float pointX, float pointY, Segment boundarySegment)
{
    float segmentX = boundarySegment.x2 - boundarySegment.x1;
    float segmentY = boundarySegment.y2 - boundarySegment.y1;
    float segmentLengthSquared = (segmentX * segmentX) + (segmentY * segmentY);

    if (segmentLengthSquared == 0.0)
    {
        return calculateDistanceBetweenPoints(pointX, pointY, boundarySegment.x1, boundarySegment.y1);
    }

    float projection = ((pointX - boundarySegment.x1) * segmentX +
                        (pointY - boundarySegment.y1) * segmentY) / segmentLengthSquared;

    if (projection < 0.0)
    {
        projection = 0.0;
    }
    else if (projection > 1.0)
    {
        projection = 1.0;
    }

    float nearestX = boundarySegment.x1 + (projection * segmentX);
    float nearestY = boundarySegment.y1 + (projection * segmentY);

    return calculateDistanceBetweenPoints(pointX, pointY, nearestX, nearestY);
}

inline float calculateSignedCrossProduct(float pointX, float pointY, Segment boundarySegment)
{
    float segmentX = boundarySegment.x2 - boundarySegment.x1;
    float segmentY = boundarySegment.y2 - boundarySegment.y1;

    return (segmentX * (pointY - boundarySegment.y1)) -
           (segmentY * (pointX - boundarySegment.x1));
}

inline BoundaryMeasurement measureBoundary(float boatX, float boatY)
{
    BoundaryMeasurement measurement;
    measurement.distanceMeters = calculateDistanceToSegment(boatX, boatY, boundarySegments[0]);
    measurement.nearestSegmentIndex = 0;
    measurement.signedCrossProduct = calculateSignedCrossProduct(boatX, boatY, boundarySegments[0]);

    for (int segmentIndex = 1; segmentIndex < NUMBER_OF_BOUNDARY_SEGMENTS; segmentIndex++)
    {
        float segmentDistance = calculateDistanceToSegment(boatX, boatY, boundarySegments[segmentIndex]);

        if (segmentDistance < measurement.distanceMeters)
        {
            measurement.distanceMeters = segmentDistance;
            measurement.nearestSegmentIndex = segmentIndex;
            measurement.signedCrossProduct = calculateSignedCrossProduct(boatX, boatY,
                                                                          boundarySegments[segmentIndex]);
        }
    }

    return measurement;
}

inline int determineWarningLevel(float minimumDistance)
{
    if (minimumDistance <= DANGER_DISTANCE_METERS)
    {
        return WARNING_DANGER;
    }
    else if (minimumDistance <= WARNING_DISTANCE_METERS)
    {
        return WARNING_CAUTION;
    }

    return WARNING_SAFE;
}

inline const char *getWarningLabel(int warningLevel)
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

inline int determineBoundarySide(BoundaryMeasurement measurement)
{
    if (measurement.distanceMeters <= BOUNDARY_SIDE_TOLERANCE_METERS)
    {
        return BOUNDARY_SIDE_ON_BOUNDARY;
    }

    bool boatIsOnRightSide = measurement.signedCrossProduct < 0.0;

    if (boatIsOnRightSide == BANGLADESH_IS_RIGHT_OF_ORDERED_BOUNDARY)
    {
        return BOUNDARY_SIDE_BANGLADESH;
    }

    return BOUNDARY_SIDE_OTHER;
}

inline const char *getBoundarySideLabel(int boundarySide)
{
    if (boundarySide == BOUNDARY_SIDE_BANGLADESH)
    {
        return "BANGLADESH";
    }
    else if (boundarySide == BOUNDARY_SIDE_ON_BOUNDARY)
    {
        return "ON BOUNDARY";
    }

    return "OTHER SIDE";
}

#endif

