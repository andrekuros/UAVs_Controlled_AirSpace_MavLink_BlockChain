#include "GCSMav_control.h"

void GCSMav_control::updateSlots()
{
    for (auto company : CompanyList)
    {       
        for (auto const& [key, uav] : company->UAV_MAP)
        {           

            //Update UAV Data
            Coord uavGridPos = geoToGrid(Geo( uav->lat, uav->lon), gconf);
                        
            int r = round(uavGridPos.r);
            int c = round(uavGridPos.c);
            if (c >= 0 && r >= 0 && c < gconf.gridSize && r < gconf.gridSize)
            {
                if (uav->cell != AirspaceSlots[r][c])
                {
                    //uav->cell->remofromOnCell(uav);
                    if (uav->cell != nullptr)
                    {
                        for (int i = 0; i < uav->cell->onCellList.size(); i++)
                        {
                            if (uav->cell->onCellList[i]->cod == uav->cod)
                            {
                                uav->cell->onCellList.erase(uav->cell->onCellList.begin() + i);
                                break;
                            }
                        }
                    }

                    uav->cell = AirspaceSlots[r][c];
                    if (AirspaceSlots[r][c]->onCellList.size() != 0)
                    {
                        AirspaceSlots[r][c]->conflictsCount++;
                    }                    
                    AirspaceSlots[r][c]->onCellList.push_back(uav);                                        
                }                                
            }                
        }        
    }
}


void GCSMav_control::checkSystems()
{
    for (auto company : CompanyList)
    {
        for (auto system : company->mavsdk->systems())
        {
            int  sysId = system->get_system_id();

            if (company->UAV_MAP.find(sysId) == company->UAV_MAP.end())
            {
                UAV_MAV* uav = new UAV_MAV();
                company->UAV_MAP[sysId] = uav;
                uav->system = system;
                uav->SysId = sysId;
                uav->company = company->cod;
                uav->cod = "c"  + std::to_string(company->cod) + "-n" + std::to_string(sysId);

                uav->telemetry = new Telemetry{ uav->system };
                uav->action = new Action{ uav->system };
                uav->missionManager = new Mission{ uav->system };

                uav->telemetry->set_rate_position_async(5.0, [](Telemetry::Result result) {});
                uav->telemetry->set_rate_velocity_ned_async(5.0, [](Telemetry::Result result) {});
                // uav->telemetry->set_rate_ar(1.0, [](Telemetry::Result result) {});
                 //uav->telemetry->set_rate_(5.0);
                 //UAVMap[sysId]->telemetry->set_rate_scaled_imu(50.0);                     
                uav->telemetry->subscribe_position([uav](Telemetry::Position position)
                    {
                        uav->lat = position.latitude_deg;
                        uav->lon = position.longitude_deg;
                    });

                uav->telemetry->subscribe_armed([uav](bool isArmed)
                    {
                        uav->isArmed = isArmed;
                    });

                uav->telemetry->subscribe_in_air([uav](bool inAir)
                    {
                        uav->inAir = inAir;
                    });

                uav->telemetry->subscribe_velocity_ned([uav](Telemetry::VelocityNed velNed)
                    {
                        uav->hdg = atan2(velNed.east_m_s, velNed.north_m_s) * 180.0 / 3.1416;
                    });

                uav->missionManager->subscribe_mission_progress([uav](Mission::MissionProgress progress)
                    {
                        uav->missionProgress = (double)progress.current / (double)progress.total;
                        /*if (progress.current == progress.total)
                        {
                            uav->onMission = false;
                            uav->finishedMission = false;
                            uav->onHold = false;

                        }*/
                        
                    });

                

                /* uav->telemetry->subscribe_status_text([uav](Telemetry::StatusText text)
                     {
                         //uav->statusText = text;
                         std::cout << "\n" << text;
                     });*/                
            }
        }
    }
}

void GCSMav_control::prepareSim(std::string type)
{
    if (type == "delivery")
    {
        for (auto company : CompanyList)
        {
            for (auto const [key, uav] : company->UAV_MAP)
            {
               
                ActionData actArm{ "arm" };                            
                uav->addTask(actArm);

                ActionData actTakeOff{ "takeOff" };
                actTakeOff.alt = 2.5f ;                
                uav->addTask(actTakeOff);
                //ActionData actFlyTo{ "flyTo", company->geoPos.lat, company->geoPos.lon, 30.0f, hdgFromPath(Geo(uav->lat, uav->lon), company->geoPos) };
                //uav->addTask(actFlyTo);
 
                //uav->createMission("flyto", company->geoPos.lat, company->geoPos.lon);
                // ActionData actMissionUpload{ "missionUpload" };                
                //uav->addTask(actMissionUpload);

                //ActionData actMissionStart{ "missionStart" };                
                //uav->addTask(actMissionStart);


            }
        }

    }
}
void GCSMav_control::runTests(std::string test)
{
    if (test == "Test1")
    {              
        double testRunTime = getSimTime() - testStart;
        if (runningTest)
        {                       
            if (testRunTime >= 10)
            {
                if ((testFase + 2) * 5 < testRunTime && testFase <= testTotal)
                {
                    testFase++;

                    std::cout << "\nTestRunning::Fase::" << testFase;

                    for (auto company : CompanyList)
                    {
                        for (auto const [key, uav] : company->UAV_MAP)
                        {                            
                            if (testFase % 2 == 0)
                            {
                                //Geo geo = gridToGeo(Coord(rand() % 20, rand() % 20), gconf);
                                //uav->createMission("flyTo", geo.lat, geo.lon, 0.01);
                               // ActionData actMissionUpload{ "flyTo", geo.lat, geo.lon,488+30,rand()% 360};
                                //uav->addTask(actMissionUpload);

                               // ActionData actMissionStart{ "missionStart" };
                               // uav->addTask(actMissionStart);
                               // uav->onMission = true;
                                uav->createMission("random", 0, 0, 0.01);
                                ActionData actMissionUpload{ "missionUpload" };
                                uav->addTask(actMissionUpload);
                            }
                            else
                            {
                                uav->createMission("random", 0, 0, 0.01);
                                ActionData actMissionUpload{ "missionUpload" };
                                uav->addTask(actMissionUpload);

                               // ActionData actMissionStart{ "missionStart" };
                               // uav->addTask(actMissionStart);
                               // uav->onMission = true;
                            }
                        }
                    }
                }
            }
        }
        else 
        {           
            testRunning == test;
            testStart = getSimTime();
            testFase = 0;
            testTotal = 20; 
            std::cout << "\nTESTE01::Started";
            prepareSim("delivery");
            runningTest = true;
        }

        if (testFase >= testTotal)
        {
            testRunning = "Finalizing...";            
            if (testRunTime > (testTotal + 2 ) * 5 + 15)
            {
                testRunning = "None";
                runningTest = false;
                std::cout << "\nTESTE01::UFinished11(" << getSimTime() - testStart << ")";
                testStart = 0;                
            }
        }        
    }

   
}

