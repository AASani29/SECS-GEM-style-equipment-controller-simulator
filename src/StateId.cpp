#include "StateId.h"

std::string toString(StateId id) {
    switch (id) {
        case StateId::Idle:       return "IDLE";
        case StateId::Setup:      return "SETUP";
        case StateId::Processing: return "PROCESSING";
        case StateId::Alarm:      return "ALARM";
        case StateId::Complete:   return "COMPLETE";
    }
    return "UNKNOWN";
}
