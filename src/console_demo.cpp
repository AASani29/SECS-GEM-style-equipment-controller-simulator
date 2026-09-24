#include "EquipmentController.h"

#include <chrono>
#include <iostream>
#include <thread>

// Stage 1 console demo: no networking, no GUI yet. This just proves the
// state machine + queue + worker thread work together correctly before
// we add any Qt on top of it.
int main() {
    EquipmentController controller;
    controller.start();

    std::cout << "Sending StartJob...\n";
    controller.submitCommand(CommandType::StartJob);

    // Give the simulated SETUP (1.5s) and PROCESSING (2.5s) delays time
    // to run on the worker thread while we just sleep here.
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Current state: " << controller.currentStateName() << "\n\n";

    std::cout << "Triggering alarm...\n";
    controller.submitCommand(CommandType::TriggerAlarm);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Current state: " << controller.currentStateName() << "\n\n";

    std::cout << "Resetting...\n";
    controller.submitCommand(CommandType::Reset);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Current state: " << controller.currentStateName() << "\n";

    controller.stop();
    return 0;
}
