#pragma once

#include "Command.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

// A thread-safe hand-off point between "whoever wants Equipment to do
// something" (main thread, GUI thread, network thread - any of them) and
// the one worker thread that actually drives the state machine. Any
// thread may call push(); only the worker thread is expected to call
// the wait functions.
class CommandQueue {
public:
    void push(Command command);

    // Blocks the calling thread until a command is available, then
    // returns it.
    Command waitAndPop();

    // Like waitAndPop(), but gives up at `deadline`. Returns an empty
    // optional if no command arrived in time. The worker uses this while
    // a simulated step (SETUP/PROCESSING) is running, so an alarm or
    // shutdown request is noticed immediately instead of after a sleep.
    std::optional<Command> waitAndPopUntil(std::chrono::steady_clock::time_point deadline);

private:
    std::queue<Command> queue_;
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
};
