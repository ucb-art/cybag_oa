#ifndef BAG_H_
#define BAG_H_

#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <math.h>
#include <map>
#include <vector>

namespace bag {

/*
 *  Layout related classes
 */

typedef std::map<std::string, unsigned int> LayerMap;
typedef LayerMap::iterator LayerIter;
typedef std::map<std::string, unsigned int> PurposeMap;
typedef PurposeMap::iterator PurposeIter;
typedef std::map<std::string, int> IntMap;
typedef IntMap::const_iterator IntIter;
typedef std::map<std::string, std::string> StrMap;
typedef StrMap::const_iterator StrIter;
typedef std::map<std::string, double> DoubleMap;
typedef DoubleMap::const_iterator DoubleIter;

// a layout instance
struct Inst {
    std::string lib_name;
    std::string cell_name;
    std::string view_name;
    std::string inst_name;
    double loc[2];
    unsigned char orient;
    int num_rows, num_cols;
    double sp_rows, sp_cols;
    IntMap int_params;
    StrMap str_params;
    DoubleMap double_params;
};

typedef std::vector<Inst> InstList;
typedef InstList::const_iterator InstIter;

// a polygon object
struct Polygon {
    std::string layer;
    std::string purpose;
    std::vector<double> xcoord;
    std::vector<double> ycoord;
};

typedef std::vector<Polygon> PolygonList;
typedef PolygonList::const_iterator PolygonIter;
    
// a boundary object
struct Boundary {
    std::string type;
    std::vector<double> xcoord;
    std::vector<double> ycoord;
};

typedef std::vector<Boundary> BoundaryList;
typedef BoundaryList::const_iterator BoundaryIter;

// a layer/area blockage object
struct Blockage {
    std::string layer;
    std::string type;
    std::vector<double> xcoord;
    std::vector<double> ycoord;
};

typedef std::vector<Blockage> BlockageList;
typedef BlockageList::const_iterator BlockageIter;

// a layout rectangle
struct Rect {
    std::string layer;
    std::string purpose;
    double bbox[4];
    int nx, ny;
    double spx, spy;
};

typedef std::vector<Rect> RectList;
typedef RectList::const_iterator RectIter;

// a layout path segment
struct PathSeg {
    std::string layer;
    std::string purpose;
    double x0, y0, x1, y1;
    double width;
    std::string begin_style;
    std::string end_style;
};

typedef std::vector<PathSeg> PathSegList;
typedef PathSegList::const_iterator PathSegIter;

// a layout via
struct Via {
    std::string via_id;
    unsigned char orient;
    double loc[2];
    int num_rows, num_cols;
    double spacing[2];
    double enc1[2];
    double off1[2];
    double enc2[2];
    double off2[2];
    double cut_width, cut_height;
    int nx, ny;
    double spx, spy;
};

typedef std::vector<Via> ViaList;
typedef ViaList::const_iterator ViaIter;

// a layout pin
struct Pin {
    std::string layer;
    std::string purpose;
    double bbox[4];
    std::string term_name;
    std::string pin_name;
    std::string label;
    bool make_pin_obj;
};

typedef std::vector<Pin> PinList;
typedef PinList::const_iterator PinIter;

// a class containing layout information of a cell.
class Layout {
public:
    Layout() {}
    ~Layout() {}

    void add_inst(const std::string & lib_name, const std::string & cell_name,
                  const std::string & view_name, const std::string & inst_name, double xc, double yc,
                  const std::string & orient, const IntMap & int_params, const StrMap & str_params,
                  const DoubleMap & double_params, int num_rows = 1, int num_cols = 1,
                  double sp_rows = 0.0, double sp_cols = 0.0);
    
    void add_rect(const std::string & lay_name, const std::string & purp_name, double xl, double yb,
                  double xr, double yt, unsigned int nx = 1, unsigned int ny = 1, double spx = 0,
                  double spy = 0);
    
    void add_path_seg(const std::string & lay_name, const std::string & purp_name,
                      double x0, double y0, double x1, double y1, double width,
                      const std::string & begin_style, const std::string & end_style);
    
    void add_via(const std::string & via_name, double xc, double yc, const std::string & orient,
                 unsigned int num_rows, unsigned int num_cols, double sp_rows, double sp_cols,
                 double enc1_xl, double enc1_yb, double enc1_xr, double enc1_yt, double enc2_xl,
                 double enc2_yb, double enc2_xr, double enc2_yt, double cut_width = -1,
                 double cut_height = -1, unsigned int nx = 1, unsigned int ny = 1, double spx = 0,
                 double spy = 0);
    
    void add_pin(const std::string & net_name, const std::string & pin_name,
                 const std::string & label, const std::string & lay_name,
                 const std::string & purp_name, double xl, double yb,
                 double xr, double yt, bool make_pin_obj = true);
    
    void add_polygon(const std::string & lay_name, const std::string & purp_name,
                     const std::vector<double> & xcoord, const std::vector<double> & ycoord);
    
    void add_blockage(const std::string & type, const std::string & layer,
                      const std::vector<double> & xcoord, const std::vector<double> & ycoord);
    
    void add_boundary(const std::string & type, const std::vector<double> & xcoord,
                      const std::vector<double> & ycoord);


    InstList inst_list;
    RectList rect_list;
    ViaList via_list;
    PinList pin_list;
    PathSegList path_seg_list;
    PolygonList polygon_list;
    BlockageList block_list;
    BoundaryList boundary_list;
    
};

unsigned char get_orient_code(const std::string & orient_str);

/*
 *  Schematic related classes
 */

// a class representing how to change a schematic instance
class SchInst {
public:
    SchInst() {
    }
    ~SchInst() {
    }

    std::string inst_name;
    std::string lib_name;
    std::string cell_name;
    StrMap params;
    StrMap term_map;
};

// a class representing how to modify a schematic cell
class SchCell {
public:
    SchCell() {
    }
    ~SchCell() {
    }

    std::string lib_name;
    std::string cell_name;
    std::string new_cell_name;
    StrMap pin_map;
    std::map<std::string, std::vector<SchInst> > inst_map;
};

}

#endif
