#pragma once

#include "Command.h"

#include <condition_variable>
#include <mutex>
#include <queue>

// A thread-safe hand-off point between "whoever wants Equipment to do
// something" (main thread, GUI thread, network thread - any of them) and
// the one worker thread that actually drives the state machine. Any
// thread may call push(); only the worker thread is expected to call
// waitAndPop().
class CommandQueue {
public:
    void push(Command command);

    // Blocks the calling thread until a command is available, then
    // returns it.
    Command waitAndPop();

private:
    std::queue<Command> queue_;
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
};
