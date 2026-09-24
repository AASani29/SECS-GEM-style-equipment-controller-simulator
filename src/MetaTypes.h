#pragma once

#include "Command.h"
#include "StateId.h"

#include <QMetaType>

// A queued signal/slot connection has to COPY its arguments into an event
// that gets posted to the receiving thread. Qt can only do that for types
// it has been told about, so our own enums that travel through signals
// are declared here and registered once at startup.
Q_DECLARE_METATYPE(StateId)
Q_DECLARE_METATYPE(CommandType)

void registerMetaTypes();
