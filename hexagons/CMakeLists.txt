set(
    HEADER_FILES
        HexagonView.h
        HexEngine.h
        HexMap.h
)

set(
    SOURCE_FILES
        main.cpp
        Source/HexagonView.cpp
        Source/HexEngine.cpp
        Source/HexMap.cpp
)

set(
    GLSL_SOURCE_FILES
        shaders/hexagons.comp
)

add_executable(
    hexagons
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

set_property( TARGET hexagons PROPERTY CXX_STANDARD 23 )

## Project libraries
target_link_libraries(
    hexagons PUBLIC
        Burst
        Imgui
)

# Burst
include_directories( ${BURST_ROOT_DIR} )

# Zep
set( ZEP_DIR "${LIB_DIR}/burst/lib/zep" )
target_include_directories(
    hexagons
        PRIVATE
        ${ZEP_DIR}/include
)
add_definitions(-DZEP_SINGLE_HEADER=1)
add_definitions(-DZEP_FEATURE_CPP_FILE_SYSTEM)

target_link_libraries(hexagons PRIVATE Zep::Zep)

# Shaders
compile_shaders( hexagons "${CMAKE_SOURCE_DIR}/resources/" "${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/" "${GLSL_SOURCE_FILES}" )