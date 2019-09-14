# distutils: language = c++

from libcpp.string cimport string
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp cimport bool

import os

cdef extern from "bag.hpp" namespace "bag":
    cdef cppclass Layout:
        Layout()

        void add_inst(const string & lib_name, const string & cell_name,
                      const string & view_name, const string & inst_name,
                      double xc, double yc, const string & orient,
                      const map[string, int] int_params, const map[string, string] str_params,
                      const map[string, double] double_params, int num_rows,
                      int num_cols, double sp_rows, double sp_cols) except +
        
        void add_rect(const string & lay_name, const string & purp_name,
                      double xl, double yb, double xr, double yt,
                      unsigned int nx, unsigned int ny,
                      double spx, double spy) except +

        void add_path_seg(const string & lay_name, const string & purp_name,
                          double x0, double y0, double x1, double y1,
                          double width, const string & begin_style,
                          const string & end_style) except +

        void add_via(const string & via_name, double xc, double yc,
                     const string & orient, unsigned int num_rows,
                     unsigned int num_cols, double sp_rows, double sp_cols,
                     double enc1_xl, double enc1_yb, double enc1_xr, double enc1_yt,
                     double enc2_xl, double enc2_yb, double enc2_xr, double enc2_yt,
                     double cut_width, double cut_height,
                     unsigned int nx, unsigned int ny,
                     double spx, double spy) except +

        void add_pin(const string & net_name, const string & pin_name,
                     const string & label, const string & lay_name,
                     const string & purp_name, double xl, double yb,
                     double xr, double yt, bool make_pin_obj) except +

        void add_polygon(const string & lay_name, const string & purp_name,
                         const vector[double] & xcoord, const vector[double] & ycoord) except +
        
        void add_blockage(const string & btype, const string & layer, const vector[double] & xcoord,
                          const vector[double] & ycoord) except +

        void add_boundary(const string & btype, const vector[double] & xcoord,
                  const vector[double] & ycoord) except +
        
    cdef cppclass SchInst:
        SchInst()
        string inst_name, lib_name, cell_name
        map[string, string] params
        map[string, string] term_map
        
    cdef cppclass SchCell:
        SchCell()
        string lib_name, cell_name, new_cell_name
        map[string, string] pin_map
        map[string, vector[SchInst]] inst_map


cdef extern from "bagoa.hpp" namespace "bagoa":
    cdef cppclass LibDefObserver:
        pass

    cdef cppclass OALayoutLibrary:
        OALayoutLibrary()
        void open_library(const string & lib_file, const string & library,
                          const string & lib_path, const string & tech_lib) except +
        void add_purpose(const string & purp_name, unsigned int purp_num) except +
        void add_layer(const string & lay_name, unsigned int lay_num) except +
        void close() except +
        void create_layout(const string & cell, const string & view, const Layout & layout) except +

    cdef cppclass OASchematicWriter:
        OASchematicWriter()
        void open_library(const string & lib_path, const string & library) except +
        void close() except +
        void create_schematics(const vector[SchCell] & cell_list, const string & sch_name,
                               const string & sym_name) except +


