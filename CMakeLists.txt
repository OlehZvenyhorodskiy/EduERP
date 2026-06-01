cmake_minimum_required(VERSION 3.20...3.31)

project(EduERP VERSION 1.0.0 LANGUAGES CXX)

# ── C++ Standard ──
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── Build Options ──
option(EDUERP_BUILD_TESTS "Build EduERP tests" ON)
option(EDUERP_BUILD_DOCS "Build documentation" OFF)
option(EDUERP_ENABLE_SANITIZERS "Enable sanitizers (ASan, UBSan)" OFF)
option(EDUERP_STATIC_ANALYSIS "Enable clang-tidy static analysis" OFF)

# ── Find Qt6 ──
find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Network
    WebSockets
    Charts
)

# ── Find vcpkg Dependencies ──
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)

# ── Source File Collection ──
file(GLOB_RECURSE CORE_SOURCES
    "src/core/*.h" "src/core/*.cpp"
)
file(GLOB_RECURSE DOMAIN_SOURCES
    "src/domain/*.h" "src/domain/*.cpp"
)
file(GLOB_RECURSE SIM_SOURCES
    "src/simulation/*.h" "src/simulation/*.cpp"
)
file(GLOB_RECURSE SERVICES_SOURCES
    "src/services/*.h" "src/services/*.cpp"
)
file(GLOB_RECURSE INFRA_SOURCES
    "src/infrastructure/*.h" "src/infrastructure/*.cpp"
)
file(GLOB_RECURSE CONTROLLER_SOURCES
    "src/controllers/*.h" "src/controllers/*.cpp"
)
file(GLOB_RECURSE APP_SOURCES
    "src/app/*.h" "src/app/*.cpp"
)

# ── QML Files ──
file(GLOB_RECURSE QML_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "src/ui/*.qml"
)

# ── Main Executable ──
qt_add_executable(EduERP WIN32
    src/main.cpp
    ${CORE_SOURCES}
    ${DOMAIN_SOURCES}
    ${SIM_SOURCES}
    ${SERVICES_SOURCES}
    ${INFRA_SOURCES}
    ${CONTROLLER_SOURCES}
    ${APP_SOURCES}
    src/infrastructure/i18n/i18n.qrc   # Bundles translation JSON files
)

qt_policy(SET QTP0001 OLD)

qt_add_qml_module(EduERP
    URI EduERP
    VERSION 1.0
    RESOURCE_PREFIX /
    QML_FILES ${QML_FILES}
)

# ── Include Directories ──
target_include_directories(EduERP PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/controllers
    ${CMAKE_SOURCE_DIR}/src/infrastructure/i18n
    ${CMAKE_SOURCE_DIR}/src/services/i18n
)

# ── Link Libraries ──
target_link_libraries(EduERP PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::WebSockets
    Qt6::Charts
    SQLiteCpp
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    jwt-cpp::jwt-cpp
)

# ── Compiler Warnings ──
if(MSVC)
    target_compile_options(EduERP PRIVATE /W4 /permissive-)
else()
    target_compile_options(EduERP PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
endif()

# ── Sanitizers ──
if(EDUERP_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(EduERP PRIVATE -fsanitize=address,undefined)
    target_link_options(EduERP PRIVATE -fsanitize=address,undefined)
endif()

# ── Static Analysis ──
if(EDUERP_STATIC_ANALYSIS)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*,-fuchsia*,-llvmlibc*")
endif()

# ── Tests ──
if(EDUERP_BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
    if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()

# ── Installation ──
install(TARGETS EduERP
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# ── CPack (Windows Installer) ──
set(CPACK_PACKAGE_NAME "EduERP")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "EduERP Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Educational ERP Simulation for Belgian Schools")
set(CPACK_GENERATOR "NSIS")
include(CPack)
