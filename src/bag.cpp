#include <bag.hpp>

namespace bag {

unsigned char get_orient_code(const std::string & orient_str) {
    if (orient_str == "R0") {
        return 0;
    }
    if (orient_str == "MX") {
        return 1;
    }
    if (orient_str == "MY") {
        return 2;
    }
    if (orient_str == "R180") {
        return 3;
    }
    if (orient_str == "R90") {
        return 4;
    }
    if (orient_str == "MXR90") {
        return 5;
    }
    if (orient_str == "MYR90") {
        return 6;
    }
    if (orient_str == "R270") {
        return 7;
    }
    throw std::invalid_argument("Invalid orientation: " + orient_str);
}

void Layout::add_inst(const std::string & lib_name, const std::string & cell_name,
        const std::string & view_name, const std::string & inst_name, double xc, double yc,
        const std::string & orient, const IntMap & int_params, const StrMap & str_params,
        const DoubleMap & double_params, int num_rows, int num_cols, double sp_rows,
        double sp_cols) {
    Inst obj;
    obj.lib_name = lib_name;
    obj.cell_name = cell_name;
    obj.view_name = view_name;
    obj.inst_name = inst_name;
    obj.loc[0] = xc;
    obj.loc[1] = yc;
    obj.orient = get_orient_code(orient);
    obj.num_rows = num_rows;
    obj.num_cols = num_cols;
    obj.sp_rows = sp_rows;
    obj.sp_cols = sp_cols;
    obj.str_params = str_params;
    obj.int_params = int_params;
    obj.double_params = double_params;

    inst_list.push_back(obj);
}

void Layout::add_rect(const std::string & lay_name, const std::string & purp_name, double xl,
        double yb, double xr, double yt, unsigned int nx, unsigned int ny, double spx, double spy) {
    Rect r;
    r.layer = lay_name;
    r.purpose = purp_name;
    r.bbox[0] = xl;
    r.bbox[1] = yb;
    r.bbox[2] = xr;
    r.bbox[3] = yt;
    r.nx = nx;
    r.ny = ny;
    r.spx = spx;
    r.spy = spy;

    rect_list.push_back(r);
}

void Layout::add_path_seg(const std::string & lay_name, const std::string & purp_name, double x0,
        double y0, double x1, double y1, double width, const std::string & begin_style,
        const std::string & end_style) {
    PathSeg p;
    p.layer = lay_name;
    p.purpose = purp_name;
    p.x0 = x0;
    p.y0 = y0;
    p.x1 = x1;
    p.y1 = y1;
    p.width= width;
    p.begin_style = begin_style;
    p.end_style = end_style;
    path_seg_list.push_back(p);
}

void Layout::add_via(const std::string & via_name, double xc, double yc, const std::string & orient,
        unsigned int num_rows, unsigned int num_cols, double sp_rows, double sp_cols,
        double enc1_xl, double enc1_yb, double enc1_xr, double enc1_yt, double enc2_xl,
        double enc2_yb, double enc2_xr, double enc2_yt, double cut_width, double cut_height,
        unsigned int nx, unsigned int ny, double spx, double spy) {
    Via v;
    v.via_id = via_name;
    v.orient = get_orient_code(orient);
    v.loc[0] = xc;
    v.loc[1] = yc;
    v.num_rows = num_rows;
    v.num_cols = num_cols;
    v.spacing[0] = sp_cols;
    v.spacing[1] = sp_rows;
    v.enc1[0] = (enc1_xr + enc1_xl) / 2.0;
    v.enc1[1] = (enc1_yt + enc1_yb) / 2.0;
    v.off1[0] = (enc1_xr - enc1_xl) / 2.0;
    v.off1[1] = (enc1_yt - enc1_yb) / 2.0;
    v.enc2[0] = (enc2_xr + enc2_xl) / 2.0;
    v.enc2[1] = (enc2_yt + enc2_yb) / 2.0;
    v.off2[0] = (enc2_xr - enc2_xl) / 2.0;
    v.off2[1] = (enc2_yt - enc2_yb) / 2.0;
    v.cut_width = cut_width;
    v.cut_height = cut_height;
    v.nx = nx;
    v.ny = ny;
    v.spx = spx;
    v.spy = spy;

    via_list.push_back(v);
}

void Layout::add_pin(const std::string & net_name, const std::string & pin_name,
        const std::string & label, const std::string & lay_name, const std::string & purp_name,
        double xl, double yb, double xr, double yt, bool make_pin_obj) {
    Pin p;
    p.layer = lay_name;
    p.purpose = purp_name;
    p.bbox[0] = xl;
    p.bbox[1] = yb;
    p.bbox[2] = xr;
    p.bbox[3] = yt;
    p.term_name = net_name;
    p.pin_name = pin_name;
    p.label = label;
    p.make_pin_obj = make_pin_obj;

    pin_list.push_back(p);
}

void Layout::add_polygon(const std::string & lay_name, const std::string & purp_name,
                         const std::vector<double> & xcoord, const std::vector<double> & ycoord) {
    Polygon b;
    b.layer = lay_name;
    b.purpose = purp_name;
    b.xcoord = xcoord;
    b.ycoord = ycoord;

    polygon_list.push_back(b);
}
    
void Layout::add_blockage(const std::string & type, const std::string & layer, const std::vector<double> & xcoord,
        const std::vector<double> & ycoord) {
    Blockage b;
    b.type = type;
    b.layer = layer;
    b.xcoord = xcoord;
    b.ycoord = ycoord;

    block_list.push_back(b);
}

void Layout::add_boundary(const std::string & type, const std::vector<double> & xcoord,
        const std::vector<double> & ycoord) {
    Boundary b;
    b.type = type;
    b.xcoord = xcoord;
    b.ycoord = ycoord;

    boundary_list.push_back(b);
}

}
