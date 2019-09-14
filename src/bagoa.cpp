#include "bagoa.hpp"

namespace bagoa {

// OA namespace object
const oa::oaNativeNS ns;
const oa::oaCdbaNS ns_cdba;

// Layout pin access direction (all)
const oa::oaByte pin_dir = oacTop | oacBottom | oacLeft | oacRight;

oa::oaString get_orient_name(unsigned char orient_code) {
    switch (orient_code) {
    case 0:
        return oa::oaString("R0");
    case 1:
        return oa::oaString("MX");
    case 2:
        return oa::oaString("MY");
    case 3:
        return oa::oaString("R180");
    case 4:
        return oa::oaString("R90");
    case 5:
        return oa::oaString("MXR90");
    case 6:
        return oa::oaString("MYR90");
    case 7:
        return oa::oaString("R270");
    default:
        std::ostringstream os;
        os << "Invalid orientation code: " << orient_code;
        throw std::invalid_argument(os.str());
    }
}

LibDefObserver::LibDefObserver(oa::oaUInt4 priority) :
        oa::oaObserver<oa::oaLibDefList>(priority, true) {}


oa::oaBoolean LibDefObserver::onLoadWarnings(oa::oaLibDefList *obj, const oa::oaString & msg,
        oa::oaLibDefListWarningTypeEnum type) {
    err_msg = "OA Error: " + static_cast<std::string>(msg);
    return true;
}

void OALayoutLibrary::open_library(const std::string & lib_file, const std::string & library,
                                   const std::string & lib_path, const std::string & tech_lib) {
    try {
        oaDesignInit
        ( oacAPIMajorRevNumber, oacAPIMinorRevNumber, oacDataModelRevNumber);

        // open library definition
        oa::oaString lib_def_file(lib_file.c_str());
        oa::oaLibDefList::openLibs(lib_def_file);

        if (!lib_def_obs.err_msg.empty()) {
            throw std::runtime_error(lib_def_obs.err_msg);
        }

        // open library
        lib_name = oa::oaScalarName(ns, library.c_str());
        lib_ptr = oa::oaLib::find(lib_name);
        if (lib_ptr == NULL) {
            // create new library
            oa::oaString oa_lib_path(lib_path.c_str());
            oa::oaScalarName oa_tech_lib(ns, tech_lib.c_str());
            lib_ptr = oa::oaLib::create(lib_name, oa_lib_path);
            oa::oaTech::attach(lib_ptr, oa_tech_lib);

            // I cannot get open access to modify the library file, so
            // we just do it by hand.
            std::ofstream outfile;
            outfile.open(lib_file, std::ios_base::app);
            outfile << "DEFINE " << library << " " << lib_path << std::endl;
        } else if (!lib_ptr->isValid()) {
            throw std::invalid_argument("Invalid library: " + library);
        }

        // open technology file
        tech_ptr = oa::oaTech::find(lib_ptr);
        if (tech_ptr == NULL) {
            // opened tech not found, attempt to open
            if (!oa::oaTech::exists(lib_ptr)) {
                throw std::runtime_error("Cannot find technology for library: " + library);
            } else {
                tech_ptr = oa::oaTech::open(lib_ptr, 'r');
                if (tech_ptr == NULL) {
                    throw std::runtime_error("Cannot open technology for library: " + library);
                }
            }
        }

        // get database unit
        dbu_per_uu = tech_ptr->getDBUPerUU(oa::oaViewType::get(oa::oacMaskLayout));

        // fill layer/purpose map
        oa::oaString temp;
        oa::oaIter<oa::oaLayer> layers(tech_ptr->getLayers());
        while (oa::oaLayer *layer = layers.getNext()) {
            layer->getName(temp);
            std::string name = static_cast<std::string>(temp);
            oa::oaLayerNum id = layer->getNumber();
            lay_map[name] = id;
        }
        oa::oaIter<oa::oaPurpose> purposes(tech_ptr->getPurposes());
        while (oa::oaPurpose *purp = purposes.getNext()) {
            purp->getName(temp);
            std::string name = static_cast<std::string>(temp);
            oa::oaLayerNum id = purp->getNumber();
            purp_map[name] = id;
        }

        is_open = true;
    } catch (oa::oaCompatibilityError &ex) {
        throw std::runtime_error(
                "OA Compatibility Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDMError &ex) {
        throw std::runtime_error("OA DM Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaError &ex) {
        throw std::runtime_error("OA Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDesignError &ex) {
        throw std::runtime_error("OA Design Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaTechError &ex) {
        throw std::runtime_error("OA Tech Error: " + static_cast<std::string>(ex.getMsg()));
    }
}

void OALayoutLibrary::add_purpose(const std::string & purp_name, unsigned int purp_num) {
    purp_map[purp_name] = (oa::oaPurposeNum) purp_num;
}

void OALayoutLibrary::add_layer(const std::string & lay_name, unsigned int lay_num) {
    lay_map[lay_name] = (oa::oaLayerNum) lay_num;
}

void OALayoutLibrary::close() {
    if (is_open) {
        tech_ptr->close();
        lib_ptr->close();

        is_open = false;
    }

}

void OALayoutLibrary::create_layout(const std::string & cell, const std::string & view,
        const bag::Layout & layout) {
    // do nothing if no library is opened
    if (!is_open) {
        return;
    }

    try {
        // open design and top block
        oa::oaScalarName cell_name(ns, cell.c_str());
        oa::oaScalarName view_name(ns, view.c_str());
        oa::oaDesign * dsn_ptr = oa::oaDesign::open(lib_name, cell_name, view_name,
                oa::oaViewType::get(oa::oacMaskLayout), 'w');
        oa::oaBlock * blk_ptr = oa::oaBlock::create(dsn_ptr);

        // create geometries
        for (bag::InstIter it = layout.inst_list.begin(); it != layout.inst_list.end(); it++) {
            create_inst(blk_ptr, *it);
        }
        for (bag::RectIter it = layout.rect_list.begin(); it != layout.rect_list.end(); it++) {
            create_rect(blk_ptr, *it);
        }
        for (bag::PathSegIter it = layout.path_seg_list.begin(); it != layout.path_seg_list.end();
                it++) {
            create_path_seg(blk_ptr, *it);
        }
        for (bag::ViaIter it = layout.via_list.begin(); it != layout.via_list.end(); it++) {
            create_via(blk_ptr, *it);
        }
        for (bag::PinIter it = layout.pin_list.begin(); it != layout.pin_list.end(); it++) {
            create_pin(blk_ptr, *it);
        }
        for (bag::PolygonIter it = layout.polygon_list.begin(); it != layout.polygon_list.end(); it++) {
            create_polygon(blk_ptr, *it);
        }
        for (bag::BlockageIter it = layout.block_list.begin(); it != layout.block_list.end(); it++) {
            create_blockage(blk_ptr, *it);
        }
        for (bag::BoundaryIter it = layout.boundary_list.begin(); it != layout.boundary_list.end(); it++) {
            create_boundary(blk_ptr, *it);
        }

        // save and close
        dsn_ptr->save();
        dsn_ptr->close();
    } catch (oa::oaCompatibilityError &ex) {
        throw std::runtime_error(
                "OA Compatibility Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDMError &ex) {
        throw std::runtime_error("OA DM Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaError &ex) {
        throw std::runtime_error("OA Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDesignError &ex) {
        throw std::runtime_error("OA Design Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaTechError &ex) {
        throw std::runtime_error("OA Tech Error: " + static_cast<std::string>(ex.getMsg()));
    }
}

oa::oaCoord OALayoutLibrary::double_to_oa(double val) {
    return (oa::oaCoord) round(val * dbu_per_uu);
}

void OALayoutLibrary::array_figure(oa::oaFig * fig_ptr, unsigned int nx, unsigned int ny,
        double spx, double spy) {
    if (nx > 1 || ny > 1) {
        oa::oaCoord spx_oa = double_to_oa(spx);
        oa::oaCoord spy_oa = double_to_oa(spy);
        for (unsigned int j = 1; j < ny; j++) {
            fig_ptr->copy(oa::oaTransform(0, j * spy_oa));
        }
        for (unsigned int i = 1; i < nx; i++) {
            oa::oaCoord offx = i * spx_oa;
            for (unsigned int j = 0; j < ny; j++) {
                fig_ptr->copy(oa::oaTransform(offx, j * spy_oa));
            }
        }
    }
}

void OALayoutLibrary::create_inst(oa::oaBlock * blk_ptr, const bag::Inst & inst) {
    oa::oaScalarName lib_name(ns, oa::oaString(inst.lib_name.c_str()));
    oa::oaScalarName cell_name(ns, oa::oaString(inst.cell_name.c_str()));
    oa::oaScalarName view_name(ns, oa::oaString(inst.view_name.c_str()));
    oa::oaScalarName inst_name(ns, oa::oaString(inst.inst_name.c_str()));

    oa::oaTransform xfm((oa::oaOffset) double_to_oa(inst.loc[0]),
            (oa::oaOffset) double_to_oa(inst.loc[1]), get_orient_name(inst.orient));

    oa::oaOffset dx = (oa::oaOffset) double_to_oa(inst.sp_cols);
    oa::oaOffset dy = (oa::oaOffset) double_to_oa(inst.sp_rows);

    // create oa ParamArray
    oa::oaParamArray oa_params;
    for (bag::IntIter it = inst.int_params.begin(); it != inst.int_params.end(); it++) {
        oa::oaString key(it->first.c_str());
        oa_params.append(oa::oaParam(key, it->second));
    }
    for (bag::DoubleIter it = inst.double_params.begin(); it != inst.double_params.end(); it++) {
        oa::oaString key(it->first.c_str());
        oa_params.append(oa::oaParam(key, it->second));
    }
    for (bag::StrIter it = inst.str_params.begin(); it != inst.str_params.end(); it++) {
        oa::oaString key(it->first.c_str());
        oa_params.append(oa::oaParam(key, oa::oaString(it->second.c_str())));
    }

    const oa::oaParamArray * params_ptr = &oa_params;
    if (params_ptr->getNumElements() == 0) {
        // disable parameters if empty
        params_ptr = NULL;
    }
    if (inst.num_rows > 1 || inst.num_cols > 1) {
        oa::oaArrayInst::create(blk_ptr, lib_name, cell_name, view_name, inst_name, xfm, dx, dy,
                inst.num_rows, inst.num_cols, params_ptr);
    } else {
        oa::oaScalarInst::create(blk_ptr, lib_name, cell_name, view_name, inst_name, xfm,
                params_ptr);
    }
}

void OALayoutLibrary::create_via(oa::oaBlock * blk_ptr, const bag::Via & inst) {
    oa::oaString oa_via_id = oa::oaString(inst.via_id.c_str());
    oa::oaStdViaDef * vdef = static_cast<oa::oaStdViaDef *>(oa::oaViaDef::find(tech_ptr, oa_via_id));
    if (vdef == NULL) {
        std::cout << "create_via: unknown via " << inst.via_id << ", skipping." << std::endl;
        return;
    }

    oa::oaTransform xfm((oa::oaOffset) double_to_oa(inst.loc[0]),
            (oa::oaOffset) double_to_oa(inst.loc[1]), get_orient_name(inst.orient));

    oa::oaViaParam params;
    params.setCutRows(inst.num_rows);
    params.setCutColumns(inst.num_cols);
    params.setCutSpacing(
            oa::oaVector((oa::oaOffset) double_to_oa(inst.spacing[0]),
                    (oa::oaOffset) double_to_oa(inst.spacing[1])));
    params.setLayer1Enc(
            oa::oaVector((oa::oaOffset) double_to_oa(inst.enc1[0]),
                    (oa::oaOffset) double_to_oa(inst.enc1[1])));
    params.setLayer1Offset(
            oa::oaVector((oa::oaOffset) double_to_oa(inst.off1[0]),
                    (oa::oaOffset) double_to_oa(inst.off1[1])));
    params.setLayer2Enc(
            oa::oaVector((oa::oaOffset) double_to_oa(inst.enc2[0]),
                    (oa::oaOffset) double_to_oa(inst.enc2[1])));
    params.setLayer2Offset(
            oa::oaVector((oa::oaOffset) double_to_oa(inst.off2[0]),
                    (oa::oaOffset) double_to_oa(inst.off2[1])));
    if (inst.cut_width > 0) {
        params.setCutWidth((oa::oaDist) double_to_oa(inst.cut_width));
    }
    if (inst.cut_height > 0) {
        params.setCutHeight((oa::oaDist) double_to_oa(inst.cut_height));
    }

    oa::oaFig * fig = static_cast<oa::oaFig *>(oa::oaStdVia::create(blk_ptr, vdef, xfm, &params));
    array_figure(fig, inst.nx, inst.ny, inst.spx, inst.spy);
}

void OALayoutLibrary::create_rect(oa::oaBlock * blk_ptr, const bag::Rect & inst) {
    LayerIter lay_iter = lay_map.find(inst.layer);
    if (lay_iter == lay_map.end()) {
        std::cout << "create_rect: unknown layer " << inst.layer << ", skipping." << std::endl;
        return;
    }
    oa::oaLayerNum layer = lay_iter->second;

    PurposeIter purp_iter = purp_map.find(inst.purpose);
    if (purp_iter == purp_map.end()) {
        std::cout << "create_rect: unknown purpose " << inst.purpose << ", skipping." << std::endl;
        return;
    }
    oa::oaPurposeNum purpose = purp_iter->second;

    oa::oaBox box(double_to_oa(inst.bbox[0]), double_to_oa(inst.bbox[1]),
            double_to_oa(inst.bbox[2]), double_to_oa(inst.bbox[3]));
    oa::oaRect * r = oa::oaRect::create(blk_ptr, layer, purpose, box);
    array_figure(static_cast<oa::oaFig *>(r), inst.nx, inst.ny, inst.spx, inst.spy);
}

void OALayoutLibrary::create_path_seg(oa::oaBlock * blk_ptr, const bag::PathSeg & inst) {
    LayerIter lay_iter = lay_map.find(inst.layer);
    if (lay_iter == lay_map.end()) {
        std::cout << "create_path_seg: unknown layer " << inst.layer << ", skipping." << std::endl;
        return;
    }
    oa::oaLayerNum layer = lay_iter->second;

    PurposeIter purp_iter = purp_map.find(inst.purpose);
    if (purp_iter == purp_map.end()) {
        std::cout << "create_path_seg: unknown purpose " << inst.purpose << ", skipping."
                << std::endl;
        return;
    }
    oa::oaPurposeNum purpose = purp_iter->second;

    oa::oaPoint start = oa::oaPoint(double_to_oa(inst.x0), double_to_oa(inst.y0));
    oa::oaPoint stop = oa::oaPoint(double_to_oa(inst.x1), double_to_oa(inst.y1));

    oa::oaDist width, diagExt;
    if (start.x() != stop.x() and start.y() != stop.y()) {
        // both X and Y coordinate differ, must be diagonal
        // set width in diagonal unit, round to even
        width = double_to_oa(inst.width * sqrt(2) / 2) * 2;
        diagExt = double_to_oa(inst.width / 2);
    } else {
        width = double_to_oa(inst.width / 2) * 2;
        diagExt = double_to_oa(inst.width * sqrt(2) / 2);
    }

    oa::oaSegStyle style(width, oa::oacTruncateEndStyle, oa::oacTruncateEndStyle);
    if (inst.begin_style == "extend") {
        style.setBeginStyle(oa::oacExtendEndStyle);
    } else if (inst.begin_style == "round") {
        style.setBeginStyle(oa::oacCustomEndStyle, width / 2, diagExt, diagExt, width / 2);
    }
    if (inst.end_style == "extend") {
        style.setEndStyle(oa::oacExtendEndStyle);
    } else if (inst.end_style == "round") {
        style.setEndStyle(oa::oacCustomEndStyle, width / 2, diagExt, diagExt, width / 2);
    }

    oa::oaPathSeg::create(blk_ptr, layer, purpose, start, stop, style);
}

void OALayoutLibrary::create_pin(oa::oaBlock * blk_ptr, const bag::Pin & inst) {
    // draw pin rectangle
    LayerIter lay_iter = lay_map.find(inst.layer);
    if (lay_iter == lay_map.end()) {
        std::cout << "create_pin: unknown layer " << inst.layer << ", skipping." << std::endl;
        return;
    }
    oa::oaLayerNum layer = lay_iter->second;

    PurposeIter purp_iter = purp_map.find(inst.purpose);
    if (purp_iter == purp_map.end()) {
        std::cout << "create_pin: unknown purpose " << inst.purpose << ", skipping." << std::endl;
        return;
    }
    oa::oaPurposeNum purpose = purp_iter->second;

    oa::oaBox box(double_to_oa(inst.bbox[0]), double_to_oa(inst.bbox[1]),
            double_to_oa(inst.bbox[2]), double_to_oa(inst.bbox[3]));

    // get label location and orientation
    oa::oaPoint op;
    box.getCenter(op);
    oa::oaOrient lorient("R0");
    oa::oaDist lheight = (oa::oaDist) box.getHeight();
    if (box.getHeight() > box.getWidth()) {
        lorient = oa::oaOrient("R90");
        lheight = (oa::oaDist) box.getWidth();
    }

    // create label
    oa::oaString oa_label = oa::oaString(inst.label.c_str());
    oa::oaText::create(blk_ptr, layer, purpose, oa_label, op, oa::oacCenterCenterTextAlign, lorient,
            oa::oacRomanFont, lheight);

    if (inst.make_pin_obj) {
        // make pin object
        oa::oaRect * r = oa::oaRect::create(blk_ptr, layer, purpose, box);

        // get terminal
        oa::oaName term_name(ns_cdba, oa::oaString(inst.term_name.c_str()));
        oa::oaTerm * term = oa::oaTerm::find(blk_ptr, term_name);
        if (term == NULL) {
            // get net
            oa::oaNet * net = oa::oaNet::find(blk_ptr, term_name);
            if (net == NULL) {
                // create net
                net = oa::oaNet::create(blk_ptr, term_name);
            }
            // create terminal
            term = oa::oaTerm::create(net, term_name);
        }

        // create pin and add rectangle to pin.
        oa::oaString oa_pin_name = oa::oaString(inst.pin_name.c_str());
        oa::oaPin * pin = oa::oaPin::create(term, oa_pin_name, pin_dir);
        r->addToPin(pin);
    }
}

void OALayoutLibrary::create_polygon(oa::oaBlock * blk_ptr, const bag::Polygon & inst) {
    // make oaPointArray
    oa::oaPointArray pt_arr;
    for (unsigned int idx = 0; idx < inst.xcoord.size(); idx++) {
        oa::oaCoord x_unit = double_to_oa(inst.xcoord[idx]);
        oa::oaCoord y_unit = double_to_oa(inst.ycoord[idx]);
        pt_arr.append(oa::oaPoint(x_unit, y_unit));
    }
    LayerIter lay_iter = lay_map.find(inst.layer);
    if (lay_iter == lay_map.end()) {
        std::cout << "create_polygon: unknown layer " << inst.layer << ", skipping." << std::endl;
        return;
    }
    oa::oaLayerNum layer = lay_iter->second;

    PurposeIter purp_iter = purp_map.find(inst.purpose);
    if (purp_iter == purp_map.end()) {
        std::cout << "create_polygon: unknown purpose " << inst.purpose << ", skipping." << std::endl;
        return;
    }
    oa::oaPurposeNum purpose = purp_iter->second;

    oa::oaPolygon::create(blk_ptr, layer, purpose, pt_arr);
}

void OALayoutLibrary::create_blockage(oa::oaBlock * blk_ptr, const bag::Blockage & inst) {
    // make oaPointArray
    oa::oaPointArray pt_arr;
    for (unsigned int idx = 0; idx < inst.xcoord.size(); idx++) {
        oa::oaCoord x_unit = double_to_oa(inst.xcoord[idx]);
        oa::oaCoord y_unit = double_to_oa(inst.ycoord[idx]);
        pt_arr.append(oa::oaPoint(x_unit, y_unit));
    }
    if (inst.type == "placement") {
        // area blockage
        oa::oaAreaBlockage::create(blk_ptr, pt_arr);
    } else {
        LayerIter lay_iter = lay_map.find(inst.layer);
        if (lay_iter == lay_map.end()) {
            std::cout << "create_blockage: unknown layer " << inst.layer << ", skipping." << std::endl;
            return;
        }
        oa::oaLayerNum layer = lay_iter->second;
        oa::oaBlockageType block_type(oa::oaString(inst.type.c_str()));

        oa::oaLayerBlockage::create(blk_ptr, block_type, layer, pt_arr);
    }
}

void OALayoutLibrary::create_boundary(oa::oaBlock * blk_ptr, const bag::Boundary & inst) {
    // make oaPointArray
    oa::oaPointArray pt_arr;
    for (unsigned int idx = 0; idx < inst.xcoord.size(); idx++) {
        oa::oaCoord x_unit = double_to_oa(inst.xcoord[idx]);
        oa::oaCoord y_unit = double_to_oa(inst.ycoord[idx]);
        pt_arr.append(oa::oaPoint(x_unit, y_unit));
    }
    if (inst.type == "PR") {
        // PR boundary
        oa::oaPRBoundary::create(blk_ptr, pt_arr);
    } else if (inst.type == "snap") {
        oa::oaSnapBoundary::create(blk_ptr, pt_arr);
    } else if (inst.type == "area") {
        oa::oaAreaBoundary::create(blk_ptr, pt_arr);
    } else {
        std::cout << "create_boundary: unknown boundary type" << inst.type << ", skipping." << std::endl;
    }
}

void OASchematicWriter::open_library(const std::string & lib_path, const std::string & library) {
    try {
        oaDesignInit
        ( oacAPIMajorRevNumber, oacAPIMinorRevNumber, oacDataModelRevNumber);

        // open library definition
        oa::oaString lib_def_path(lib_path.c_str());
        oa::oaLibDefList::openLibs(lib_def_path);

        // open library
        lib_name = oa::oaScalarName(ns, library.c_str());
        lib_ptr = oa::oaLib::find(lib_name);
        if (lib_ptr == NULL) {
            throw std::invalid_argument("Cannot find library " + library);
        } else if (!lib_ptr->isValid()) {
            throw std::invalid_argument("Invalid library: " + library);
        }

        is_open = true;
    } catch (oa::oaCompatibilityError &ex) {
        throw std::runtime_error(
                "OA Compatibility Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDMError &ex) {
        throw std::runtime_error("OA DM Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDesignError &ex) {
        throw std::runtime_error("OA Design Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaError &ex) {
        throw std::runtime_error("OA Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaTechError &ex) {
        throw std::runtime_error("OA Tech Error: " + static_cast<std::string>(ex.getMsg()));
    }
}

void OASchematicWriter::create_schematics(const std::vector<bag::SchCell> & cell_list,
        const std::string & sch_name, const std::string & sym_name) {
    // do nothing if no library is opened
    if (!is_open) {
        return;
    }

    try {
        oa::oaScalarName sch_view(ns, sch_name.c_str());
        oa::oaScalarName sym_view(ns, sym_name.c_str());
        oa::oaString part_name_prop("partName");
        std::vector<bag::SchCell>::const_iterator it;
        for (std::vector<bag::SchCell>::const_iterator it = cell_list.begin();
                it != cell_list.end(); it++) {
            oa::oaScalarName sch_lib(ns, it->lib_name.c_str());
            oa::oaScalarName cell_name(ns, it->cell_name.c_str());
            oa::oaScalarName new_cell_name(ns, it->new_cell_name.c_str());
            oa::oaBoolean rename_module = (it->cell_name != it->new_cell_name);

            // oa::oaString debug_lib;
            // lib_name.get(debug_lib);
            // std::cout << "copying " << it->lib_name << ", " << it->cell_name << " to " << debug_lib
            //      << ", " << it->new_cell_name << " , rename = " << rename_module << std::endl;

            // copy schematic
            oa::oaDesign * dsn_ptr = oa::oaDesign::open(sch_lib, cell_name, sch_view,
                    oa::oaViewType::get(oa::oacSchematic), 'r');
            dsn_ptr->saveAs(lib_name, new_cell_name, sch_view, rename_module);
            dsn_ptr->close();

            // open symbol
            dsn_ptr = oa::oaDesign::open(sch_lib, cell_name, sym_view,
                    oa::oaViewType::get(oa::oacSchematicSymbol), 'r');
            // modify partName property
            oa::oaStringProp * prop_ptr = static_cast<oa::oaStringProp *>(oa::oaProp::find(dsn_ptr,
                    part_name_prop));
            if (prop_ptr != NULL) {
                prop_ptr->setValue(oa::oaString(it->new_cell_name.c_str()));
            } else {
                std::cout << "create_schematic : cannot find partName property, not modifying."
                        << std::endl;
            }
            // copy symbol
            dsn_ptr->saveAs(lib_name, new_cell_name, sym_view, rename_module);
            dsn_ptr->close();
        }

    } catch (oa::oaCompatibilityError &ex) {
        throw std::runtime_error(
                "OA Compatibility Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDMError &ex) {
        throw std::runtime_error("OA DM Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaDesignError &ex) {
        throw std::runtime_error("OA Design Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaError &ex) {
        throw std::runtime_error("OA Error: " + static_cast<std::string>(ex.getMsg()));
    } catch (oa::oaTechError &ex) {
        throw std::runtime_error("OA Tech Error: " + static_cast<std::string>(ex.getMsg()));
    }
}

void OASchematicWriter::close() {
    if (is_open) {
        lib_ptr->close();

        is_open = false;
    }

}

}