void GCSMav_control::generateStats(std::string file)
{    
    std::ofstream myfile;
    myfile.open(file, std::ios_base::app);

    myfile << "\n\ncod; company; actionRequests; actionSuccess; missionRequests; missionsSuccess; busy; cancel; timeouts\n";
     
    for (auto company : CompanyList)
    {                       
        for (auto const [key, uav] : company->UAV_MAP)
        {  
            int actionReq = 0;
            int actionOk = 0;
            int missionReq = 0;
            int missionOk = 0;
            int missionBusy = 0;
            int missionCancel = 0;
            int timeOuts = 0;

            for (auto const [key, res] : uav->actionsCounter) actionReq = actionReq + res;
            actionOk = uav->resultsCounter[Action::Result::Success];
            
            if (uav->resultsCounter.find(Action::Result::Timeout) !=
                uav->resultsCounter.end())
            {
                timeOuts += uav->resultsCounter[Action::Result::Timeout];
            }

            for (auto const [key, res] : uav->missionEventsCounter) missionReq = missionReq + res;
            missionOk = uav->resultsCounterMission[Mission::Result::Success];
            
            if (uav->resultsCounterMission.find(Mission::Result::TransferCancelled) !=
                uav->resultsCounterMission.end())
            {
                missionCancel = uav->resultsCounterMission[Mission::Result::TransferCancelled];
            }  
            
            if (uav->resultsCounterMission.find(Mission::Result::Busy) !=
                uav->resultsCounterMission.end())
            {
                missionBusy = uav->resultsCounterMission[Mission::Result::Busy];
            }

            if (uav->resultsCounterMission.find(Mission::Result::Timeout) !=
                uav->resultsCounterMission.end())
            {
                timeOuts = uav->resultsCounterMission[Mission::Result::Timeout];
            }
            
 //myfile << "cod; company; actionRequests; actionSuccess; missionRequests; missionsSuccess, timeouts\n";
            myfile << uav->cod << ";" << company->cod << ";";
            myfile << actionReq << ";" << actionOk << ";";
            myfile << missionReq << ";" << missionOk << ";";
            myfile << missionBusy << ";" << missionCancel << ";";
            myfile << timeOuts << "\n";
        }
    }    
    myfile.close();
}


GCSMav_control::GCSMav_control(GridConfig gconf) : gconf(gconf)
{            
    std::vector<Coord> companiesCoord = { {2,2}, {19,3}, {15,18} };
    
    
    for (int i = 0; i < numCompanies; i++)
    {
        Mavsdk* mavsdk = new Mavsdk();        
        mavsdk->set_timeout_s(3);

        ConnectionResult connection_result = mavsdk->add_any_connection("udp://0.0.0.0:1454" + std::to_string(i));
        mavsdk->subscribe_on_new_system([&mavsdk]() {});

        Company* aux_company = new Company();
        aux_company->mavsdk = mavsdk;
        aux_company->cod = i + 1;
        aux_company->cellPos = companiesCoord[i];
        aux_company->geoPos = gridToGeo(companiesCoord[i], gconf);

        CompanyList.push_back(aux_company);
        std::cout << "Connection result: " << connection_result << '\n';        
    }
    
    for (int r = 0; r < gconf.gridSize; r++)
    {        
        std::vector<Cell*> row;
        for (int c = 0; c < gconf.gridSize; c++)
        {
            Cell* cell = new Cell();
            cell->r = r;
            cell->c = c;
            row.push_back(cell);
            for (int i = 0; i < numCompanies; i++)
            {
                if (CompanyList[i]->cellPos.c == c && CompanyList[i]->cellPos.r == r)
                {
                    cell->company = i + 1 ;
                }
            }

        }
        AirspaceSlots.push_back(row);               
    }  
    
    startTime = std::clock();
}

double GCSMav_control::getSimTime()
{
    return (std::clock() - startTime) / (double)CLOCKS_PER_SEC;
}

double GCSMav_control::diffTime(double timeIni, double timeCur)
{
    return (timeCur - timeIni);
}


void GCSMav_control::resetTime()
{    
    startTime = std::clock();
    return;
}
    






