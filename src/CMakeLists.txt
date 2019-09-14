# list sources explicitly, so if we add/remove sources cmake 
# knows to update makefiles.
set(SOURCES
  bag.cpp 
  bagoa.cpp
  ${CMAKE_SOURCE_DIR}/include/bag.hpp
  ${CMAKE_SOURCE_DIR}/include/bagoa.hpp
  )

# setup link library path.  Must call before defining target.
link_directories( $ENV{OA_LINK_DIR} )

# build shared library
add_library( bagoa SHARED ${SOURCES} )

# setup include libraries
include_directories( ${CMAKE_SOURCE_DIR}/include
  $ENV{OA_INCLUDE_DIR}
  )

message(status "** OA_LINK_DIR: $ENV{OA_LINK_DIR}")
message(status "** OA_INCLUDE_DIR: $ENV{OA_INCLUDE_DIR}")

# shared library dependencies
target_link_libraries( bagoa oaCommon oaBase oaPlugIn oaDM oaTech oaDesign ${CMAKE_DL_LIBS} )

# set shared library file folder
set_property( TARGET bagoa PROPERTY FOLDER "libraries" )

# install target to folders
install( TARGETS bagoa
  RUNTIME DESTINATION ${bagoa_BINARY_DIR}/bin
  LIBRARY DESTINATION ${bagoa_BINARY_DIR}/lib
  )
