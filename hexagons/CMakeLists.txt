set(
    HEADER_FILES
        HexagonView.h
)

set(
    SOURCE_FILES
        main.cpp
        Source/HexagonView.cpp
)

set(
    GLSL_SOURCE_FILES
        shaders/Sampler.frag
        shaders/Compute.comp
)

set(
    GLSL_EDITING_FILES
        shaders/dither.comp.glsl
        shaders/compute.comp.glsl
        shaders/mask.comp.glsl
        shaders/copy.comp.glsl
        shaders/copytodraw.comp.glsl
)

add_executable(
    hexagons
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

set_property( TARGET hexagons PROPERTY CXX_STANDARD 17 )

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

# Shader editing files
foreach( GLSL ${GLSL_EDITING_FILES} )

    get_filename_component( FILE_NAME ${GLSL} NAME )

    add_custom_command(
            OUTPUT ${FILE_NAME}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/resources/${GLSL} ${FILE_NAME}
            DEPENDS ${CMAKE_SOURCE_DIR}/resources/${GLSL}
    )

    list( APPEND RUNTIME_SHADERS ${FILE_NAME} )

endforeach( GLSL )

add_custom_target(
        hexagons_runtime_shaders
        DEPENDS ${RUNTIME_SHADERS}
)
add_dependencies( hexagons hexagons_runtime_shaders )

# Shaders
compile_shaders( hexagons "${CMAKE_SOURCE_DIR}/resources/" "${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/" "${GLSL_SOURCE_FILES}" )