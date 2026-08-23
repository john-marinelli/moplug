cmake_minimum_required(VERSION 3.25)

project(
    mojo_vst3
    VERSION 0.1.0
    DESCRIPTION "VST3 backend for Mojo audio plugins"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/mojo"
)

set(MOJO_BUILD_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/mojo"
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

set(
    vst3sdk_SOURCE_DIR
    "${VST3_SDK_ROOT}"
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
    REQUIRED
)

set(
    MOJO_BRIDGE_OBJECT
    "${MOJO_BUILD_DIR}/bridge.o"
)

add_custom_command(
    OUTPUT "${MOJO_BRIDGE_OBJECT}"

    COMMAND
        "${MOJO_EXECUTABLE}"
        build
        "${MOJO_SOURCE_DIR}/bridge.mojo"
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
    DEPENDS "${MOJO_BRIDGE_OBJECT}"
)

# plugin

smtg_add_vst3plugin(
    mojo_vst3_plugin

    include/bridge.h
    include/ids.h
    include/processor.h
    include/controller.h

    native/processor.cpp
    native/controller.cpp
    native/factory.cpp
)

add_dependencies(
    mojo_vst3_plugin
    mojo_bridge
)

target_include_directories(
    mojo_vst3_plugin

    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${VST3_SDK_ROOT}"
)

target_compile_definitions(
    mojo_vst3_plugin
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

target_sources(
    mojo_vst3_plugin

    PRIVATE
        "${MOJO_BRIDGE_OBJECT}"
)

target_link_libraries(
    mojo_vst3_plugin

    PRIVATE
        sdk
)


if(MSVC)

    target_compile_options(
        mojo_vst3_plugin
        PRIVATE
            /W4
    )

else()

    target_compile_options(
        mojo_vst3_plugin
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

# metadata

smtg_target_configure_version_file(
    mojo_vst3_plugin
)

# tests

option(
    MOJO_VST3_BUILD_TESTS
    "Build mojo_vst3 tests"
    ON
)

if(MOJO_VST3_BUILD_TESTS)

    enable_testing()

    add_executable(
        mojo_vst3_bridge_tests

        tests/bridge_test.cpp
        tests/mock_mojo.cpp
    )

    target_include_directories(
        mojo_vst3_bridge_tests

        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
	    "${VST3_SDK_ROOT}"
    )

    add_test(
        NAME mojo_vst3_bridge_tests
        COMMAND mojo_vst3_bridge_tests
    )

endif()