cdef class PyLayout:
    cdef Layout c_layout
    cdef unicode encoding
    def __init__(self, unicode encoding):
        self.encoding = encoding

    def add_inst(self, unicode lib, unicode cell, unicode view,
                 unicode name, object loc, unicode orient, object params=None,
                 int num_rows=1, int num_cols=1, double sp_rows=0.0,
                 double sp_cols=0.0):
        cdef map[string, int] int_map
        cdef map[string, string] str_map
        cdef map[string, double] double_map
        cdef string lib_name = lib.encode(self.encoding)
        cdef string cell_name = cell.encode(self.encoding)
        cdef string view_name = view.encode(self.encoding)
        cdef string inst_name = name.encode(self.encoding)
        cdef string c_orient = orient.encode(self.encoding)
        cdef string par_key
        cdef double xo = loc[0]
        cdef double yo = loc[1]
        if params is not None:
            for key, val in params.items():
                par_key = key.encode(self.encoding)
                if isinstance(val, bytes):
                    str_map[par_key] = val
                elif isinstance(val, unicode):
                    str_map[par_key] = val.encode(self.encoding)
                elif isinstance(val, int):
                    int_map[par_key] = val
                elif isinstance(val, float):
                    double_map[par_key] = val

        self.c_layout.add_inst(lib_name, cell_name, view_name,
                               inst_name, xo, yo,
                               c_orient, int_map, str_map,
                               double_map, num_rows, num_cols,
                               sp_rows, sp_cols)
        
    def add_rect(self, object layer, object bbox,
                 int arr_nx=1, int arr_ny=1,
                 double arr_spx=0.0, double arr_spy=0.0):
        cdef string lay = layer[0].encode(self.encoding)
        cdef string purp = layer[1].encode(self.encoding)
        cdef double xl = bbox[0][0]
        cdef double yb = bbox[0][1]
        cdef double xr = bbox[1][0]
        cdef double yt = bbox[1][1]
        self.c_layout.add_rect(lay, purp, xl, yb, xr, yt, arr_nx, arr_ny,
                               arr_spx, arr_spy)

    def add_polygon(self, object layer, list points):
        cdef string lay = layer[0].encode(self.encoding)
        cdef string purp = layer[1].encode(self.encoding)
        cdef vector[double] xcoord
        cdef vector[double] ycoord
        for xval, yval in points:
            xcoord.push_back(xval)
            ycoord.push_back(yval)

        self.c_layout.add_polygon(lay, purp, xcoord, ycoord)

    def add_blockage(self, unicode btype, unicode layer, list points):
        cdef string btype_c = btype.encode(self.encoding)
        cdef string layer_c = layer.encode(self.encoding)
        cdef vector[double] xcoord
        cdef vector[double] ycoord
        for xval, yval in points:
            xcoord.push_back(xval)
            ycoord.push_back(yval)

        self.c_layout.add_blockage(btype_c, layer_c, xcoord, ycoord)

    def add_boundary(self, unicode btype, list points):
        cdef string btype_c = btype.encode(self.encoding)
        cdef vector[double] xcoord
        cdef vector[double] ycoord
        for xval, yval in points:
            xcoord.push_back(xval)
            ycoord.push_back(yval)

        self.c_layout.add_boundary(btype_c, xcoord, ycoord)

    def add_path(self, object layer, double width, list points,
                 unicode end_style, unicode join_style):
        cdef string lay = layer[0].encode(self.encoding)
        cdef string purp = layer[1].encode(self.encoding)
        cdef string estyle = end_style.encode(self.encoding)
        cdef string jstyle = join_style.encode(self.encoding)
        cdef string start_s, stop_s
        cdef double x0 = 0
        cdef double y0 = 0
        cdef double x1 = 0
        cdef double y1 = 0
        cdef int plen = len(points)
        cdef int idx
        for idx, p in enumerate(points):
            x1, y1 = p[0], p[1]
            if idx > 0:
                start_s = stop_s = jstyle
                if idx == 1:
                    start_s = estyle
                if idx == plen - 1:
                    stop_s = estyle
                self.c_layout.add_path_seg(lay, purp, x0, y0, x1, y1, width,
                                           start_s, stop_s)
            x0, y0 = x1, y1

    def add_via(self, unicode id, loc, unicode orient,
                int num_rows, int num_cols, double sp_rows, double sp_cols,
                object enc1, object enc2, double cut_width=-1, double cut_height=-1,
                int arr_nx=1, int arr_ny=1, double arr_spx=0.0,
                double arr_spy=0.0):
        cdef string via_name = id.encode(self.encoding)
        cdef string via_orient = orient.encode(self.encoding)
        cdef double xo = loc[0]
        cdef double yo = loc[1]
        cdef double xl1 = enc1[0]
        cdef double yb1 = enc1[3]
        cdef double xr1 = enc1[1]
        cdef double yt1 = enc1[2]
        cdef double xl2 = enc2[0]
        cdef double yb2 = enc2[3]
        cdef double xr2 = enc2[1]
        cdef double yt2 = enc2[2]
        self.c_layout.add_via(via_name, xo, yo, via_orient, 
                              num_rows, num_cols, sp_rows, sp_cols,
                              xl1, yb1, xr1, yt1,
                              xl2, yb2, xr2, yt2,
                              cut_width, cut_height, arr_nx, arr_ny, arr_spx, arr_spy)

    def add_pin(self, unicode net_name, unicode pin_name, unicode label, object layer,
                object bbox, bool make_rect=True):
        cdef string c_net = net_name.encode(self.encoding)
        cdef string c_pin = pin_name.encode(self.encoding)
        cdef string c_label = label.encode(self.encoding)
        cdef string lay = layer[0].encode(self.encoding)
        cdef string purp = layer[1].encode(self.encoding)
        cdef double xl = bbox[0][0]
        cdef double yb = bbox[0][1]
        cdef double xr = bbox[1][0]
        cdef double yt = bbox[1][1]
        self.c_layout.add_pin(c_net, c_pin, c_label, lay, purp,
                              xl, yb, xr, yt,
                              make_rect)
        
