cmake_minimum_required(VERSION 3.2)

include( FindPkgConfig )
include( FindDoxygen )

add_custom_target(check COMMAND ${CMAKE_SOURCE_DIR}/run-check ${CMAKE_BINARY_DIR}/ept/test-ept)

set( EPT_VERSION "1.1" )

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

add_subdirectory( ept )
add_subdirectory( tools )
add_subdirectory( doc )
