cmake_minimum_required(VERSION 4.1)
project(seat_allocator_v3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenXLSX CONFIG REQUIRED)

qt_standard_project_setup()

qt_add_executable(seat_allocator_v3
    main.cpp

    src/utils/utils.cpp

    src/algorithm/algorithm.cpp

    src/widgets/mainwindow.cpp

    src/ui/mainwindow.ui
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(seat_allocator_v3 PRIVATE LOG_UNTIL_DEBUG)
endif()

target_include_directories(seat_allocator_v3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

set_target_properties(seat_allocator_v3 PROPERTIES
    AUTOUIC ON
    AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    AUTORCC ON
)

target_link_libraries(seat_allocator_v3 PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    stdc++exp
    spdlog::spdlog
    OpenXLSX::OpenXLSX
)