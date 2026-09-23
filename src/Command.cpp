#include "Command.h"

std::string toString(CommandType type) {
    switch (type) {
        case CommandType::StartJob:           return "StartJob";
        case CommandType::SetupFinished:      return "SetupFinished";
        case CommandType::ProcessingFinished: return "ProcessingFinished";
        case CommandType::Reset:              return "Reset";
        case CommandType::TriggerAlarm:       return "TriggerAlarm";
        case CommandType::Shutdown:           return "Shutdown";
    }
    return "Unknown";
}
