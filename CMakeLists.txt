cmake_minimum_required(VERSION 3.2)

include( FindPkgConfig )
include( FindDoxygen )

add_custom_target( unit )

set( EPT_VERSION "1.0.14" )

# Get the soversion from libapt-pkg to include in our own
execute_process(
	COMMAND "cc" "-xc" "-shared" "-Wl,--no-as-needed" "/dev/null" "-lapt-pkg"
	)
execute_process(
	COMMAND "objdump" "-p" "a.out"
	COMMAND "awk" "/NEEDED.*libapt-pkg/ { sub(\".*\\\\.so\\\\.\", \"\"); print }"
	OUTPUT_VARIABLE LIBAPT_ABI_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)
set( LIBEPT_SOVERSION "1.aptpkg${LIBAPT_ABI_VERSION}" )
set( APT_INCLUDES "/usr/include/" CACHE STRING "apt include path" )
set( APT_LINKDIRS "/usr/lib/" CACHE STRING "apt library path" )
set( OPT_FLAGS "-O0 -g" CACHE STRING "optimization level" )

option( INTERNAL_WIBBLE "use internal copy of wibble" )
option( RPM "using apt-rpm" )

if( INTERNAL_WIBBLE )
  add_subdirectory( wibble )
  set( WIBBLE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )
  set( WIBBLE_LIBRARY_DIRS ${wibble_BINARY_DIR} )
  set( WIBBLE_LIBRARIES "wibble" )
  set( WIBBLE_TEST_CMAKE "${wibble_SOURCE_DIR}/test.cmake" )
else( INTERNAL_WIBBLE )
  pkg_check_modules( WIBBLE REQUIRED "libwibble >= 1.0" )
  find_program( WIBBLE_TEST_GENRUNNER wibble-test-genrunner )
  set( WIBBLE_TEST_CMAKE "${WIBBLE_PREFIX}/share/wibble/test.cmake" )
endif( INTERNAL_WIBBLE )

add_subdirectory( ept )
add_subdirectory( tools )
add_subdirectory( doc )
