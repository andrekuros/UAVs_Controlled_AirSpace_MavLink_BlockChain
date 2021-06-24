#include "GCSMav_control.h"


//==============================================================================
// BOARD
//==============================================================================
Board::Board(Application* app) : app(app), color(Grays::Gray50) {
    shape.set_point_count(4);
    shape.set_point(0, coordPosition(-1, -1));
    shape.set_point(1, coordPosition(gridSize, -1));
    shape.set_point(2, coordPosition(gridSize, gridSize));
    shape.set_point(3, coordPosition(-1, gridSize));
    // shape.set_radii(g_gridSize);
    // auto hole = makeShape(matrix);
    // hole = offset_shape(hole, 2.0f);
    // shape.push_back_hole(hole);
}

void Board::draw(NVGcontext* vg) {
    float trans[6], inv[6];
    nvgCurrentTransform(vg, trans);
    nvgTransformInverse(inv, trans);
    Vec2 mouse = app->get_mouse_pos();
    nvgTransformPoint(&mouse.x, &mouse.y, inv, mouse.x, mouse.y);
     bool hovered = shape.contains(mouse);

    if (hovered) {
        ImGui::BeginTooltip();
        ImGui::Text("SLOTSs");
        ImGui::Separator();
        auto matStr = matToStr(Slots);
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
SLOT_draw::SLOT_draw(Application* app, Color col) : app(app), color(col)
{
    shape = make_rectangle_shape(g_sideLength * 2, g_sideLength * 2);
    shape = offset_shape(shape, -4.0f / gridCorrection);
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
    if (hovered) {
        ImGui::BeginTooltip();
        ImGui::TextColored(color, "SLOT");
        ImGui::Text("R:%f C:%f", coord.r, coord.c);
        ImGui::Text("Lat:%f Lon:%f", pos().x, pos().y);
        ImGui::Separator();
        ImGui::EndTooltip();
    }

    if (Slots[coord.r][coord.c] != 0)
        alpha = 0.7;
    else
        alpha = 0.1;

    if (Slots[coord.r][coord.c] > 1) 
        color = Reds::FireBrick;
    else
        color = Blues::LightBlue;

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

UAV_draw::UAV_draw(Application* app, Color col) : app(app), color(col)
{
    shape = make_polygon_shape(3,15, mahi::gui::PolyParam::SideLength);
    
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
        ImGui::Text("R:%f C:%f", coord.r, coord.c);  
        ImGui::Text("Lat:%f Lon:%f", pos().x, pos().y);        
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

GridView::GridView(Config conf, std::vector<Company*>  cList) : Application(conf), board(this), companyList(cList) {
    SLOTs.reserve(gridSize * gridSize);

    
    for (size_t i = 0; i < gridSize; i++) {
        for (size_t j = 0; j < gridSize; j++) {
            auto slot = SLOT_draw(this, Grays::DimGray);
            slot.setColor(Blues::DeepSkyBlue, 0.10);
            Coord newCoord{ i, j };
            slot.place(newCoord, 0);
            SLOTs.emplace_back(std::move(slot));
        }
    }
    // solve
    set_vsync(false);
    nvgCreateFontMem(m_vg, "roboto-bold", Roboto_Bold_ttf, Roboto_Bold_ttf_len, 0);
    createCheckerBoard();
}

void GridView::createCheckerBoard() {
    checker = nvgluCreateFramebuffer(m_vg, gridSize+1, gridSize+1, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
    nvgluBindFramebuffer(checker);
    glViewport(0, 0, gridSize+1, gridSize+1);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    nvgBeginFrame(m_vg, gridSize+1, gridSize+1, 1);
    nvgBeginPath(m_vg);
    nvgRect(m_vg, 0, 0, gridSize/2, gridSize/2);
    nvgFillColor(m_vg, Grays::Gray10);
    nvgFill(m_vg);
    nvgBeginPath(m_vg);
    nvgRect(m_vg, gridSize/2, gridSize/2, gridSize+1, gridSize+1);
    nvgFillColor(m_vg, Grays::Gray10);
    nvgFill(m_vg);
    nvgEndFrame(m_vg);
    nvgluBindFramebuffer(nullptr);
}

void GridView::update()
{
    ImGui::Begin("GridView");
    ImGui::SameLine();    
    ImGui::EndDisabled(); 
    ImGui::Text("Num Sys: %d", NumSys);    
    ImGui::Separator();
    if (ImGui::Button("Update Systems"))
        checkSystems();
    
    Slots.clear();
    Slots = zeros(gridSize, gridSize);
    
    for (auto company : companyList)
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
            ImGui::Text("ID: %3d | Lat: %f | Lon %f | Hdg: %5.1f", uav->SysId, uav->lat, uav->lon, uav->hdg);
            //ImGui::SameLine();

            ImGui::SameLine();
            if (ImGui::Button(uav->isArmed ? "TakeOff" : "Arm")) 
            {
                if (uav->isArmed)
                    uav->action->takeoff_async([](Action::Result result) {});
                else
                    uav->action->arm_async([](Action::Result result) {});
            }
                
            ImGui::SameLine();
            if (ImGui::Button("RND Mission"))
            {
                uav->createMission("random");
                uav->missionManager->upload_mission_async(*uav->nextMission, [](Mission::Result result) {
                    std::cout << "\nUpload mission result: " << result;
                    });
                //uav->missionManager->start_mission_async([](Mission::Result result) {});
            }
            //static int r0 = 0;
            //static int c0 = 0;
            //ImGui::SameLine();
            //ImGui::InputInt("r", &r0);                        
            //if (ImGui::Button("Fly to Coord")) { }
            //ImGui::InputInt("c", &c0);
            ImGui::SameLine();
            if (ImGui::Button("Start"))
            {
                uav->missionManager->start_mission_async([](Mission::Result result) {
                    std::cout << "\nMission Start result: " << result;
                    });
            }
            
            ImGui::PopItemWidth();
            ImGui::PopID();

            //Update UAV Data
            Coord uavPos = coordToGrid({ uav->lat, uav->lon });
            uav->drawer->place(uavPos, uav->hdg);
            int r = round(uav->drawer->coord.r);
            int c = round(uav->drawer->coord.c);
            if (c >= 0 && r >= 0 && c < gridSize && r < gridSize)
                Slots[r][c]++;// = uav->SysId;
        
        }
        ImGui::Separator();
    }
    ImGui::End();

    //processRequests();
}

void GridView::checkSystems() 
{    
    for (auto company : companyList)
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

                uav->telemetry = new Telemetry{ uav->system };
                uav->action = new Action{ uav->system };
                uav->missionManager = new Mission{ uav->system };

                uav->telemetry->set_rate_position(5.0);
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

                uav->telemetry->subscribe_velocity_ned([uav](Telemetry::VelocityNed velNed)
                    {
                        uav->hdg = atan2(velNed.east_m_s, velNed.north_m_s) * 180.0 / 3.1416;
                    });

                auto u = new UAV_draw(this, Grays::DarkSlateGray);
                u->company = company->cod;
                u->sysId = uav->SysId;
                u->setColor(g_colors[company->cod], 0.8);
                uav->company = company->cod;
                u->place({ 0,0 }, 0);
                uav->drawer = u;
            }
        }
    }

}

