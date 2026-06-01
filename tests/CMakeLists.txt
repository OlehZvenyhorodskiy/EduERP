cmake_minimum_required(VERSION 3.20)

# ── Unit test suite for EduERP ──
# Each subdirectory maps to one domain area.
# Uses GTest + cmake DISCOVER_TESTS for CTest integration.

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

enable_testing()

# ── Helper to register a test executable ──
function(eduerp_add_test name sources)
    add_executable(${name} ${sources})
    target_include_directories(${name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}
    )
    target_link_libraries(${name} PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        Qt6::Core
        Qt6::Network
    )
    gtest_discover_tests(${name})
endfunction()

# ── Test suites ──
add_subdirectory(unit/services)
add_subdirectory(unit/simulation)
add_subdirectory(integration)
