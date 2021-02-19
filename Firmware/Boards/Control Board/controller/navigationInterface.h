#pragma once

#include "devices.h"

class NavigationInterface
{
public:
    enum SensorPosition
    {
        NORTH,
        NORTH_EAST,
        SOUTH_EAST,
        SOUTH_WEST,
        NORTH_WEST
    };
    enum WallPosition
    {
        LEFT_WALL,
        RIGHT_WALL
    };

    NavigationInterface() {}

    WallPosition getClosestSideWall()
    {
        float leftDist = northWest.getDist(maxSideDistance);
        float rightDist = northEast.getDist(maxSideDistance);
        return leftDist < rightDist ? LEFT_WALL : RIGHT_WALL;
    }

    bool checkForFrontWall()
    {
        return north.getDist(maxFrontDistance) <= frontWallDistance;
    }
    bool isObstacleInRange()
    {
        return north.getDist(maxFrontDistance) <= obstacleDetectDistance;
    }

    bool checkForTimeout(SensorPosition pos)
    {
        switch (pos)
        {
        case NORTH:
            if (north.getDist(maxFrontDistance) != maxFrontDistance)
                return false;
            else
                return true;
            break;
        case NORTH_EAST:
            if (northEast.getDist(maxSideDistance) != maxSideDistance)
                return false;
            else
                return true;
            break;
        case SOUTH_EAST:
            if (southEast.getDist(maxSideDistance) != maxSideDistance)
                return false;
            else
                return true;
            break;
        case SOUTH_WEST:
            if (southWest.getDist(maxSideDistance) != maxSideDistance)
                return false;
            else
                return true;
            break;
        case NORTH_WEST:
            if (northWest.getDist(maxSideDistance) != maxSideDistance)
                return false;
            else
                return true;
            break;
        }
    }

    unsigned int getMaxFrontDistance() { return maxFrontDistance; }
    void setMaxFrontDistance(unsigned int _maxFrontDistance)
    {
        maxFrontDistance = fixedFrontDistance + _maxFrontDistance;
    }

    unsigned int getMaxSideDistance() { return maxSideDistance; }
    void setMaxSideDistance(unsigned int _maxSideDistance)
    {
        maxSideDistance = fixedSideDistance + _maxSideDistance;
    }

    unsigned int getObstacleDetectDistance() { return obstacleDetectDistance; }
    void setObstacleDetectDistance(unsigned int _obstacleDetectDistance)
    {
        obstacleDetectDistance = fixedFrontDistance + _obstacleDetectDistance;
    }

    unsigned int getObstacleSideDistance() { return obstacleSideDistance; }
    void setObstacleSideDistance(unsigned int _obstacleSideDistance)
    {
        obstacleSideDistance = fixedFrontDistance + _obstacleSideDistance;
    }

    unsigned int getFrontWallDistance() { return frontWallDistance; }
    void setFrontWallDistance(unsigned int _frontWallDistance)
    {
        frontWallDistance = fixedFrontDistance + _frontWallDistance;
    }

    SR_04 &getNorthSensor()
    {
        return north;
    }
    SR_04 &getNorthEastSensor()
    {
        return northEast;
    }
    SR_04 &getSouthEastSensor()
    {
        return southEast;
    }
    SR_04 &getSouthWestSensor()
    {
        return southWest;
    }
    SR_04 &getNorthWestSensor()
    {
        return northWest;
    }

private:
    SR_04 north = SR_04(US_N_T, US_N_E);
    SR_04 northEast = SR_04(US_NE_T, US_NE_E);
    SR_04 southEast = SR_04(US_SE_T, US_SE_E);
    SR_04 southWest = SR_04(US_SW_T, US_SW_E);
    SR_04 northWest = SR_04(US_NW_T, US_NW_E);

    unsigned long maxFrontDistance = 10; //cm
    unsigned long maxSideDistance = 5;   //cm

    static const int fixedFrontDistance = 1; //cm
    static const int fixedSideDistance = 3;  //cm

    unsigned int obstacleDetectDistance = fixedFrontDistance;
    unsigned int obstacleSideDistance = fixedSideDistance;

    unsigned int frontWallDistance = fixedFrontDistance;
    unsigned int sideWallDistance = fixedSideDistance;
};