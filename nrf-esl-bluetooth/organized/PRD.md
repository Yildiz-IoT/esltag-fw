# Project Requirements Document (PRD): ESL Tag Firmware Refactor

## 1. Project Brief

**Goal:** To refactor the existing ESL tag firmware into a new, well-structured Zephyr RTOS application. The aim is to improve modularity, maintainability, and scalability while leveraging the core logic from the original `service/` and `samples/peripheral_esl/` directories.

**Target Platform:** nRF-based hardware (e.g., nRF52833 DK initially), using the Bluetooth SIG ESL profile for communication.

## 2. Tech Stack

*   **RTOS:** Zephyr Project
*   **Hardware:** Nordic Semiconductor nRF series (initially nRF52833)
*   **Language:** C
*   **Build System:** CMake / West
*   **Core Protocol:** Bluetooth Low Energy (BLE), Bluetooth SIG ESL GATT Service

## 3. Proposed Directory Structure

```
organized/
├── app/                  # Main application logic, state management
│   ├── include/
│   └── src/
│       └── main.c
├── bsp/                  # Board Support Package (Hardware specific init)
│   ├── include/
│   └── src/
├── drivers/              # Hardware drivers (EPD, buttons, sensors etc.)
│   ├── include/
│   └── src/
│       ├── epd/
│       └── button/
├── esl/                  # Core ESL protocol implementation (Refactored from service/)
│   ├── include/
│   └── src/
├── bluetooth/            # Generic Bluetooth management (advertising, connection)
│   ├── include/
│   └── src/
├── config/               # Application configuration (prj.conf, Kconfig fragments)
├── boards/               # Board-specific device tree overlays and config
├── CMakeLists.txt        # Main CMake build script
├── Kconfig               # Main Kconfig file
├── PRD.md                # This file
└── zephyr/               # Zephyr RTOS (as submodule or managed by west) - TBD
```

## 4. ToDo

*   [ ] **Setup & Planning:**
    *   [ ] Finalize detailed project requirements (specific ESL features, power constraints, etc.) - *Requires User Input*
    *   [ ] Set up basic Zephyr project structure (`organized/CMakeLists.txt`, `organized/Kconfig`).
    *   [ ] Decide on Zephyr integration method (submodule vs. west manifest).
*   [ ] **Core Bluetooth & ESL Service:**
    *   [ ] Create `bluetooth/` module structure and basic files.
    *   [ ] Implement basic BLE advertising and connection management using Zephyr APIs.
    *   [ ] Create `esl/` module structure and basic files.
    *   [ ] Define ESL GATT service structure using Zephyr GATT APIs.
    *   [ ] Refactor relevant peripheral-role code from `service/esl.*` and `service/esl_common.*` into `esl/` and `bluetooth/`.
*   [ ] **Hardware Abstraction (Drivers & BSP):**
    *   [ ] Identify required drivers (EPD, Button).
    *   [ ] Create `drivers/` module structure.
    *   [ ] Refactor EPD driver from `samples/peripheral_esl/driver` or `hw` into `drivers/epd/`.
    *   [ ] Create Button driver in `drivers/button/`.
    *   [ ] Create `bsp/` module structure.
    *   [ ] Implement BSP for the target board (e.g., nRF52833DK) in `bsp/`.
*   [ ] **Application Logic (`app`):**
    *   [ ] Create `app/` module structure (`include/`, `src/main.c`).
    *   [ ] Implement main application state machine skeleton.
    *   [ ] Integrate module initializations (BSP, drivers, Bluetooth, ESL).
    *   [ ] Implement basic logic to handle ESL events and update display.
*   [ ] **Integration & Configuration:**
    *   [ ] Create initial `config/prj.conf`.
    *   [ ] Create initial board overlay in `boards/`.
    *   [ ] Ensure all modules are correctly linked via CMake.
*   [ ] **Testing & Refinement:**
    *   [ ] Perform initial build and flash test.
    *   [ ] Test basic advertising and connection.
    *   [ ] Test ESL service discovery and basic command handling.
    *   [ ] Test EPD updates.
    *   [ ] Optimize and refactor as needed.

## 5. Done

*   [X] Created `organized/` directory.
*   [X] Created initial `PRD.md`.

---

*This PRD will be updated as the project progresses.* 