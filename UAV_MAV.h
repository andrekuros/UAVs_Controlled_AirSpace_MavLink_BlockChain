#include <chrono>
#include <cstdint>
#include <mavsdk/mavsdk.h>
#include <mavsdk/system.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/param/param.h>
#include <mavsdk/plugins/info/info.h>
#include <mavsdk/plugins/mission/mission.h>
#include <mavsdk/plugins/mission_raw/mission_raw.h>
#include <mavsdk/plugins/log_files/log_files.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <iostream>
#include <future>
#include <memory>
#include <thread>
#include <map>
#include <random>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// Defines utilizados pela MAVSDK para colorir o console
#define ERROR_CONSOLE_TEXT     "\033[31m"  // Turn text on console red
#define TELEMETRY_CONSOLE_TEXT "\033[34m"  // Turn text on console blue
#define NORMAL_CONSOLE_TEXT    "\033[0m"   // Restore normal console colour

using namespace mavsdk;


class UAV_draw;

class UAV_MAV {

    public:
        UAV_MAV();

        void positionCallBack(Telemetry::Position position);

        void createMission(std::string type, double lat=0, double lon=0);
        static Mission::MissionItem make_mission_item(double latitude_deg, double longitude_deg,
            float relative_altitude_m, float speed_m_s,
            bool is_fly_through, float gimbal_pitch_deg,
            float gimbal_yaw_deg,
            Mission::MissionItem::CameraAction camera_action);

    
        int SysId;  
        std::shared_ptr<System> system;

        //Param* params;       
        Mission* missionManager; 
        Mission::MissionPlan* currentMission = nullptr;
        Mission::MissionPlan* nextMission = nullptr;
        //LogFiles* logs;
        Telemetry* telemetry;
        Action* action;

        float lat = 0;
        float lon = 0;
        float alt = 0;
        bool isArmed = false;
        bool isLanded = true;
        //bool missionStatus = 
        double hdg = 0;
        int company = -1;

        UAV_draw* drawer;
    
};


