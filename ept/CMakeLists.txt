project( ept )

# Find sources and tests
file(GLOB src *.cpp debtags/*.cc debtags/maint/*.cc debtags/coll/*.cc apt/*.cc axi/*.cc utils/*.cc)
file(GLOB tests *-test.cc apt/*-test.cc debtags/*-test.cc axi/*-test.cc)
list(REMOVE_ITEM src ${tests})

# Find headers
file( GLOB h_top *.h )
file( GLOB h_apt apt/*.h )
file( GLOB h_debtags debtags/*.h )
file( GLOB h_debtags_maint debtags/maint/*.h )
file( GLOB h_debtags_coll debtags/coll/*.h )
file( GLOB h_axi axi/*.h )
file( GLOB h_utils utils/*.h )

#file( GLOB debtagstesth debtags/*.test.h debtags/maint/*.test.h )
#file( GLOB apttesth apt/*.test.h )
#file( GLOB axitesth axi/*.test.h )
#set( testh ${debtagstesth} ${apttesth} ${axitesth} )

include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} 
                     ${TAGCOLL_INCLUDE_DIRS} ${WIBBLE_INCLUDE_DIRS} )
link_libraries( ${WIBBLE_LIBRARIES} ${TAGCOLL_LIBRARIES} apt-pkg xapian -lpthread )
add_definitions( --std=c++11 -fexceptions -fPIC -fvisibility=default )

add_library( ept SHARED ${src} )
add_library( ept-static STATIC ${src} )
set_target_properties( ept PROPERTIES SOVERSION ${LIBEPT_SOVERSION} CLEAN_DIRECT_OUTPUT 1)
set_target_properties( ept-static PROPERTIES SOVERSION ${LIBEPT_SOVERSION} OUTPUT_NAME "ept" CLEAN_DIRECT_OUTPUT 1)

add_executable(test-ept EXCLUDE_FROM_ALL utils/tests-main.cc ${tests})
target_link_libraries(test-ept ept)
add_test(test-ept test-ept)
add_dependencies(check test-ept)

configure_file( ${ept_SOURCE_DIR}/config.h.cmake-in
  ${ept_BINARY_DIR}/config.h )

set( prefix "${CMAKE_INSTALL_PREFIX}" )
set( exec_prefix "${prefix}/bin" )
set( libdir "${prefix}/lib" )
set( includedir "${prefix}/include" )
configure_file( ${ept_SOURCE_DIR}/libept.pc.in
  ${ept_BINARY_DIR}/libept.pc @ONLY )

# regression testing
link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../lib )
link_libraries( ept )

install( TARGETS ept ept-static
    LIBRARY DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE}
    ARCHIVE DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE} )

install( FILES ${ept_BINARY_DIR}/libept.pc DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig )
install( FILES ${h_top} DESTINATION include/ept )
install( FILES ${h_apt} DESTINATION include/ept/apt )
install( FILES ${h_debtags} DESTINATION include/ept/debtags )
install( FILES ${h_debtags_maint} DESTINATION include/ept/debtags/maint )
install( FILES ${h_debtags_coll} DESTINATION include/ept/debtags/coll )
install( FILES ${h_axi} DESTINATION include/ept/axi )
install( FILES ${h_utils} DESTINATION include/ept/utils )
