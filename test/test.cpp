#include "bagoa.hpp"

int test_layout(int argc, char * argv[]) {

	try {

		double enc1[] = { 0.1, 0.2, 0.3, 0.4 };
		double enc2[] = { 0.4, 0.3, 0.2, 0.1 };

		// create layout
		bag::Layout layout;
		layout.add_rect(argv[1], "drawing", 0.0, 0.0, 0.2, 0.1, 3, 2, 0.3, 0.6);
		layout.add_via(argv[2], 0.25, 0.25, "R0", 2, 3, 0.1, 0.15, enc1[0], enc1[3], enc1[1],
				enc1[2], enc2[0], enc2[3], enc2[1], enc2[2], -1, -1, 4, 5, 0.4, 0.5);
		layout.add_pin("foo", "foo1", "foo:", "M1", "pin", 1.0, 1.1, 1.2, 1.2);

		// open library and draw layout
		bagoa::OALayoutLibrary lib;
		lib.open_library("./cds.lib", "AAAFOO");
		lib.add_purpose("pin", 251);
		lib.create_layout("testoa", "layout", layout);
		lib.close();
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

int test_sch_copy(int argc, char *argv[]) {

	try {

		const oa::oaNativeNS ns;
		std::string lib_path("./cds.lib");
		std::string library("AAAFOO");
		std::string cell("sampler_nmos");
		std::string new_cell("sampler_nmos_copy");
		std::string sch_view("schematic");
		std::string sym_view("symbol");

		oaDesignInit
		( oacAPIMajorRevNumber, oacAPIMinorRevNumber, oacDataModelRevNumber);

		// open library definition
		oa::oaString lib_def_path(lib_path.c_str());
		oa::oaLibDefList::openLibs(lib_def_path);

		// open library
		oa::oaScalarName lib_name = oa::oaScalarName(ns, library.c_str());
		oa::oaLib * lib_ptr = oa::oaLib::find(lib_name);
		if (lib_ptr == NULL) {
			throw std::invalid_argument("Cannot find library " + library);
		} else if (!lib_ptr->isValid()) {
			throw std::invalid_argument("Invalid library: " + library);
		}

		// open technology file
		oa::oaTech * tech_ptr = oa::oaTech::find(lib_ptr);
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

		oa::oaScalarName cell_name(ns, cell.c_str());
		oa::oaScalarName new_cell_name(ns, new_cell.c_str());
		oa::oaScalarName sch_view_name(ns, sch_view.c_str());
		oa::oaScalarName sym_view_name(ns, sym_view.c_str());

		oa::oaDesign * dsn_ptr = oa::oaDesign::open(lib_name, cell_name, sch_view_name,
				oa::oaViewType::get(oa::oacSchematic), 'r');

		dsn_ptr->saveAs(lib_name, new_cell_name, sch_view_name, true);
		dsn_ptr->close();

		dsn_ptr = oa::oaDesign::open(lib_name, cell_name, sym_view_name,
				oa::oaViewType::get(oa::oacSchematicSymbol), 'r');

		oa::oaIter<oa::oaProp> dsn_par_iter = dsn_ptr->getProps();
		while (oa::oaProp * dsn_par = dsn_par_iter.getNext()) {
			oa::oaString par_name, par_value;
			dsn_par->getName(par_name);
			dsn_par->getValue(par_value);
			std::cout << par_name << " = " << par_value << std::endl;
		}

		// modify partName property
		oa::oaStringProp * prop_ptr = static_cast<oa::oaStringProp *>(oa::oaProp::find(dsn_ptr,
				oa::oaString("partName")));
		if (prop_ptr != NULL) {
			prop_ptr->setValue(oa::oaString(new_cell.c_str()));
		}
		dsn_ptr->saveAs(lib_name, new_cell_name, sym_view_name, true);
		dsn_ptr->close();

	} catch (oa::oaCompatibilityError &ex) {
		throw std::runtime_error(
				"OA Compatibility Error: " + static_cast<std::string>(ex.getMsg()));
	} catch (oa::oaDMError &ex) {
		throw std::runtime_error("OA DM Error: " + static_cast<std::string>(ex.getMsg()));
	} catch (oa::oaDesignError &ex) {
		throw std::runtime_error("OA Design Error: " + static_cast<std::string>(ex.getMsg()));
	} catch (oa::oaError &ex) {
		throw std::runtime_error("OA Error: " + static_cast<std::string>(ex.getMsg()));
	}
	return 0;

}

int main(int argc, char *argv[]) {
// return test_layout(argc, argv);
	return test_sch_copy(argc, argv);
}
