#include "GCSMav_control.h"

//#define GRIDVIEW 

#ifdef GRIDVIEW
#include "GridView.h"
#endif

std::vector<std::string> tests = {"Test1"};
std::string runningTest = tests[0];

int main(int argc, char const* argv[])
{
	GridConfig gconf;
	gconf.refArea = 0.01;
	gconf.gridSize = 20;
	gconf.updateGrid();
	GCSMav_control* gcs = new GCSMav_control(gconf);

#ifdef GRIDVIEW

	Application::Config conf;
	conf.width = 800;
	conf.height = 800;
	conf.resizable = true;
	conf.title = "AIRSPACE GRID";
	conf.msaa = true;	
	GridView grid(conf, gcs);

	grid.run();
#else
	
	sleep(30);
	gcs->checkSystems();	
	sleep(30);


	while (tests.size() > 0)
	{
		double runTime = gcs->getSimTime();
		//Eval UAV Tasks (every 1 second)
		if (runTime - gcs->lastEvalTasks >= 1)
		{
			gcs->lastEvalTasks = runTime;

			for (auto company : gcs->CompanyList)
			{
				for (auto const& [key, uav] : company->UAV_MAP)
				{
					uav->evaluateTasks();
				}
			}
		}

		//Update UAV tasks (every 1 second)
		if (runTime - gcs->lastCheckSlots >= 1)
		{
			gcs->lastCheckSlots = runTime;
			gcs->updateSlots();
		}		

		//Run Tests
		if (runTime - gcs->lastCheckTest >= 1)
		{
			gcs->checkSystems();
			gcs->lastCheckTest = runTime;
			gcs->runTests(runningTest);
			if (gcs->runningTest == false)
			{
					tests.pop_back();
			}
		}
		usleep(5);
	}

	gcs->generateStats(runningTest + ".csv");
	
#endif
	
	return 0;

}
