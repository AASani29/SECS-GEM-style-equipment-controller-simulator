#include "CommandQueue.h"

void CommandQueue::push(Command command) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(command);
    }
    // Notify after releasing the lock: it means the thread we wake up
    // isn't immediately blocked again waiting for a mutex we're still
    // holding. Not required for correctness, just avoids a pointless
    // context switch.
    notEmpty_.notify_one();
}

Command CommandQueue::waitAndPop() {
    std::unique_lock<std::mutex> lock(mutex_);
    // The predicate (second argument) protects us from spurious wakeups:
    // condition_variable::wait() is allowed to return even if nobody
    // called notify_*(), so we always re-check the actual condition
    // ourselves instead of trusting that a wakeup means "there's data".
    notEmpty_.wait(lock, [this] { return !queue_.empty(); });

    Command command = queue_.front();
    queue_.pop();
    return command;
}
