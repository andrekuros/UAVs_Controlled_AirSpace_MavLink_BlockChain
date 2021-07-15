#include "UAV_MAV.h"

UAV_MAV::UAV_MAV() : SysId(0), lat(0), lon(0),system(nullptr), telemetry(nullptr), action(nullptr)
{
	nextMission = new Mission::MissionPlan();
	currentMission = new Mission::MissionPlan();
	
	actionCallback = [&](Action::Result result)
	{		
		if (resultsCounter.find(result) != resultsCounter.end())
			resultsCounter[result] = resultsCounter[result] + 1;
		else
			resultsCounter[result] = 1;

		if (result == Action::Result::Success)
		{
			std::cout << "\n" << cod << "::Task:(" << taskList.front().type << ")::Success\n";
			taskList.erase(taskList.begin());
			actionStatus = "Ready";			
			
		}
		else
		{
			taskRetryCount++;
			actionStatus = "Retry";
			std::cout << "\n" << cod << "::Task:(" << taskList.front().type << ")::Fail(" << taskRetryCount << ")::" << result;
			if (taskRetryCount == 3)
			{
				taskRetryCount = 0;
				taskList.erase(taskList.begin());
				actionStatus = "Ready";

			}
		}				
	};


	missionCallback = [&](Mission::Result result)
	{
		if (resultsCounterMission.find(result) != resultsCounterMission.end())
			resultsCounterMission[result] = resultsCounterMission[result] + 1;
		else
			resultsCounterMission[result] = 1;

		uploadingMission = false;

		if (result == Mission::Result::Success)
		{
			std::cout << "\n" << cod << "::Task:(" << taskList.front().type << ")::Success\n";			
			
			if (taskList.front().type == "missionStart") 
			{
				onMission = true;
				onHold = false;
			}
			if (taskList.front().type == "missionPause")
			{
				onHold = true;
				onMission = false;
			}			
			taskList.erase(taskList.begin());
			actionStatus = "Ready";
		}
		else
		{			
			taskRetryCount++;
			actionStatus = "Retry";
			std::cout << "\n" << cod << "::Task:(" << taskList.front().type << ")::Fail(" << taskRetryCount << "):->" << result;

			if (taskRetryCount == 3)
			{
				taskRetryCount = 0;
				taskList.erase(taskList.begin());
				actionStatus = "Ready";

			}
		}
	};
	
};

void UAV_MAV::createMission(std::string type, double lat, double lon, double area)
{
	
	if (type == "random")
	{
		nextMission->mission_items.clear();
		for (double i = 0; i < 5; i++)
		{
			double rnd = rand() % 16 ;
			double tlat = 47.3977534 - area * (double)rnd/16;
			rnd = rand() % 16;
			double tlon = 8.5456992 + area * (double)rnd / 16;
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
				
		nextMission->mission_items.clear();

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

	if (type == "delivery")
	{

		nextMission->mission_items.clear();

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

void UAV_MAV::evaluateTasks()
{
	if (actionStatus == "Waiting" || taskList.size() == 0 || uploadingMission) return;

	ActionData act = taskList.front();

	if (act.type == "arm") {
		if (!isArmed) 
		{
			sendArm();
		}
		else 
		{
			taskList.erase(taskList.begin());
		}
	}
	
	if (act.type == "takeOff")
	{	
		if (isArmed)
		{
			if (!inAir) {
				sendTakeOff(act.alt);
			}
			else 
			{
				taskList.erase(taskList.begin());
			}
		}
		else 
		{
			ActionData act{ "arm" };
			taskList.insert(taskList.begin(), act);						
		}

	}
	
	if (act.type == "flyTo")
	{
		if (inAir) sendFlyTo(act.lat, act.lon, act.alt, act.hdg);
	}
	
	if (act.type == "missionUpload")
	{
		sendMissionUpload();
	}

	if (act.type == "missionPause")
	{
		sendMissionPause();
	}
	
	if (act.type == "missionStart")
	{
		if (inAir)
		{
			sendMissionStart();
		}
		else 
		{
			ActionData act{ "takeOff" };
			act.alt = 30;
			taskList.insert(taskList.begin(), act);
		}
	}
}

void UAV_MAV::addTask(ActionData act)
{
	std::cout << "\n" << cod << "::NewTask::" << act.type;
	taskList.push_back(act);
}

void UAV_MAV::sendMissionUpload()
{

	if (actionStatus == "Ready" || actionStatus == "Retry")
	{		
		if (missionEventsCounter.find("missionUpload") != missionEventsCounter.end())
			missionEventsCounter["missionUpload"] = missionEventsCounter["missionUpload"] + 1;
		else
			missionEventsCounter["missionUpload"] = 1;
		
		uploadingMission = true;
		missionManager->upload_mission_async(*nextMission, missionCallback);		
		actionStatus = "Waiting";
	}
}


void UAV_MAV::sendMissionPause()
{

	if (actionStatus == "Ready" || actionStatus == "Retry")
	{

		if (missionEventsCounter.find("missionPause") != missionEventsCounter.end())
			missionEventsCounter["missionPause"] = missionEventsCounter["missionPause"] + 1;
		else
			missionEventsCounter["missionPause"] = 1;

		missionManager->pause_mission_async(missionCallback);
		actionStatus = "Waiting";
	}
}

void UAV_MAV::sendMissionStart()
{
	if (actionStatus == "Ready" || actionStatus == "Retry")
	{

		if (missionEventsCounter.find("missionStart") != missionEventsCounter.end())
			missionEventsCounter["missionStart"] = missionEventsCounter["missionStart"] + 1;
		else
			missionEventsCounter["missionStart"] = 1;

		missionManager->start_mission_async(missionCallback);
		actionStatus = "Waiting";
	}
}


void UAV_MAV::sendTakeOff(float alt)
{	

	if (actionStatus == "Ready" || actionStatus == "Retry")
	{		

		if (actionsCounter.find("takeOff") != actionsCounter.end())
			actionsCounter["takeOff"] = actionsCounter["takeOff"] + 1;
		else
			actionsCounter["takeOff"] = 1;

		action->takeoff_async(actionCallback); 

		actionStatus = "Waiting";
	}
}

void UAV_MAV::sendFlyTo(float lat, float lon, float alt, float yaw)
{
	if (actionStatus == "Ready" || actionStatus == "Retry")
	{
		if (actionsCounter.find("flyTo") != actionsCounter.end())
			actionsCounter["flyTo"] = actionsCounter["flyTo"] + 1;
		else
			actionsCounter["flyTo"] = 1;

		action->goto_location_async(lat, lon, 488.0+30.0, yaw, actionCallback);
	
		actionStatus = "Waiting";
	}
}

void UAV_MAV::sendArm()
{
	if (actionStatus == "Ready" || actionStatus == "Retry")
	{

		if (actionsCounter.find("arm") != actionsCounter.end())
			actionsCounter["arm"] = actionsCounter["arm"] + 1;
		else
			actionsCounter["arm"] = 1;

		std::cout << "\n" << cod << "::Task::Arm::Sent";
		
		action->arm_async(actionCallback);

		actionStatus = "Waiting";
	}
}

std::function<void(Action::Result)> actionCallback;
