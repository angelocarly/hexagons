set(
    HEADER_FILES
        TestPresenter.h
)

set(
    SOURCE_FILES
        Source/TestPresenter.cpp
        main.cpp
)

set(
    GLSL_SOURCE_FILES
        shaders/Sampler.frag
        shaders/Compute.comp
)

add_executable(
    feedbackloop
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

set_property( TARGET feedbackloop PROPERTY CXX_STANDARD 17 )

## Project libraries
target_link_libraries(
    feedbackloop PUBLIC
        Burst
        Imgui
)

# Burst
include_directories( ${BURST_ROOT_DIR} )

# Zep
set( ZEP_DIR "${LIB_DIR}/burst/lib/zep" )
target_include_directories(
    feedbackloop
        PRIVATE
        ${ZEP_DIR}/include
)
add_definitions(-DZEP_SINGLE_HEADER=1)
add_definitions(-DZEP_FEATURE_CPP_FILE_SYSTEM)

target_link_libraries(feedbackloop PRIVATE Zep::Zep)

# Shaders
compile_shaders( feedbackloop "${CMAKE_SOURCE_DIR}/resources/" "${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/" "${GLSL_SOURCE_FILES}" )