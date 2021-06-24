#include <Mahi/Gui.hpp>
#include <Mahi/Util.hpp>
#include <Mahi/Util/Range.hpp>

#include "UAV_MAV.h"

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
#include <mavsdk/mavsdk.h>
#include <mavsdk/system.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/param/param.h>
#include <mavsdk/plugins/info/info.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

using std::size_t;
using std::vector;
using std::pair;

using namespace mahi::gui;
using namespace mahi::util;
using namespace mavsdk;


//==============================================================================
// MATRIX OPERATIONS
//==============================================================================

typedef int         Num;
typedef vector<Num> Row;     // Matrix row
typedef vector<Row> Matrix;  // 2D Matrix


/// Makes a matrix of all zeros, size r x c
inline Matrix zeros(size_t r, size_t c) {
    return Matrix(r, Row(c, 0));
}
                             /// Returns the size of a matrix along a dimension
inline size_t size(const Matrix& mat, int dim) {
    if (dim == 0)
        return mat.size();
    else if (dim == 1)
        return mat[0].size();
    return 0;
}


std::string matToStr(const Matrix& mat) {
    std::stringstream ss;
    for (auto& row : mat) {
        for (auto& val : row) {
            if (val == 0)
                ss << ". ";
            else
                ss << val << " ";
        }
        ss << std::endl;
    }
    return ss.str();
}


/// Represents a coordinate position in the Board
struct Coord {
    float r;
    float c;
};

//==============================================================================
// PARAMS
//==============================================================================

int NumSys = 0;
int numCompanies = 3;
float refLat = 47.397742;
float refLon = 8.545594;
float refAlt = 488.0;
float refArea = 0.025;
int gridSize = 20;
float gridCorrection = (float)gridSize / 15.0;
float refStep = refArea / gridSize;

Matrix Slots;

float g_sideLength = 25.0f / gridCorrection;
float g_gridSize = (2.0f + (float)SQRT2) * 0.5f * g_sideLength * 0.999f;


/// SLOT_draw colors
const vector<Color> g_colors{Oranges::OrangeRed, Greens::SpringGreen,   Reds::LightCoral,
                             Yellows::Yellow,     Greens::YellowGreen,  Greens::Chartreuse,
                             Greens::LawnGreen,   Cyans::LightSeaGreen, Blues::DeepSkyBlue,
                             Blues::DodgerBlue,   Blues::SteelBlue,     Purples::BlueViolet,
                             Purples::Magenta,    Pinks::DeepPink };

//==============================================================================
// HELPERS
//==============================================================================

inline Vec2 coordPosition(const Coord& coord) {
    return Vec2(coord.c * g_gridSize, coord.r * g_gridSize);
}

inline Vec2 coordPosition(float i, float j) { return coordPosition({ i, j }); }

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


class Board {
public:
    Board(Application* app);

    void draw(NVGcontext* vg);

    Application* app;
    Shape        shape;
    Matrix       matrix;
    Color        color;
};

class Company {
public:
    Company() {};
    Mavsdk* mavsdk = nullptr;
    std::map<int, UAV_MAV*> UAV_MAP;
    std::string name = "Company";
    int cod = 0;
};


//==============================================================================
// SLOT_draw CLASS
//==============================================================================

class SLOT_draw : public Transformable {
public:
    SLOT_draw(Application* app, Color col);

    void setColor(Color _col, float _alpha) {
        color = _col;
        alpha = _alpha;
    };

    Vec2 computeScale(int perm) { return perm < 4 ? Vec2(1, 1) : Vec2(-1, 1); }

    Vec2 computeOrigin(int perm) {
        return g_gridSize *
            Vec2((perm == 2 || perm == 3 || perm == 4 || perm == 5) ? (float)size(matrix, 1) - 1
                : 0.0f,
                (perm == 1 || perm == 2 || perm == 5 || perm == 6) ? (float)size(matrix, 0) - 1
                : 0.0f);
    }

    float computeRotation(int perm) { return 90.0f * (float)(perm % 4); }

    void place(const Coord& new_coord, int new_perm) {
        set_pos(coordPosition(new_coord));
        set_origin(computeOrigin(new_perm));
        set_scale(computeScale(new_perm));
        set_rotation(computeRotation(new_perm));
        coord = new_coord;
        perm = new_perm;
    }

    void draw(NVGcontext* vg);

    Application* app;
    Shape        shape;
    bool         transitioning = false;
    int          perm;
    Coord        coord;
    Matrix       matrix;
    Color        color;
    float        alpha;
};

//==============================================================================
// UAV_draw CLASS
//==============================================================================

class UAV_draw : public Transformable {
public:
    UAV_draw(Application* app, Color col);

    
    void setColor(Color _col, float _alpha) {
        color = _col;
        alpha = _alpha;
    };
    
    float computeRotation(int perm) { return 90.0f * (float)(perm % 4); }

    void place(const Coord& new_coord, double rot) {
        set_pos(coordPosition(new_coord));
        set_origin(coordPosition({ 0,0 }));
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
};


//==============================================================================
// GridView
//==============================================================================

class GridView : public Application {
public:
    GridView(Config conf, std::vector<Company*> companyList);
    ~GridView() { nvgluDeleteFramebuffer(checker); }

    void update();
    void checkSystems();
    void draw(NVGcontext* vg) override;


    /// Makes a matrix of all zeros, size r x c
    inline Matrix zeros(size_t r, size_t c);

    inline void sparseToDense(const Matrix& sparse, Matrix& dense);  

    inline void printMat(const Matrix& mat);

    Coord coordToGrid(Vec2 pos);

    int               ms_solve = -1;
    Board             board;
    vector<SLOT_draw> SLOTs;
    bool              spinning = false;
    bool              animating = false;
    bool              toggled = true;
    float             percent = 0;
    NVGLUframebuffer* checker = NULL;
    void createCheckerBoard();
    
    std::vector<Company*> companyList;
    
};



