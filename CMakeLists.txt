cmake_minimum_required( VERSION 3.5 )
project( Hexagons VERSION 0.1.0 )

set( PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )

include( lib/burst/Burst.cmake )

set( LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib" )

add_subdirectory( lib )
include_directories( . )
include_directories( lib/burst )

add_subdirectory( hexagons )