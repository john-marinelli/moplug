cmake_minimum_required(VERSION 3.25)

project(
    moplug_vst3
    VERSION 0.1.0
    DESCRIPTION "VST3 backend for moplug audio plugins"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# paths

set(VST3_SDK_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/vst3sdk"
)

set(MOJO_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/mojo_bridge"
)

set(MOJO_BUILD_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/mojo_bridge"
)

file(MAKE_DIRECTORY "${MOJO_BUILD_DIR}")


# vst3sdk

set(
    SMTG_ENABLE_VST3_PLUGIN_EXAMPLES
    OFF
    CACHE BOOL ""
    FORCE
)

set(
    SMTG_ENABLE_VST3_HOSTING_EXAMPLES
    OFF
    CACHE BOOL ""
    FORCE
)

set(
    SMTG_ENABLE_VSTGUI_SUPPORT
    OFF
    CACHE BOOL ""
    FORCE
)

add_subdirectory(
    "${VST3_SDK_ROOT}"
    "${CMAKE_CURRENT_BINARY_DIR}/vst3sdk"
)

smtg_enable_vst3_sdk()


# bridge

find_program(
    MOJO_EXECUTABLE
    NAMES mojo
    HINTS
        "${CMAKE_CURRNT_SOURCE_DIR}/.pixi/envs/default/bin"
        "$ENV{PIXI_ENVIRONMENT_PATH}/bin"
    REQUIRED
)

message(STATUS "Found mojo: ${MOJO_EXECUTABLE}")

get_filename_component(
    MOJO_BIN_DIR
    "${MOJO_EXECUTABLE}"
    DIRECTORY
)

get_filename_component(
    MOJO_ENV_DIR
    "${MOJO_BIN_DIR}"
    DIRECTORY
)

set(
    MOJO_LIB_DIR
    "${MOJO_ENV_DIR}/lib"
)

find_library(
    MOJO_KGEN_RUNTIME
    NAMES
        KGENCompilerRT
        KGENCompilerRTShared
    PATHS
        "${MOJO_LIB_DIR}"
    REQUIRED
)

message(
    STATUS
    "Found mojo runtime: ${MOJO_KGEN_RUNTIME}"
)


set(
    MOJO_BRIDGE_OBJECT
    "${MOJO_BUILD_DIR}/bridge.o"
)

add_custom_command(
    OUTPUT
        "${MOJO_BRIDGE_OBJECT}"

    COMMAND
        "${MOJO_EXECUTABLE}"
        build
        "${MOJO_SOURCE_DIR}/bridge.mojo"
        -I "${MOJO_SOURCE_DIR}"
        --emit object
        -o "${MOJO_BRIDGE_OBJECT}"

    DEPENDS
        "${MOJO_SOURCE_DIR}/bridge.mojo"
        "${MOJO_SOURCE_DIR}/dsp_bridge.mojo"
        "${MOJO_SOURCE_DIR}/state_bridge.mojo"
        "${MOJO_SOURCE_DIR}/types.mojo"

    COMMENT
        "Compiling Mojo VST3 bridge"

    VERBATIM
)

add_custom_target(
    mojo_bridge
    DEPENDS
        "${MOJO_BRIDGE_OBJECT}"
)


# plugin

smtg_add_vst3plugin(
    moplug_vst3_plugin

    include/bridge.h
    include/ids.h
    include/processor.h
    include/controller.h

    native/processor.cpp
    native/controller.cpp
    native/factory.cpp
)


add_dependencies(
    moplug_vst3_plugin
    mojo_bridge
)


target_sources(
    moplug_vst3_plugin
    PRIVATE
        "${MOJO_BRIDGE_OBJECT}"
)


target_include_directories(
    moplug_vst3_plugin

    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${VST3_SDK_ROOT}"
)


target_compile_definitions(
    moplug_vst3_plugin

    PRIVATE

    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)


target_link_libraries(
    moplug_vst3_plugin

    PRIVATE
        sdk
        "${MOJO_KGEN_RUNTIME}"
)


# warnings

if(MSVC)

    target_compile_options(
        moplug_vst3_plugin
        PRIVATE
            /W4
    )

else()

    target_compile_options(
        moplug_vst3_plugin
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()


# metadata

smtg_target_configure_version_file(
    moplug_vst3_plugin
)


# tests

option(
    MOPLUG_VST3_BUILD_TESTS
    "Build moplug_vst3 tests"
    OFF
)


if(MOPLUG_VST3_BUILD_TESTS)

    enable_testing()

    add_executable(
        moplug_vst3_bridge_tests

        tests/bridge_test.cpp
        tests/mock_mojo.cpp
    )

    target_include_directories(
        moplug_vst3_bridge_tests

        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
            "${VST3_SDK_ROOT}"
    )

    add_test(
        NAME moplug_vst3_bridge_tests
        COMMAND moplug_vst3_bridge_tests
    )

endif()
