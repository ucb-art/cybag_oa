#ifndef BAGOA_H_
#define BAGOA_H_

#include <bag.hpp>

#include "oaDesignDB.h"

// techID = techOpenTechFile(lib_name "tech.oa" "r")
// techGetPurposeNum(techID "pin")

namespace bagoa {

typedef std::map<std::string, oa::oaLayerNum> LayerMap;
typedef LayerMap::iterator LayerIter;
typedef std::map<std::string, oa::oaPurposeNum> PurposeMap;
typedef PurposeMap::iterator PurposeIter;
typedef std::map<std::string, int> IntMap;
typedef IntMap::const_iterator IntIter;
typedef std::map<std::string, std::string> StrMap;
typedef StrMap::const_iterator StrIter;
typedef std::map<std::string, double> DoubleMap;
typedef DoubleMap::const_iterator DoubleIter;

class LibDefObserver: public oa::oaObserver<oa::oaLibDefList> {
public:
    std::string err_msg;

    explicit LibDefObserver(oa::oaUInt4 priority);

    oa::oaBoolean onLoadWarnings(oa::oaLibDefList *obj, const oa::oaString & msg,
            oa::oaLibDefListWarningTypeEnum type);
};

class OALayoutLibrary {
public:
    OALayoutLibrary() :
            is_open(false), dbu_per_uu(1000), mfg_grid_res(1), lib_def_obs(1), lib_ptr(NULL), tech_ptr(NULL) {
    }
    ~OALayoutLibrary() {
    }

    void open_library(const std::string & lib_file, const std::string & library,
                      const std::string & lib_path, const std::string & tech_lib);

    void add_purpose(const std::string & purp_name, unsigned int purp_num);

    void add_layer(const std::string & lay_name, unsigned int lay_num);

    void close();

    void create_layout(const std::string & cell, const std::string & view,
            const bag::Layout & layout);

private:
    oa::oaCoord double_to_oa(double val);
    void array_figure(oa::oaFig * fig_ptr, unsigned int nx, unsigned int ny, double spx,
            double spy);
    void create_inst(oa::oaBlock * blk_ptr, const bag::Inst & inst);
    void create_rect(oa::oaBlock * blk_ptr, const bag::Rect & inst);
    void create_path_seg(oa::oaBlock * blk_ptr, const bag::PathSeg & inst);
    void create_via(oa::oaBlock * blk_ptr, const bag::Via & inst);
    void create_pin(oa::oaBlock * blk_ptr, const bag::Pin & inst);
    void create_polygon(oa::oaBlock * blk_ptr, const bag::Polygon & inst);
    void create_blockage(oa::oaBlock * blk_ptr, const bag::Blockage & inst);
    void create_boundary(oa::oaBlock * blk_ptr, const bag::Boundary & inst);

    bool is_open;
    oa::oaUInt4 dbu_per_uu;
    oa::oaUInt4 mfg_grid_res;
    LayerMap lay_map;
    PurposeMap purp_map;
    LibDefObserver lib_def_obs;

    oa::oaLib * lib_ptr;
    oa::oaTech * tech_ptr;
    oa::oaScalarName lib_name;
};

class OASchematicWriter {
public:
    OASchematicWriter() :
            is_open(false), lib_def_obs(1), lib_ptr(NULL) {
    }
    ~OASchematicWriter() {
    }

    void open_library(const std::string & lib_path, const std::string & library);

    void create_schematics(const std::vector<bag::SchCell> & cell_list,
            const std::string & sch_name, const std::string & sym_name);

    void close();

private:
    bool is_open;
    LibDefObserver lib_def_obs;

    oa::oaLib * lib_ptr;
    oa::oaScalarName lib_name;
};

oa::oaString get_orient_name(unsigned char orient_code);
}

#endif
