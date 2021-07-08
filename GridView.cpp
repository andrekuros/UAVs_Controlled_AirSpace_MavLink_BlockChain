#include "GridView.h"

//==============================================================================
// BOARD
//==============================================================================
Board::Board(GridView* grid, GridConfig gconf)
{
    shape.set_point_count(4);
    shape.set_point(0, coordPosition(-1, -1, gconf).x, coordPosition(-1, -1, gconf).y);
    shape.set_point(1, coordPosition(gconf.gridSize, -1, gconf).x, coordPosition(gconf.gridSize, -1, gconf).y);
    shape.set_point(2, coordPosition(gconf.gridSize, gconf.gridSize, gconf).x, coordPosition(gconf.gridSize, gconf.gridSize, gconf).y);
    shape.set_point(3, coordPosition(-1, gconf.gridSize, gconf).x, coordPosition(-1, gconf.gridSize, gconf).y);
    // shape.set_radii(g_gridSize);
    // auto hole = makeShape(matrix);
    // hole = offset_shape(hole, 2.0f);
    // shape.push_back_hole(hole);
    this->grid = grid;
}

std::string Board::matToStr(std::vector<std::vector<Cell*>> mat) {
    std::stringstream ss;
    for (auto& row : mat) {
        for (auto& val : row) {
            if (val->onCellList.size() == 0)
                ss << ". ";
            else
                ss << val->onCellList.size() << " ";
        }
        ss << std::endl;
    }
    return ss.str();
}

void Board::draw(NVGcontext* vg) 
{
    float trans[6], inv[6];
    nvgCurrentTransform(vg, trans);
    nvgTransformInverse(inv, trans);
    Vec2 mouse = grid->get_mouse_pos();
    nvgTransformPoint(&mouse.x, &mouse.y, inv, mouse.x, mouse.y);
    bool hovered = shape.contains(mouse);
    
    if (grid->showSlots && hovered)
    {
        ImGui::BeginTooltip();
        ImGui::Text("SLOTSs");
        ImGui::Separator();
        auto matStr = matToStr(grid->gcs->AirspaceSlots);
        ImGui::Text(matStr.c_str());
        ImGui::Separator();
        ImGui::EndTooltip();
    }

    Vec2 tl = shape.bounds().position();
    Vec2 br = tl + shape.bounds().size();
    nvgBeginPath(vg);
    nvgShapeWithHoles(vg, shape);
    auto paint = nvgLinearGradient(vg, tl.x, tl.y, br.x, br.y, { 0.5f, 0.5f, 0.5f, 0.25f },
        { 0.5f, 0.5f, 0.5f, 0.5f });
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    nvgStrokeColor(vg, Grays::Gray80);
    // nvgStrokeWidth(vg, hovered ? 3 : 1);
    nvgStroke(vg);
}


//==============================================================================
// SLOT_draw
//==============================================================================
SLOT_draw::SLOT_draw(Application* app, Color col, GridConfig gconf, GCSMav_control* gcs) : app(app), color(col), gconf(gconf), gcs(gcs)
{
    shape = make_rectangle_shape(gconf.g_sideLength * 2, gconf.g_sideLength * 2);
    shape = offset_shape(shape, -4.0f / gconf.gridCorrection);
    alpha = 0.25;
    set_pos({ 0, 0 });
    set_origin({ 0, 0 });
    set_scale({ 1, 1 });
    set_rotation(0);
}

