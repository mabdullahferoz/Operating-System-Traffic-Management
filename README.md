# Smart City Traffic Control System

## Overview
This is a C-based simulation of a Smart City Traffic Control System using Linux OS primitives:
- **Processes**: `fork()` for vehicle generation.
- **Threads**: `pthread` for lane monitoring sensors.
- **IPC**: POSIX Message Queues (`mq_open`, `mq_send`) for communication.
- **Synchronization**: Mutexes for Critical Section (Intersection) management.
- **Scheduling**: Priority-based handling for Emergency vehicles + Aging for fairness.

## Prerequisites
**This project requires a Linux Environment system.**
It will **NOT** compile on standard Windows (MinGW/MSVC) because it uses:
- `<mqueue.h>`
- `<sys/wait.h>`
- `fork()`

### Recommended Environments:
1.  **WSL (Windows Subsystem for Linux)** - *Easiest for Windows users*
2.  **Ubuntu / Debian / Fedora** VM or native install.

## Compilation
1.  Open your terminal (in WSL or Linux).
2.  Navigate to the project directory.
3.  Run:
    ```bash
    make
    ```
    This will generate the `traffic_system` executable.

## Running
Run the executable:
```bash
./traffic_system
```

## Usage
- **Automatic**: Vehicles are generated randomly.
- **Interactive**: 
  - `1`, `2`, `3`, `4`: Add Regular Car to North, South, East, West.
  - `a`, `b`, `c`, `d`: Add Emergency Vehicle to North, South, East, West.

- Watch the Dashboard for real-time status.
- **Green** indicates the active lane.
- **Red** indicates stopped lanes.
- Emergency vehicles (AMB, POL, FIRE) will jump the queue.
- Press `Ctrl+C` to safely stop the system and clean up resources (Queues/Mutexes).

## Troubleshooting
- **mq_open failed**: Ensure you have permissions or the queue wasn't cleaned up. The code tries to `unlink` old queues automatically, but you can manually run `rm /dev/mqueue/*` if needed.
