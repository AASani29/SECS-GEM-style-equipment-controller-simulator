# SECS/GEM-Style Equipment Controller Simulator

A small C++17 / Qt 6 project that simulates a semiconductor equipment
controller and a host talking to each other over TCP, using a simplified
SECS-II-style message format. It was built to learn C++, Qt, and
multithreading, and it is meant to be readable rather than clever.

> **This is a simplified, educational subset of SECS-II. It is NOT a
> certified or complete implementation of SEMI E5 (SECS-II), SEMI E30
> (GEM), or SEMI E37 (HSMS), and it must not be used to control or
> connect to real equipment.** Message names and stream/function numbers
> follow the real standards so the ideas carry over, but the wire format,
> data encoding, and behaviour are deliberately simplified. See
> [Differences from the real standards](#differences-from-the-real-standards).

## What it does

- An **equipment** state machine with five states: `IDLE`, `SETUP`,
  `PROCESSING`, `ALARM`, `COMPLETE` (State design pattern).
- A **worker thread** that takes commands from a thread-safe queue and
  drives the state machine, with simulated setup/processing delays.
- An **equipment server** and a **host client** (Qt `QTcpServer` /
  `QTcpSocket`) that exchange length-prefixed messages on a separate
  **network thread**.
- A **Qt Widgets dashboard**: a colour-coded state label, a scrolling
  message log, and buttons to connect the host, send messages, and
  trigger a simulated alarm.

The host and equipment run in the same process and talk over
`127.0.0.1:15000`, so you can watch both sides of the conversation in one
window.

## Architecture

```
 +------------------------------+
 |  GUI thread                  |
 |  MainWindow (QWidgets)       |
 +------------------------------+
     |  ^
     |  |  Qt signals / slots, queued connections
     v  |  (arguments are copied between threads)
 +-----------------------------------------------------------+
 |  Network thread (QThread, runs an event loop)              |
 |                                                            |
 |   HostClient  --- S1F1 / S2F41 --->  EquipmentServer       |
 |   (QTcpSocket) <-- S1F2 / S2F42 --- (QTcpServer + socket)  |
 |                <-- S6F11 event ----                        |
 |                --- S6F12 ack ----->                        |
 |                                                            |
 |   EquipmentAdapter  (Qt facade over the controller)        |
 +-----------------------------------------------------------+
     |  ^
     |  |  push onto the command queue (mutex + condition variable)
     v  |  callbacks -> emit signals
 +------------------------------+
 |  Equipment worker thread     |
 |  (std::thread)               |
 |                              |
 |  CommandQueue -> Equipment   |
 |     (State pattern:          |
 |      Idle / Setup /          |
 |      Processing / Alarm /    |
 |      Complete)               |
 +------------------------------+
```

There are three threads:

| Thread | Owns | Notes |
|---|---|---|
| GUI thread | `QApplication`, `MainWindow` | Only thread allowed to touch widgets |
| Network thread | `EquipmentServer`, `HostClient`, their sockets, `EquipmentAdapter` | A `QThread` running its own event loop |
| Equipment worker | `EquipmentController` (`Equipment`, `CommandQueue`) | A plain `std::thread`, no Qt inside |

### Why signals and slots are safe here

- **Sockets are never shared.** A `QTcpSocket` is not thread-safe: its
  buffers and its notifications from the OS belong to the thread that owns
  it. So each socket is created and used only on the network thread. No
  other thread calls a method on it.
- **Threads talk by queued connections.** Every connection that crosses a
  thread boundary is `Qt::QueuedConnection`. When a signal is emitted, Qt
  copies the arguments into an event and posts it to the *receiver's*
  thread. The receiver's own event loop later runs the slot on the
  receiver's thread. The emitting thread never runs the receiver's code,
  and no data is shared, only copies, so no lock is needed. (That is also
  why custom types sent this way are registered in `MetaTypes.cpp`.)
- **The equipment worker is not a Qt thread**, so it has no event loop to
  receive slot calls. Instead it is reached through the mutex-protected
  `CommandQueue`, and it reports back through callbacks that just `emit`
  signals (see `EquipmentAdapter`).

### Why the GUI is only updated from the GUI thread

`QWidget` and its subclasses are not thread-safe. Changing a label from
another thread races with the GUI thread painting it, and can corrupt
Qt's internal state or crash. So background threads never call
`setText()` and friends. They emit a signal instead, and the queued
connection makes `MainWindow`'s slot run on the GUI thread, where it is
safe to touch the widgets.

## State machine

| Current state | Command | Next state |
|---|---|---|
| `IDLE` | `StartJob` | `SETUP` |
| `SETUP` | setup timer elapsed | `PROCESSING` |
| `PROCESSING` | processing timer elapsed | `COMPLETE` |
| `COMPLETE` | `Reset` | `IDLE` |
| `ALARM` | `Reset` | `IDLE` |
| `IDLE`, `SETUP`, `PROCESSING`, `COMPLETE` | `TriggerAlarm` | `ALARM` |

Any other command in a given state is ignored and logged. Setup takes
1.5 s and processing takes 2.5 s (simulated).

## Message set

| Message | Direction | Meaning | Payload (text) |
|---|---|---|---|
| S1F1 | Host -> Equipment | Are You There | (empty) |
| S1F2 | Equipment -> Host | On Line Data | `MDLN=SIM-EQP;SOFTREV=1.0` |
| S2F41 | Host -> Equipment | Remote command | `RCMD=START` or `RCMD=RESET` |
| S2F42 | Equipment -> Host | Command acknowledge | `HCACK=4` (accepted) or `HCACK=1` (unknown command) |
| S6F11 | Equipment -> Host | Event report on state change | `CEID=STATE_CHANGE;FROM=IDLE;TO=SETUP` |
| S6F12 | Host -> Equipment | Event report acknowledge | `ACKC6=0` |

### Wire format

TCP delivers a byte stream, so each message is length-prefixed and a
framer (`SecsFramer`) reassembles whole messages from whatever chunks
arrive. All multi-byte numbers are big-endian.

```
bytes 0-3    length: number of bytes after this field (6 + payload size)
byte  4      stream
byte  5      function
bytes 6-9    transaction id (a reply repeats the id of its request)
bytes 10...  payload
```

## Project layout

| File(s) | Role |
|---|---|
| `StateId`, `Command` | State and command enums with `toString()` |
| `EquipmentState`, `EquipmentStates` | State pattern: base class and the five states |
| `Equipment` | State machine; holds the current state |
| `CommandQueue` | Thread-safe queue (`std::mutex` + `std::condition_variable`) |
| `EquipmentController` | Owns the worker thread; plain C++, no Qt |
| `SecsMessage`, `SecsFramer` | Message struct/builders and the byte-stream framer |
| `EquipmentAdapter` | Bridges the controller to Qt signals and slots |
| `EquipmentServer`, `HostClient` | The two ends of the TCP link |
| `MainWindow` | The dashboard |
| `MetaTypes` | Registers custom types for queued connections |
| `main.cpp` | Creates the objects and wires every connection |
| `console_demo.cpp` | Stage 1 demo: the state machine with no Qt |

## Build and run

Requirements: a C++17 compiler with real thread support, CMake 3.16+, and
Qt 6 (Core, Network, Widgets). On Windows, the simplest route is the Qt
Online Installer with the **MinGW 64-bit** kit plus the **CMake** and
**Ninja** components.

### Qt Creator

1. *File > Open File or Project...* and choose `CMakeLists.txt`.
2. Select a **Desktop Qt 6.x MinGW 64-bit** kit.
3. Build, then run the `secs_gem_simulator` target.

### Command line

Make sure Qt's MinGW compiler, CMake, and Ninja are on your `PATH`, then:

```
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.x.y/mingw_64
cmake --build build
```

Put Qt's `bin` folder (for example `C:\Qt\6.x.y\mingw_64\bin`) on `PATH`
so the executable can find the Qt DLLs, then run:

```
build\secs_gem_simulator.exe
```

If CMake cannot find Qt, only the Qt-free `console_demo` target is built:

```
build\console_demo.exe
```

### Using the dashboard

1. Click **Connect Host**. The link label turns to "connected".
2. Click **Send S1F1** to see the handshake in the log.
3. Choose `START` and click **Send S2F41 Remote Command**. Watch the state
   go `IDLE -> SETUP -> PROCESSING -> COMPLETE`, with an S6F11 event
   report in the log for each change.
4. Click **Trigger Alarm** at any time to force `ALARM`. Send `RESET` to
   return to `IDLE`.

## Differences from the real standards

- **Transport:** plain TCP with a made-up header, not HSMS (SEMI E37).
  There is no session/select/linktest handling, no T3/T5/T6/T7/T8 timers,
  and no reconnect logic.
- **Encoding:** payloads are `KEY=value` ASCII text, not SECS-II data
  items (lists, `U1`, `A`, and so on).
- **Header:** no device/session id and no W-bit (reply-expected) flag.
- **Messages:** only the six listed above. There is no S9 error reporting,
  no S1F13 communications establishment, no S2F33/S2F35 report setup, and
  no collection events, variables, or alarms as defined by GEM (E30).
- **Command result:** `S2F42` reports `HCACK=4` ("will be performed, done
  is signalled by an event"). Whether the state machine actually accepts
  the command is only reported later, in the log and the S6F11.
- **Connections:** one host at a time, loopback only.
