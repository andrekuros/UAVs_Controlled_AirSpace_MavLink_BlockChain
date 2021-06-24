#include "UAV_MAV.h"

UAV_MAV::UAV_MAV() : SysId(0), lat(0), lon(0),system(nullptr), telemetry(nullptr), action(nullptr)
{
	nextMission = new Mission::MissionPlan();
	currentMission = new Mission::MissionPlan();
	
};

/*void UAV_MAV::updateTelemetry() {

    telemetry->subscribe_position(positionCallBack);

}*/


void UAV_MAV::createMission(std::string type, double lat, double lon)
{
	
	if (type == "random")
	{
		for (double i = 0; i < 5; i++)
		{
			double rnd = rand() % 16 ;
			double tlat = 47.3977534 - 0.025 * (double)rnd/16;
			rnd = rand() % 16;
			double tlon = 8.5456992 + 0.025 * (double)rnd / 16;
			nextMission->mission_items.push_back(make_mission_item(
				tlat,  //Latitude  [degress]
				tlon,   //Longitude [degress]
				30.0f,       //Altitude  [meters]
				20.0f,        //Speed     [m/s]
				false,       //is_fly_thought - drone para ou não no Waypoint
				20.0f,       //Gimbal Pitch [degress]
				0.0f,       //Gimbal Yaw [degress]
				Mission::MissionItem::CameraAction::None)); //Tipo de ação de camera  ++		
		}
	}

	if (type == "flyto")
	{
				
		nextMission->mission_items.push_back(make_mission_item(
			lat,  //Latitude  [degress]
			lon,   //Longitude [degress]
			30.0f,       //Altitude  [meters]
			20.0f,        //Speed     [m/s]
			false,       //is_fly_thought - drone para ou não no Waypoint
			20.0f,       //Gimbal Pitch [degress]
			0.0f,       //Gimbal Yaw [degress]
			Mission::MissionItem::CameraAction::None)); //Tipo de ação de camera  ++		
		
	}

	if (type == "delyvery")
	{

		nextMission->mission_items.push_back(make_mission_item(
			lat,  //Latitude  [degress]
			lon,   //Longitude [degress]
			30.0f,       //Altitude  [meters]
			20.0f,        //Speed     [m/s]
			false,       //is_fly_thought - drone para ou não no Waypoint
			20.0f,       //Gimbal Pitch [degress]
			0.0f,       //Gimbal Yaw [degress]
			Mission::MissionItem::CameraAction::None)); //Tipo de ação de camera  ++		

	}
}

Mission::MissionItem UAV_MAV::make_mission_item(
	double latitude_deg,
	double longitude_deg,
	float relative_altitude_m,
	float speed_m_s,
	bool is_fly_through,
	float gimbal_pitch_deg,
	float gimbal_yaw_deg,
	Mission::MissionItem::CameraAction camera_action)
{
	Mission::MissionItem new_item{};
	new_item.latitude_deg = latitude_deg;
	new_item.longitude_deg = longitude_deg;
	new_item.relative_altitude_m = relative_altitude_m;
	new_item.speed_m_s = speed_m_s;
	new_item.is_fly_through = is_fly_through;
	new_item.gimbal_pitch_deg = gimbal_pitch_deg;
	new_item.gimbal_yaw_deg = gimbal_yaw_deg;
	new_item.camera_action = camera_action;
	return new_item;
}

