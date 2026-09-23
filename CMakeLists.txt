cmake_minimum_required(VERSION 3.16)
project(SecsGemSimulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Stage 1 needs real std::thread/std::mutex support, which requires an
# actual threading library to be found and linked in.
find_package(Threads REQUIRED)

add_executable(secs_gem_simulator
    src/main.cpp
    src/StateId.cpp
    src/Command.cpp
    src/EquipmentState.cpp
    src/EquipmentStates.cpp
    src/Equipment.cpp
    src/CommandQueue.cpp
    src/EquipmentController.cpp
)

target_link_libraries(secs_gem_simulator PRIVATE Threads::Threads)
