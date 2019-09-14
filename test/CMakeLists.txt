# list sources explicitly, so if we add/remove sources cmake 
# knows to update makefiles.
set(SOURCES
  read_oa.cpp
  )

# setup link library path.  Must call before defining target.
link_directories( $ENV{OA_LINK_DIR} )

# build test executable
add_executable(test_bagoa ${SOURCES})

# setup include libraries
include_directories(${CMAKE_SOURCE_DIR}/include
  $ENV{OA_INCLUDE_DIR}
  )

# shared library dependencies
target_link_libraries(test_bagoa bagoa)

# set test executable folder
set_property(TARGET test_bagoa PROPERTY FOLDER "executables")

# install targets to folders
install(TARGETS test_bagoa
  RUNTIME DESTINATION ${bagoa_BINARY_DIR}/bin
  LIBRARY DESTINATION ${bagoa_BINARY_DIR}/lib
  )
