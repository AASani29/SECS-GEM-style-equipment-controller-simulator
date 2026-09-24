cmake_minimum_required(VERSION 3.16)
project(SecsGemSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Real std::thread/std::mutex support needs an actual threading library.
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------
# Stage 1: equipment core (state machine + command queue + worker thread)
# Plain C++17, no Qt. Built as a library so both the console demo and the
# Qt application below can share it.
# ---------------------------------------------------------------------
add_library(equipment_core STATIC
    src/StateId.cpp
    src/Command.cpp
    src/EquipmentState.cpp
    src/EquipmentStates.cpp
    src/Equipment.cpp
    src/CommandQueue.cpp
    src/EquipmentController.cpp
)
target_include_directories(equipment_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(equipment_core PUBLIC Threads::Threads)

# Stage 1 console demo: works without Qt installed.
add_executable(console_demo src/console_demo.cpp)
target_link_libraries(console_demo PRIVATE equipment_core)

# ---------------------------------------------------------------------
# Stages 2 + 3: Qt networking layer and GUI
# If Qt 6 isn't found, only the console demo above is built. When
# configuring from the command line, point CMake at your Qt install, e.g.
#   -DCMAKE_PREFIX_PATH=C:/Qt/6.x.y/mingw_64
# (Qt Creator does this for you through the selected kit.)
# ---------------------------------------------------------------------
find_package(Qt6 COMPONENTS Core Network Widgets)

if(Qt6_FOUND)
    # Automatically runs Qt's "moc" on headers that contain Q_OBJECT.
    set(CMAKE_AUTOMOC ON)

    add_executable(secs_gem_simulator
        src/main.cpp
        src/MetaTypes.h
        src/MetaTypes.cpp
        src/SecsMessage.h
        src/SecsMessage.cpp
        src/SecsFramer.h
        src/SecsFramer.cpp
        src/EquipmentAdapter.h
        src/EquipmentAdapter.cpp
        src/EquipmentServer.h
        src/EquipmentServer.cpp
        src/HostClient.h
        src/HostClient.cpp
        src/MainWindow.h
        src/MainWindow.cpp
    )
    target_link_libraries(secs_gem_simulator PRIVATE
        equipment_core
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
    )
else()
    message(WARNING "Qt 6 not found - building only the Stage 1 console demo. "
                    "Set CMAKE_PREFIX_PATH to your Qt install to build the GUI.")
endif()
