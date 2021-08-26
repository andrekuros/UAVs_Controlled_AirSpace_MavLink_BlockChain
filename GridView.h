#ifndef __GRID_VIEW__
#define __GRID_VIEW__

#include <Mahi/Gui.hpp>
#include <Mahi/Util.hpp>
#include <Mahi/Util/Range.hpp>

#include "GCS_UTM.h"

#include <iostream>
#include <future>
#include <memory>
#include <thread>
#include <map>
#include <deque>
#include <set>
#include <fstream>
#include <utility>
#include <algorithm>
#include <random>
#include <windows.h>
#include <chrono>
#include <cstdint>


using namespace mahi::gui;
using namespace mahi::util;

using std::size_t;
using std::vector;
using std::pair;


/// SLOT_draw colors
const vector<Color> g_colors{ Oranges::OrangeRed, Greens::SpringGreen,   Reds::LightCoral,
                             Yellows::Yellow,     Greens::YellowGreen,  Greens::Chartreuse,
                             Greens::LawnGreen,   Cyans::LightSeaGreen, Blues::DeepSkyBlue,
                             Blues::DodgerBlue,   Blues::SteelBlue,     Purples::BlueViolet,
                             Purples::Magenta,    Pinks::DeepPink };



/*Shape makeShape() {
    // create square and octagon primitives
    auto shape = make_rectangle_shape(g_sideLength * 2, g_sideLength * 2);
    /*auto oct = make_polygon_shape(8, g_sideLength, PolyParam::SideLength);
    oct.rotate(360.0f / 16.0f);
    // create que of shapes to merge
    std::deque<Shape> shapes;
    for (auto & r : range(size(mat,0))) {
        for (auto & c : range(size(mat,1))) {
            if (mat[r][c] == 4)
                shapes.push_back(sqr);
            else if (mat[r][c] == 8)
                shapes.push_back(oct);
            oct.move(g_gridSize, 0);
            sqr.move(g_gridSize, 0);
        }
        oct.move(-g_gridSize * size(mat,1), g_gridSize);
        sqr.move(-g_gridSize * size(mat,1), g_gridSize);
    }
    // marge shapes
    Shape shape;
    while (!shapes.empty()) {
        auto toMerge = shapes.front();
        shapes.pop_front();
        auto merged = clip_shapes(shape, toMerge, ClipType::Union);
        if (merged.size() == 1)
            shape = merged[0];
        else
            shapes.push_back(toMerge);
    }
    return shape;
};*/

class GridView;
class Board {
public:
    Board(GridView* grid, GridConfig gconf);
    std::string Board::matToStr(std::vector<std::vector<Cell*>> mat);

    void draw(NVGcontext* vg);
    
    Shape        shape;   
    Color        color;
    GridConfig gconf;
    GridView* grid;
};


//==============================================================================
// SLOT_draw CLASS
//==============================================================================

class SLOT_draw : public Transformable {
public:
    SLOT_draw(Application* app, Color col, GridConfig gconf, GCS_UTM* gcs);

    void setColor(Color _col, float _alpha) {
        color = _col;
        alpha = _alpha;
    };

    /*Vec2 computeScale(int perm) { return perm < 4 ? Vec2(1, 1) : Vec2(-1, 1); }

    Vec2 computeOrigin(int perm) {
        return gconf.g_gridSize *
            Vec2((perm == 2 || perm == 3 || perm == 4 || perm == 5) ? (float)size(matrix, 1) - 1
                : 0.0f,
                (perm == 1 || perm == 2 || perm == 5 || perm == 6) ? (float)size(matrix, 0) - 1
                : 0.0f);
    }

    float computeRotation(int perm) { return 90.0f * (float)(perm % 4); }
    */
    void place(const Coord& new_coord, int new_perm) 
    {
        Pos pos = coordPosition(new_coord, gconf);
        set_pos(pos.x, pos.y);
        set_origin(Vec2(0, 0));
        set_scale(Vec2(1, 1));
        set_rotation(0);
        coord = new_coord;
        perm = new_perm;
    }

    void draw(NVGcontext* vg);

    Application* app;
    Shape        shape;
    bool         transitioning = false;
    int          perm;
    Coord        coord;
    //Matrix       matrix;
    Color        color;
    float        alpha;
    GridConfig gconf;
    GCS_UTM* gcs;
};

//==============================================================================
// UAV_draw CLASS
//==============================================================================

class UAV_draw : public Transformable {
public:
    UAV_draw(Application* app, Color col, GridConfig gconf);


    void setColor(Color _col, float _alpha) {
        color = _col;
        alpha = _alpha;
    };

    float computeRotation(int perm) { return 90.0f * (float)(perm % 4); }

    void place(const Coord& new_coord, double rot) 
    {
        Pos pos = coordPosition(new_coord, gconf);
        set_pos(Vec2(pos.x,pos.y));
        Pos pOrg = coordPosition({ 0,0 }, gconf);
        set_origin(pOrg.x, pOrg.y);
        //set_scale(computeScale(new_perm));
        set_rotation(rot);
        coord = new_coord;
        
    }

    void draw(NVGcontext* vg);

    Application* app;
    Shape        shape;
    Coord        coord;
    Color        color;
    float        alpha;
    int company = -1;
    int sysId = 0;
    GridConfig gconf;
};


//==============================================================================
// GridView
//==============================================================================

class GridView : public Application {
public:
    GridView(Config conf, GCS_UTM* gcs);
    ~GridView() { nvgluDeleteFramebuffer(checker); }

    void update();    
    void draw(NVGcontext* vg) override;
    void GridView::Run();

    GridConfig gconf;


    /// Makes a matrix of all zeros, size r x c
    //inline Matrix zeros(size_t r, size_t c);

    //inline void sparseToDense(const Matrix& sparse, Matrix& dense);

    //inline void printMat(const Matrix& mat);

    int               ms_solve = -1;
    Board             board;
    vector<SLOT_draw> SLOTs;
    

    NVGLUframebuffer* checker = NULL;
    void createCheckerBoard();   
    std::map<std::string, UAV_draw*> uavDrawers;
    bool showSlots{ false };
    bool runningTest{ false };
    std::string testRunning = "None";

    GCS_UTM* gcs;

};

#endif