void GridView::draw(NVGcontext* vg)
{
    NVGpaint img = nvgImagePattern(vg, 0, 0, gridSize+1, gridSize+1, 0, checker->image, 1.0f);
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, 800, 800);
    nvgFillPaint(vg, img);
    nvgFill(vg);

    nvgTranslate(vg, 400 - g_gridSize * gridSize/2, 400 - g_gridSize * gridSize / 2);

    // grid
    for (int i = 0; i < gridSize; ++i) 
    {       
        auto p1 = coordPosition(-1, i);
        auto p2 = coordPosition(gridSize, i);
        auto p3 = coordPosition(i, -1);
        auto p4 = coordPosition(i, gridSize);
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
        nvgText(vg, p3.x - 12 , p3.y, rtxt.c_str(), nullptr);
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


    for (auto company : companyList)
        for (auto const& [key, uav] : company->UAV_MAP)
        {
            nvgSave(vg);
            uav->drawer->draw(vg);
            nvgRestore(vg);   
        }
  
}

/// Makes a matrix of all zeros, size r x c
inline Matrix GridView::zeros(size_t r, size_t c) { return Matrix(r, Row(c, 0)); }

inline void GridView::sparseToDense(const Matrix& sparse, Matrix& dense) {
    auto numRows = size(sparse, 0);
    auto numCols = size(sparse, 1);
    dense.assign(numRows, {});
    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numCols; ++c) {
            if (sparse[r][c])
                dense[r].push_back((Num)c);
        }
    }
}


inline void GridView::printMat(const Matrix& mat) {
    for (auto& row : mat) {
        for (auto& val : row) {
            if (val == 0)
                std::cout << ".  ";
            else
                std::cout << val << "  ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

Coord GridView::coordToGrid(Vec2 pos)
{    
    return { -(pos.x - refLat) / refStep, (pos.y - refLon) / refStep};
}


int main(int argc, char const* argv[]) 
{
             
    //Mavsdk é a classe base da biblioteca que permite definir diversos parâmetros como
    //configurações de rede, tipos de sistema que pretende representar, id desse sistema
    //entreo outros.
    std::vector<Company*> CompanyList;
        
    for (int i = 0; i < numCompanies; i++)
    {
        Mavsdk* mavsdk = new Mavsdk();        
        ConnectionResult connection_result = mavsdk->add_any_connection("udp://0.0.0.0:1454" + std::to_string(i));
        mavsdk->subscribe_on_new_system([&mavsdk]() {});
        Company* aux_company = new Company();
        aux_company->mavsdk = mavsdk;
        aux_company->cod = i + 1;
        CompanyList.push_back(aux_company);
        std::cout << "Connection result: " << connection_result << '\n';        
    }
      
    Slots = zeros(gridSize, gridSize);
    
    Application::Config conf;
    conf.width = 800;
    conf.height = 800;
    conf.resizable = false;
    conf.title = "Grid View";
    conf.msaa = false;
    GridView grid(conf, CompanyList);
                 
    grid.run();
    return 0;
}