void SLOT_draw::draw(NVGcontext* vg)
{
    nvgTransform(vg, transform());

    float trans[6], inv[6];
    nvgCurrentTransform(vg, trans);
    nvgTransformInverse(inv, trans);
    Vec2 mouse = app->get_mouse_pos();
    nvgTransformPoint(&mouse.x, &mouse.y, inv, mouse.x, mouse.y);
    bool hovered = shape.contains(mouse);
    if (hovered)
    {
        ImGui::BeginTooltip();
        ImGui::TextColored(color, "SLOT");
        ImGui::Text("R:%f C:%f", coord.r, coord.c);
        ImGui::Text("Lat:%f Lon:%f", pos().x, pos().y);
        ImGui::Separator();
        ImGui::EndTooltip();
    }

    if (gcs->AirspaceSlots[coord.r][coord.c]->onCellList.size() != 0)
        alpha = 0.7;
    else
        alpha = 0.1;

    if (gcs->AirspaceSlots[coord.r][coord.c]->onCellList.size() > 1)
    {
        color = Reds::FireBrick;
    }
    else
    {
        color = Blues::LightBlue;
    }

    if (gcs->AirspaceSlots[coord.r][coord.c]->company != -1)
    {
        color = g_colors[gcs->AirspaceSlots[coord.r][coord.c]->company];
        alpha = 0.9;
    }

 

    Vec2 tl = shape.bounds().position();
    Vec2 br = tl + shape.bounds().size();
    nvgBeginPath(vg);
    nvgShape(vg, shape);
    auto paint = nvgLinearGradient(vg, tl.x, tl.y, br.x, br.y, with_alpha(color, alpha),
        with_alpha(color, alpha));
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    nvgStrokeColor(vg, Tween::Linear(Whites::White, color, 0.5f));
    nvgStrokeWidth(vg, hovered ? 3 : 1);
    nvgStroke(vg);
}

UAV_draw::UAV_draw(Application* app, Color col, GridConfig gconf) : app(app), color(col), gconf(gconf)
{
    shape = make_polygon_shape(3, 15, mahi::gui::PolyParam::SideLength);

    //shape = offset_shape(shape, -4.0f);   
    alpha = 1.0;
    set_pos({ 0, 0 });
    set_origin({ 0, 0 });
    set_scale({ 1, 1 });
    set_rotation(0);
}

void UAV_draw::draw(NVGcontext* vg)
{
    nvgTransform(vg, transform());

    float trans[6], inv[6];
    nvgCurrentTransform(vg, trans);
    nvgTransformInverse(inv, trans);
    Vec2 mouse = app->get_mouse_pos();
    nvgTransformPoint(&mouse.x, &mouse.y, inv, mouse.x, mouse.y);
    bool hovered = shape.contains(mouse);
    if (hovered) {
        ImGui::BeginTooltip();
        ImGui::TextColored(color, "UAV");
        ImGui::Text("Company: %d SYS_ID: %d", company, sysId);
        Geo geoPos = gridToGeo(coord, gconf);
        ImGui::Text("R:%f C:%f", coord.r, coord.c);
        ImGui::Text("Lat:%f Lon:%f", geoPos.lat , geoPos.lon);
        ImGui::Separator();
        ImGui::EndTooltip();
    }

    Vec2 tl = shape.bounds().position();
    Vec2 br = tl + shape.bounds().size();
    nvgBeginPath(vg);
    nvgShape(vg, shape);
    auto paint = nvgLinearGradient(vg, tl.x, tl.y, br.x, br.y, with_alpha(color, alpha),
        with_alpha(color, alpha));
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    nvgStrokeColor(vg, Tween::Linear(Whites::White, color, 0.5f));
    nvgStrokeWidth(vg, hovered ? 3 : 1);
    nvgStroke(vg);
}
//==============================================================================
// GRID VIEW
//==============================================================================

GridView::GridView(Config conf, GCSMav_control* gcs) : Application(conf), board(this, gcs->gconf), gcs(gcs){
    SLOTs.reserve(gconf.gridSize * gconf.gridSize);

    for (size_t i = 0; i < gconf.gridSize; i++) {
        for (size_t j = 0; j < gconf.gridSize; j++) {
            auto slot = SLOT_draw(this, Grays::DimGray, gconf, gcs);
            slot.setColor(Blues::DeepSkyBlue, 0.10);
            Coord newCoord{ (float)i, (float)j };
            slot.place(newCoord, 0);
            SLOTs.emplace_back(std::move(slot));
        }
    }
    // solve
    set_vsync(true);
    nvgCreateFontMem(m_vg, "roboto-bold", Roboto_Bold_ttf, Roboto_Bold_ttf_len, 0);
    createCheckerBoard();
}

