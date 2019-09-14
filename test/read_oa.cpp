#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <exception>
#include <stdexcept>
#include <math.h>
#include <map>
#include <vector>

#include "oaDesignDB.h"

int read_oa(int argc, char * argv[]) {
    try {

        oaDesignInit
            ( oacAPIMajorRevNumber, oacAPIMinorRevNumber, oacDataModelRevNumber);
        
        const oa::oaNativeNS ns;
        oa::oaString lib_def_path("cds.lib");
        oa::oaLibDefList::openLibs(lib_def_path);
        
        // open library
        oa::oaScalarName lib_name(ns, "AAAscratch");
        oa::oaScalarName cell_name(ns, "oa_mos");
        oa::oaScalarName view_name(ns, "layout");
        oa::oaLib * lib_ptr = oa::oaLib::find(lib_name);
        if (lib_ptr == NULL) {
            throw std::invalid_argument("Cannot find library.");
        } else if (!lib_ptr->isValid()) {
            throw std::invalid_argument("Invalid library.");
        }
        
        oa::oaDesign * dsn_ptr = oa::oaDesign::open(lib_name, cell_name, view_name,
                                                    oa::oaViewType::get(oa::oacMaskLayout), 'r');
        
        
        oa::oaBlock * blk_ptr = dsn_ptr->getTopBlock();
        oa::oaSimpleName inst_name(ns, "X0");
        
        oa::oaInst *inst_ptr = oa::oaInst::find(blk_ptr, inst_name);
        oa::oaParamArray parr;
        
        inst_ptr->getParams(parr);
        
        unsigned int num = parr.getNumElements();

        std::ofstream myfile("oa_mos_params.txt");
        
        for (unsigned int x = 0; x < num; x++) {
            oa::oaParam par = parr[x];
            oa::oaParamType ptype = par.getType();
            oa::oaString name_str;
            oa::oaString temp_str;
            par.getName(name_str);
            switch(ptype) {
            case oa::oacIntParamType :
                std::cout << name_str << ": " << ptype.getName() << ", " << par.getIntVal() << std::endl;
                break;
            case oa::oacStringParamType :
                par.getStringVal(temp_str);
                std::cout << name_str << ": " << ptype.getName() << ", " << temp_str << std::endl;
                myfile << name_str << " " << temp_str << std::endl;
                break;
            default :
                std::cout << name_str << ": " << ptype.getName() << std::endl;
            }
        }

        myfile.close();
        
        lib_ptr->close();
    } catch (oa::oaCompatibilityError &ex) {
        throw std::runtime_error(
            "OA Compatibility Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDMError &ex) {
        throw std::runtime_error("OA DM Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaError &ex) {
        throw std::runtime_error("OA Error: " + static_cast<std::string>(ex.getMsg()));
    }
    
    return 0;
}

int write_oa(int argc, char * argv[]) {
    try {

        oaDesignInit
            ( oacAPIMajorRevNumber, oacAPIMinorRevNumber, oacDataModelRevNumber);
        
        const oa::oaNativeNS ns;
        oa::oaString lib_def_path("cds.lib");
        oa::oaLibDefList::openLibs(lib_def_path);
        
        // open library
        oa::oaScalarName lib_name(ns, "AAAscratch");
        oa::oaScalarName cell_name(ns, "oa_mos_write");
        oa::oaScalarName view_name(ns, "layout");
        oa::oaLib * lib_ptr = oa::oaLib::find(lib_name);
        if (lib_ptr == NULL) {
            throw std::invalid_argument("Cannot find library.");
        } else if (!lib_ptr->isValid()) {
            throw std::invalid_argument("Invalid library.");
        }
        
        oa::oaDesign * dsn_ptr = oa::oaDesign::open(lib_name, cell_name, view_name,
                                                    oa::oaViewType::get(oa::oacMaskLayout), 'w');
        
        
        oa::oaBlock * blk_ptr = oa::oaBlock::create(dsn_ptr);

        oa::oaScalarName mlib_name(ns, "cds_ff_mpt");
        oa::oaScalarName mcell_name(ns, "nch_lvt_mac");
        oa::oaDesign * master_ptr = oa::oaDesign::open(mlib_name, mcell_name, view_name,
                                                       oa::oaViewType::get(oa::oacMaskLayout), 'r');


        oa::oaScalarName inst_name(ns, "X0");
        oa::oaTransform inst_tran(oa::oaPoint(0, 0));
        oa::oaParamArray parr;

        std::ifstream myfile("oa_mos_params.txt");
        std::string line;
        while( std::getline(myfile, line) ) {
            std::istringstream iss(line);
            std::string name;
            std::string value;
            iss >> name;
            iss >> value;
            oa::oaParam par;
            par.setName(name.c_str());
            par.setStringVal(value.c_str());
            parr.append(par);
            std::cout << "adding par " << name << ": " << value << std::endl;
        }
        
        
        oa::oaScalarInst::create(blk_ptr, master_ptr, inst_name, inst_tran, &parr);

        dsn_ptr->save();
        dsn_ptr->close();
        lib_ptr->close();
    } catch (oa::oaCompatibilityError &ex) {
        throw std::runtime_error(
            "OA Compatibility Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDMError &ex) {
        throw std::runtime_error("OA DM Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaError &ex) {
        throw std::runtime_error("OA Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDesignError &ex) {
        throw std::runtime_error("OA Design Error: " + static_cast<std::string>(ex.getMsg()));
    }
    
    return 0;
}


int main(int argc, char *argv[]) {
    // return read_oa(argc, argv);
    return write_oa(argc, argv);
}
