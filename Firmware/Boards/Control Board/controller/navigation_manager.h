#pragma once

#include "devices.h"

enum SensorPosition
{
    NORTH,
    NORTH_EAST
    SOUTH_EAST
    SOUTH_WEST
    NORTH_WEST
};

class NavigationManager
{
public:
    bool checkForObstacle()
    {
        return north.getDist(maxDistance) <= obstacleDistance;
    }

    bool checkFrontWall()
    {
        return north.getDist(maxDistance) <= frontWallDistance;
    }

    bool checkForTimeout(SensorPosition pos)
    {
        switch (pos)
        {
        case NORTH:
            if (north.getDist(maxDistance) != maxDistance)
                return false;
            else
                return true;
            break;
        case NORTH_EAST:
            if (northEast.getDist(maxDistance) != maxDistance)
                return false;
            else
                return true;
            break;
        case SOUTH_EAST:
            if (southEast.getDist(maxDistance) != maxDistance)
                return false;
            else
                return true;
            break;
        case SOUTH_WEST:
            if (southWest.getDist(maxDistance) != maxDistance)
                return false;
            else
                return true;
            break;
        case NORTH_WEST:
            if (northWest.getDist(maxDistance) != maxDistance)
                return false;
            else
                return true;
            break;
        }

    }

    void getMaxDistance() { return maxDistance; }
    void setMaxDistance(unsigned int _maxDistance)
    {
        maxDistance = _maxDistance;
    }

    void getObstacleDistance() { return obstacleDistance; }
    void setObstacleDistance(unsigned int _obstacleDistance)
    {
        obstacleDistance = _obstacleDistance;
    }

    void getFrontWallDistance() { return frontWallDistance; }
    void setFrontWallDistance(unsigned int _frontWallDistance)
    {
        frontWallDistance = _frontWallDistance;
    }

private:
    SR_04 north = SR_04(US_N_T, US_N_E);
    SR_04 northEast = SR_04(US_NE_T, US_NE_E);
    SR_04 southEast = SR_04(US_SE_T, US_SE_E);
    SR_04 southWest = SR_04(US_SW_T, US_SW_E);
    SR_04 northWest = SR_04(US_NW_T, US_NW_E);

    unsigned long maxDistance = 10 //cm

        static const int fixedFrontDistance = 1; //cm
    static const int fixedSideDistance = 3;      //cm

    unsigned int obstacleDistance = fixedFrontDistance;
    unsigned int sideWallDistance = fixedSideDistance;
    unsigned int frontWallDistance = fixedFrontDistance;
}