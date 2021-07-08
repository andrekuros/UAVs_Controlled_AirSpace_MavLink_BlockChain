#ifndef __Utils__
#define __Utils__

#include "UAV_MAV.h"



//constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double HALFPI = 3.141592653589793238462643383279502884 * 0.5;
constexpr double TWOPI = 2 * 3.141592653589793238462643383279502884;
constexpr double E = 2.718281828459045235360287471352662497;
constexpr double SQRT2 = 1.414213562373095048801688724209698078;
constexpr double SQRT3 = 1.7320508075688772935274463415058723669;
constexpr double EPS = std::numeric_limits<double>::epsilon();
constexpr double INF = std::numeric_limits<double>::infinity();
constexpr double NaN = std::numeric_limits<double>::quiet_NaN();

//class UAV_MAV;

/// Represents a coordinate position in the Board
struct Coord {
    Coord() : r(0), c(0) {}
    Coord(float x, float y) : r(x), c(y) {}
    float r;
    float c;
};

struct Pos {
    Pos() : x(0), y(0) {}
    Pos(float x, float y) : x(x), y(y) {}
    float x;
    float y;
};


struct Geo {
    Geo() : lat(0), lon(0) {}
    Geo(float lat, float lon) : lat(lat), lon(lon) {}
    float lat;
    float lon;
};


class Company {
public:    
    Company() {};
    mavsdk::Mavsdk* mavsdk = nullptr;
    std::map<int, UAV_MAV*> UAV_MAP;
    std::string name = "Company";
    int cod = 0;
    Coord cellPos;
    Geo geoPos;

};

struct CellRequest
{
    CellRequest(UAV_MAV* uav, double timeTake, double timeRelease) :
        uav(uav), timeTake(timeTake), timeRelease(timeRelease) {}

    UAV_MAV* uav;
    double timeTake;
    double timeRelease;

};

class Cell {    
public:
    int r;
    int c;
    int conflictsCount = 0;
    std::vector<UAV_MAV*> onCellList;    
    std::vector<CellRequest> requestList;
    int company = -1;

   /* void remofromOnCell(UAV_MAV* uav)
    { 
        for (int i = 0; i < onCellList.size(); i++)
        {
            if (onCellList[i]->cod == uav->cod)
            {
                onCellList.erase(onCellList.begin() + i);
                return;
            }
        }
    }*/
};

struct GridConfig
{
    float refLat = 47.397742;
    float refLon = 8.545594;
    float refAlt = 488.0;
    float refArea = 0.01;
    int gridSize = 20;
    
    float gridCorrection = (float)gridSize / 15.0;
    float refStep = refArea / gridSize;
    float g_sideLength = 25.0f / gridCorrection;
    float g_gridSize = (2.0f + 1.414213562373095048801688724209698078f) * 0.5f * g_sideLength * 0.999f;


    void updateGrid() 
    {
        gridCorrection = (float)gridSize / 15.0;
        refStep = refArea / gridSize;
        g_sideLength = 25.0f / gridCorrection;
        g_gridSize = (2.0f + 1.414213562373095048801688724209698078f) * 0.5f * g_sideLength * 0.999f;
    }
    
};



//==========================================================================
// HELPERS
//==============================================================================

inline Pos coordPosition(const Coord& coord, GridConfig gconf) {
    return Pos(coord.c * gconf.g_gridSize, coord.r * gconf.g_gridSize);
}

inline Pos coordPosition(float i, float j, GridConfig gconf) { return coordPosition({ i, j }, gconf); }

inline Coord geoToGrid(Geo pos, GridConfig gconf)
{
    return { -(pos.lat - gconf.refLat) / gconf.refStep, (pos.lon - gconf.refLon) / gconf.refStep };
}

inline Geo gridToGeo(Coord pos, GridConfig gconf)
{
    return { gconf.refLat - pos.r * gconf.refStep, gconf.refLon + pos.c * gconf.refStep };
}

inline float hdgFromPath(Geo pos1, Geo pos2)
{
    return atan2(pos1.lat - pos2.lat, pos1.lon - pos2.lon) * 180.0 / 3.1416;
}



#endif