void GridView::createCheckerBoard() 
{
    checker = nvgluCreateFramebuffer(m_vg, gconf.gridSize + 1, gconf.gridSize + 1, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
    nvgluBindFramebuffer(checker);
    glViewport(0, 0, gconf.gridSize + 1, gconf.gridSize + 1);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    nvgBeginFrame(m_vg, gconf.gridSize + 1, gconf.gridSize + 1, 1);
    nvgBeginPath(m_vg);
    nvgRect(m_vg, 0, 0, gconf.gridSize / 2, gconf.gridSize / 2);
    nvgFillColor(m_vg, Grays::Gray10);
    nvgFill(m_vg);
    nvgBeginPath(m_vg);
    nvgRect(m_vg, gconf.gridSize / 2, gconf.gridSize / 2, gconf.gridSize + 1, gconf.gridSize + 1);
    nvgFillColor(m_vg, Grays::Gray10);
    nvgFill(m_vg);
    nvgEndFrame(m_vg);
    nvgluBindFramebuffer(nullptr);
}

void GridView::update()
{
    double runTime = gcs->getSimTime();
    bool evaluatedTasks = false;

    //Update UAV tasks (every 1 second)
    if (runTime - gcs->lastCheckSlots >= 1)
    {
        gcs->lastCheckSlots = runTime;
        gcs->updateSlots();
    }
    
    ImGui::Begin("GridView");
    ImGui::EndDisabled();
    ImGui::Text("Settings");
    ImGui::Separator();
    ImGui::SameLine();
    if (ImGui::Button("Update Systems"))
        gcs->checkSystems();
    ImGui::SameLine();
    if (ImGui::Button("Prepare Simulation"))
        gcs->prepareSim("delivery");
    ImGui::SameLine();
    ImGui::Text("RunTime: %3d", (int)runTime);
    ImGui::SameLine();
    if (ImGui::Button("rst"))
        gcs->resetTime();
    
    ImGui::Separator();
    ImGui::Checkbox("ShowSlots", &showSlots);
    ImGui::Separator();
   
    ImGui::SameLine();
    //ImGui::Text("Num Sys: %d", gcs->NumSys);
    ImGui::Separator();

    //Slots.clear();
   // Slots = zeros(gconf.gridSize, gconf.gridSize);
    
    for (auto company : gcs->CompanyList)
    {

    ImGui::Separator();
    std::string comp = "#Company: " + std::to_string(company->cod);
    ImGui::ColorButton("a", g_colors[company->cod]);
    ImGui::SameLine();
    ImGui::Text(comp.c_str());
    ImGui::Separator();

        for (auto const& [key, uav] : company->UAV_MAP)
        {
            ImGui::PushID(key);
            ImGui::PushItemWidth(100);
            ImGui::Text("ID: %3d | Lat: %4.2f | Lon %4.2f | Hdg: %6.1f", uav->SysId, uav->lat, uav->lon, uav->hdg);
            ImGui::SameLine();
            ImGui::Text("Status: %s", uav->actionStatus);

            //ImGui::SameLine();
            if (ImGui::Button(uav->isArmed ? (uav->inAir ? "InAir" : "TakeOff") : "Arm"))
            {
                if (uav->isArmed)
                {
                    ActionData actTakeOff{ "takeOff" };
                    actTakeOff.alt = 30.0f;
                    uav->addTask(actTakeOff);
                }
                
                else 
                {
                    ActionData actArm{ "arm" };
                    uav->addTask(actArm);                
                }
                
            }

            ImGui::SameLine();
            if (ImGui::Button("RND Mission"))
            {
                uav->createMission("random", 0, 0, 0.01);                      
                ActionData actMissionUpload{ "missionUpload" };
                uav->addTask(actMissionUpload);
           
            }
            ImGui::SameLine();
            if (ImGui::Button(!uav->onMission? "Start" : "Pause"))
            {
                if (!uav->onMission)
                {
                    ActionData actMissionStart{ "missionStart" };
                    uav->addTask(actMissionStart);
                    uav->onMission = true;
                }
                else
                {
                    ActionData actMissionPause{ "missionPause" };
                    uav->addTask(actMissionPause);
                    uav->onMission = false;
                }
            }


            ImGui::PopItemWidth();
            ImGui::PopID();

            if (!uav->hasDrawer)
            {
                auto u = new UAV_draw(this, Grays::DarkSlateGray, gconf);
                u->company = company->cod;
                u->sysId = uav->SysId;
                u->setColor(g_colors[company->cod], 0.8);
                u->place({ 0,0 }, 0);
                uavDrawers[uav->cod] = u;
                uav->hasDrawer = true;
            }

            UAV_draw* uavDrawer = uavDrawers[uav->cod];
            Coord uavPos = geoToGrid({ uav->lat, uav->lon }, gcs->gconf);
            uavDrawer->place(uavPos, uav->hdg);

            //Eval UAV Tasks (every 1 second)
            if (runTime - gcs->lastEvalTasks >= 1)
            {
                evaluatedTasks = true;
                uav->evaluateTasks();
            }
        }
        ImGui::Separator();
    }

    if (evaluatedTasks) gcs->lastEvalTasks = runTime;

    ImGui::Separator();
    ImGui::Text("------REUNION-------");
    static int r0 = 0;
    static int c0 = 0;

    ImGui::InputInt("R", &r0);
    ImGui::InputInt("C", &c0);
    if (ImGui::Button("Fly to Coord"))
    {
        for (auto company : gcs->CompanyList)
        {
            for (auto const& [key, uav] : company->UAV_MAP)
            {
                
                ActionData actMisionPause{ "missionPause"};
                uav->addTask(actMisionPause);
                Geo geo = gridToGeo(Coord((float)r0, (float)c0), gcs->gconf);                               
                ActionData actFlyTo{ "flyTo", geo.lat, geo.lon, 488+30.0f, hdgFromPath(Geo(uav->lat, uav->lon), geo) };
                uav->addTask(actFlyTo);
              
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("----BENCHMARK TESTS----");
    if (ImGui::Button("    Run Test 1     ")) 
    {        
        gcs->runTests("Test1");  
        //gcs->lastCheckTest = runTime;
        testRunning = "Test1";
    }  
    ImGui::SameLine();
    if (ImGui::Button("  Run StressTest   "))
    {
        gcs->runTests("StressTest");
        testRunning = "StressTest";
    }

    if (gcs->testRunning != "None")
    {        
        if (runTime - gcs->lastCheckTest >= 1)
        {           
            gcs->lastCheckTest = runTime;

            gcs->runTests(testRunning);

            if (gcs->runningTest == false)
            {
                gcs->generateStats(testRunning + ".csv");
                testRunning = "None";
            }
        }
    } 

    ImGui::Text("Progress: %d / %d (%s)", gcs->testFase, gcs->testTotal, gcs->testRunning);

    if (ImGui::Button("Show Results"))
    {
        gcs->generateStats("out.csv");
    }

    ImGui::Separator();
    ImGui::Text("Deliveries");
    if (ImGui::Button("Load Requests"))
    {

    }

    ImGui::End();
   //processRequests();
}


void GridView::draw(NVGcontext* vg)
{
    NVGpaint img = nvgImagePattern(vg, 0, 0, gconf.gridSize + 1, gconf.gridSize + 1, 0, checker->image, 1.0f);
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, 800, 800);
    nvgFillPaint(vg, img);
    nvgFill(vg);

    nvgTranslate(vg, 400 - gconf.g_gridSize * gconf.gridSize / 2, 400 - gconf.g_gridSize * gconf.gridSize / 2);
    
    // grid
    for (int i = 0; i < gconf.gridSize; ++i)
    {
        auto p1 = coordPosition(-1, i, gconf);
        auto p2 = coordPosition(gconf.gridSize, i, gconf);
        auto p3 = coordPosition(i, -1, gconf);
        auto p4 = coordPosition(i, gconf.gridSize, gconf);

        nvgBeginPath(vg);
        nvgMoveTo(vg, p1.x, p1.y);
        nvgLineTo(vg, p2.x, p2.y);
        nvgStrokeColor(vg, Grays::Gray30);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);
        nvgBeginPath(vg);
        nvgMoveTo(vg, p3.x, p3.y);
        nvgLineTo(vg, p4.x, p4.y);
        nvgStrokeColor(vg, Grays::Gray30);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);
        nvgFontSize(vg, 15);
        nvgFontFace(vg, "roboto-bold");
        nvgFillColor(vg, Whites::White);
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        std::string rtxt = "R" + std::to_string(i);
        std::string ctxt = "C" + std::to_string(i);
        nvgText(vg, p3.x - 12, p3.y, rtxt.c_str(), nullptr);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
        nvgText(vg, p2.x, p2.y + 10, ctxt.c_str(), nullptr);
    }
    
    board.draw(vg);
    
    for (auto& p : SLOTs)
    {
        nvgSave(vg);
        p.draw(vg);
        nvgRestore(vg);
    }


   for (auto const& [key, uav] : uavDrawers)
   {
        nvgSave(vg);
        uav->draw(vg);
        nvgRestore(vg);  
   }

}

