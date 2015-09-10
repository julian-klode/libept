include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} 
                     ${TAGCOLL_INCLUDE_DIRS} ${WIBBLE_INCLUDE_DIRS} )
add_definitions( --std=c++11 )
link_libraries( ept )

add_executable( ept-cat ept-cat.cpp )
add_executable( pkglist pkglist.cpp )

set( bindir ${CMAKE_CURRENT_BINARY_DIR} )
set( srcdir ${CMAKE_CURRENT_SOURCE_DIR} )
