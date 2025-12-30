# Smart City Traffic Control System

## Overview
A high-performance simulation of an automated traffic control system for a 4-way intersection (North, South, East, West). Developed in **C** for **Linux** environments, this project uses advanced Operating System concepts including **Multi-threading**, **Process Management**, **Inter-Process Communication (IPC)**, and **Synchronization**.

The generic "Round Robin" scheduler has been enhanced with **Preemptive Priority Scheduling** to ensure Emergency Vehicles (Ambulances, Police, Fire Trucks) are never stuck in traffic.

## Features

### 🚦 Intelligent Traffic Scheduling
-   **Priority Handling**: Emergency vehicles jump the queue immediately.
-   **Preemption**: If a normal lane is Green and an Emergency Vehicle arrives in another lane, the current Green light is **terminated immediately** to serve the emergency.
-   **Fair Emergency Rotation**: If multiple lanes have emergency vehicles, they are served in a Round-Robin fashion (One-by-One) to prevent starvation.
-   **Timer-Based Flow**: Normal traffic flows for a fixed duration (e.g., 8 seconds), allowing multiple cars to pass per cycle.

### 🖥️ Real-Time Visualization
-   **Live Dashboard**: Displays the status (GO/STOP), queue size, and waiting vehicles for all 4 lanes.
-   **Process Log**: Tracks the history of vehicles that have successfully crossed, highlighting Emergency vehicles in **Bold Red**.
-   **Graphical Intersection**: A clean, ANSI-colored ASCII art visualization of the intersection that shows:
    -   Active lanes lighting up in **GREEN**.
    -   Stopped lanes in **RED**.
    -   Vehicles physically moving across the center junction.

### 🎮 Interactive & Automatic Modes
-   **Background Generation**: Vehicles arrive randomly (simulating real traffic).
-   **Manual Control**: You can inject traffic manually to test specific scenarios (e.g., creating a traffic jam or testing emergency preemption).

---

## Technical Architecture

The project is modularized into 4 key components:

1.  **`main.c`**: The central controller.
    -   Manages the main event loop.
    -   Handles the **Green Light Timer** and **Preemption Logic**.
    -   Spawns the Vehicle Generator process (`fork()`) and Input Thread.
2.  **`traffic_logic.c`**: Core scheduling algorithms.
    -   Determines the next lane based on priority rules.
    -   Manages **Mutexes** (`pthread_mutex_t`) to protect critical sections (queues).
3.  **`ipc_manager.c`**: Inter-Process Communication.
    -   Uses **POSIX Message Queues** (`mq_open`, `mq_send`) to receive vehicle data from the generator process safely.
4.  **`utils.c`**: Data Structures & UI.
    -   Implements **Linked Lists** to manage vehicle queues dynamically.
    -   Handles the complex ANSI drawing logic for the dashboard.

---

## Installation & Usage

### Prerequisites
-   **Operating System**: Linux (Ubuntu, Kali, Debian, etc.) or **WSL** (Windows Subsystem for Linux).
-   **Compiler**: `gcc`
-   **Build Tool**: `make`

### Compilation
Open your terminal in the project directory and run:
```bash
make
```
This generates the executable `traffic_system`.

### Running the Simulation
```bash
./traffic_system
```

### Controls
While the simulation is running, use these keys to add vehicles instantly:

| Key | Action |
| :---: | :--- |
| `1` | Add **Car** to **North** Lane |
| `2` | Add **Car** to **South** Lane |
| `3` | Add **Car** to **East** Lane |
| `4` | Add **Car** to **West** Lane |
| `a` | Add **Ambulance** (Priority) to **North** |
| `b` | Add **Ambulance** (Priority) to **South** |
| `c` | Add **Ambulance** (Priority) to **East** |
| `d` | Add **Ambulance** (Priority) to **West** |

To exit, press `Ctrl+C`.

---

## Scenarios to Test
1.  **Standard Flow**: Let it run automatically to see the Round Robin timer switching lanes every few seconds.
2.  **Emergency Jump**: Press `1` five times to fill North with cars. Then press `a` (Emergency North). Watch the Ambulance jump to the front of the text queue.
3.  **Preemption**: Wait for the **East** lane to turn Green. Immediately press `a` (Emergency North). Watch the East lane turn Red instantly to let North go.
