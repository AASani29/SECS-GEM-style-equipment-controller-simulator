#include "MetaTypes.h"

void registerMetaTypes() {
    qRegisterMetaType<StateId>("StateId");
    qRegisterMetaType<CommandType>("CommandType");
}