cdef class PyOALayoutLibrary:
    cdef OALayoutLibrary c_lib
    cdef string lib_file
    cdef string library
    cdef string lib_path
    cdef string tech_lib
    cdef unicode encoding
    def __init__(self, lib_file, library, lib_path, tech_lib, encoding):
        if not lib_path:
            lib_path = os.getcwd()
        lib_path = os.path.join(os.path.abspath(lib_path), library)
        self.lib_file = lib_file.encode(encoding)
        self.library = library.encode(encoding)
        self.lib_path = lib_path.encode(encoding)
        self.tech_lib = tech_lib.encode(encoding)
        self.encoding = encoding
    
    def __enter__(self):
        self.c_lib.open_library(self.lib_file, self.library, self.lib_path, self.tech_lib)
        return self

    def __exit__(self, *args):
        self.close()
        
    def __del__(self):
        self.close()
            
    def close(self):
        self.c_lib.close()
        
    def add_purpose(self, unicode purp_name, int purp_num):
        cdef string purp = purp_name.encode(self.encoding)
        self.c_lib.add_purpose(purp, purp_num)

    def add_layer(self, unicode lay_name, int lay_num):
        cdef string lay = lay_name.encode(self.encoding)
        self.c_lib.add_layer(lay, lay_num)

    def create_layout(self, unicode cell, unicode view, PyLayout layout):
        cdef string cname = cell.encode(self.encoding)
        cdef string vname = view.encode(self.encoding)
        self.c_lib.create_layout(cname, vname, layout.c_layout)


cdef class PySchCell:
    cdef SchCell c_inst
    cdef unicode encoding
    def __init__(self, unicode lib_name, unicode cell_name, unicode new_cell_name, unicode encoding):
        self.encoding = encoding
        self.c_inst.lib_name = lib_name.encode(encoding)
        self.c_inst.cell_name = cell_name.encode(encoding)
        self.c_inst.new_cell_name = new_cell_name.encode(encoding)

    def rename_pin(self, unicode old_name, unicode new_name):
        cdef string oname = old_name.encode(self.encoding)
        cdef string nname = new_name.encode(self.encoding)
        self.c_inst.pin_map[oname] = nname

    def add_inst(self, unicode inst_name, unicode default_lib, object inst_list):
        cdef vector[SchInst] cinst_list
        cdef SchInst cur_inst
        cdef unicode actual_lib
        cdef string iname = inst_name.encode(self.encoding)
        for inst in inst_list:
            cur_inst = SchInst()
            actual_lib = inst['lib_name']
            if actual_lib is None:
                actual_lib = default_lib
            cur_inst.inst_name = inst['name'].encode(self.encoding)
            cur_inst.lib_name = actual_lib.encode(self.encoding)
            cur_inst.cell_name = inst['cell_name'].encode(self.encoding)
            for name, value in inst['params']:
                cur_inst.params[name.encode(self.encoding)] = value.encode(self.encoding)
            for name, value in inst['term_mapping']:
                cur_inst.term_map[name.encode(self.encoding)] = value.encode(self.encoding)

            cinst_list.push_back(cur_inst)

        self.c_inst.inst_map[iname] = cinst_list


cdef class PyOASchematicWriter:
    cdef OASchematicWriter c_writer
    cdef vector[SchCell] c_cell_list
    cdef string lib_path
    cdef string library
    cdef unicode encoding
    def __init__(self, unicode lib_path, unicode library, unicode encoding):
        self.lib_path = lib_path.encode(encoding)
        self.library = library.encode(encoding)
        self.encoding = encoding
    
    def __enter__(self):
        self.c_writer.open_library(self.lib_path, self.library)
        return self

    def __exit__(self, *args):
        self.close()
        
    def __del__(self):
        self.close()
            
    def close(self):
        self.c_writer.close()

    def add_sch_cell(self, PySchCell cell):
        self.c_cell_list.push_back(cell.c_inst)
    
    def create_schematics(self, unicode sch_name, unicode sym_name):
        cdef string c_sch_name = sch_name.encode(self.encoding)
        cdef string c_sym_name = sym_name.encode(self.encoding)
        self.c_writer.create_schematics(self.c_cell_list, c_sch_name,
                                        c_sym_name)
