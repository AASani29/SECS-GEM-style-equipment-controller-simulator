#include "EquipmentState.h"

#include <iostream>

std::unique_ptr<EquipmentState> EquipmentState::logIgnored(CommandType command) const {
    std::cout << "[Equipment] " << name() << " ignored command " << toString(command) << "\n";
    return nullptr;
}
