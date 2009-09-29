include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} 
                     ${TAGCOLL_INCLUDE_DIRS} ${WIBBLE_INCLUDE_DIRS} )
link_libraries( ept )

add_executable( ept-search ept-search.cpp )
add_executable( ept-cat ept-cat.cpp )
add_executable( pkglist pkglist.cpp )
add_executable( manpage manpage.cc )
add_executable( ept-cache
  ept-cache.cc Environment.cc filters.cc info.cc )

set( bindir ${CMAKE_CURRENT_BINARY_DIR} )
set( srcdir ${CMAKE_CURRENT_SOURCE_DIR} )

add_custom_command(
  TARGET manpage
  POST_BUILD
  DEPENDS manpage
  COMMAND LD_LIBRARY_PATH=${ept_BINARY_DIR} ${bindir}/manpage ept-cache ${srcdir}/ept-cache-man-hooks > ${bindir}/ept-cache.1 )

install( TARGETS ept-cache RUNTIME DESTINATION bin )
install( FILES ${bindir}/ept-cache.1 DESTINATION share/man/man1 )
