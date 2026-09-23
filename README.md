# SECS/GEM-Style Equipment Controller Simulator

Work in progress. A simplified, educational SECS/GEM-style equipment
controller simulator built in stages to learn C++, Qt, and multithreading.

A full README (architecture diagram, build/run instructions, and an
explicit "this is not a certified SEMI E5/E30 implementation" note) will
be added once all stages are complete.

## Stage 1: Core state machine (current)

Console-only demo, no Qt yet. Builds an `Equipment` state machine (State
design pattern: `IDLE` / `SETUP` / `PROCESSING` / `ALARM` / `COMPLETE`)
driven by a worker thread that consumes commands from a thread-safe queue.

### Build

Requires CMake 3.16+ and a C++17 compiler with real thread support (e.g.
the MinGW toolchain bundled with the Qt installer).

```
cmake -S . -B build
cmake --build build
```

### Run

```
build\secs_gem_simulator.exe
```
