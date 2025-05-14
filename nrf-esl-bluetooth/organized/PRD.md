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
├── tag/                    # ESL Tag Firmware Application
│   ├── app/                #   Main application logic & state management (.c, .h files)
│   ├── bsp/                #   Board Support Package (.c, .h files)
│   ├── drivers/            #   Hardware drivers
│   │   ├── epd/            #     E-Paper Display driver (.c, .h files)
│   │   └── button/         #     Button driver (.c, .h files)
│   ├── bluetooth/          #   Bluetooth LE management & ESL Service
│   │   └── esl/            #     ESL specific service/profile logic (.c, .h files)
│   │   # Generic BLE mgmt (.c, .h files) would also go here
│   ├── config/             #   Application configuration (prj.conf, Kconfig fragments)
│   ├── boards/             #   Board-specific device tree overlays and config
│   ├── CMakeLists.txt      #   Main CMake build script for the tag
│   └── Kconfig             #   Main Kconfig file for the tag
├── access_point/           # ESL Access Point Firmware Application (Placeholder)
│   └── ...               #   (Structure TBD)
├── PRD.md                  # This file (describes overall reorganization)
└── zephyr/                 # Zephyr RTOS (location/management TBD)
```

## 4. Development Principles

*   **Structure First:** Focus on establishing the complete directory structure, basic build files (`CMakeLists.txt`, `Kconfig`), and configuration files (`prj.conf`, board overlays) for the **tag** firmware before refactoring or copying detailed source code logic (*.c, *.h) from the original project.
*   **Modular Design:** Aim for clear separation of concerns between modules within the tag firmware (`app`, `bsp`, `drivers`, `bluetooth`).
*   **PRD Driven:** Follow the steps outlined in the ToDo section below. Update the PRD as tasks are completed or requirements change.

## 5. ToDo

*   [ ] **Setup & Planning (Tag Firmware):**
    *   [X] Create `organized/tag/` directory structure.
    *   [~] Finalize detailed project requirements for the **tag**. *(Initial details gathered; further refinement as needed during development)*
        *   **JSON Update Method:** As defined in the root `PRD.MD` (Section 2), using a JSON object with fields like `prod`, `price`, `barcode`, `pos`, `format`, and `color` for partial or full display updates. Key aspects:
            *   Field list (`prod`, `brnd`, `price`, `unit`, `exp`, `barcode`) is fixed for now.
            *   `format` array values (bold, size, style) will be mapped to enums/macros.
            *   `pos` coordinates are absolute pixel values.
            *   The tag will render text/barcodes based on this JSON and update the EPD.
        *   **Tag Lifecycle States:** (Manufacturing, Testing, Inventory, Provisioned, Assigned Store, Assigned Product, Active, Deactivate) - See details from provided table.
            *   Display of company/store name in Provisioned/Assigned Store states: Method TBD (likely AP or NFC).
            *   In "Deactivate" state, the EPD will display "Disconnected".
            *   Tag status changes are reported to the backend by the Access Point upon reconnection.
            *   Requires a robust state machine and NVM for persisting state.
    *   [X] Set up basic Zephyr project structure for the tag (`organized/tag/CMakeLists.txt`, `organized/tag/Kconfig`).
*   [ ] **Core Bluetooth & ESL Service (Tag Firmware):**
    *   [X] Create `organized/tag/bluetooth/` module structure and copy all relevant files from `service/`.
    *   [ ] Implement basic BLE advertising and connection management using Zephyr APIs in `organized/tag/bluetooth/`.
    *   [ ] Create `organized/tag/bluetooth/esl/` structure and basic files.
    *   [ ] Define ESL GATT service structure using Zephyr GATT APIs in `organized/tag/bluetooth/esl/`.
    *   [ ] Refactor relevant peripheral-role code from `service/esl.*` and `service/esl_common.*` into `organized/tag/bluetooth/` and `organized/tag/bluetooth/esl/`.
*   [ ] **Hardware Abstraction (Drivers & BSP - Tag Firmware):**
    *   [ ] Identify required drivers for the **tag** (EPD, Button).
    *   [ ] Create `organized/tag/drivers/` module structure.
    *   [ ] Refactor EPD driver from `samples/peripheral_esl/driver` or `hw` into `organized/tag/drivers/epd/`.
    *   [ ] Create Button driver in `organized/tag/drivers/button/`.
    *   [ ] Create `organized/tag/bsp/` module structure.
    *   [ ] Implement BSP for the target **tag** board (e.g., nRF52833DK) in `organized/tag/bsp/`.
*   [ ] **Application Logic (`app` - Tag Firmware):**
    *   [ ] Create `organized/tag/app/` module structure.
    *   [ ] Implement main application state machine skeleton in `organized/tag/app/`.
    *   [ ] Integrate module initializations (BSP, drivers, Bluetooth, ESL) in `organized/tag/app/`.
    *   [ ] Implement basic logic to handle ESL events and update display in `organized/tag/app/`.
*   [ ] **Integration & Configuration (Tag Firmware):**
    *   [ ] Create initial `organized/tag/config/prj.conf`.
    *   [ ] Create initial board overlay in `organized/tag/boards/`.
    *   [ ] Ensure all tag modules are correctly linked via `organized/tag/CMakeLists.txt`.
*   [ ] **Testing & Refinement (Tag Firmware):**
    *   [ ] Perform initial build and flash test for the **tag**.
    *   [ ] Test basic advertising and connection for the **tag**.
    *   [ ] Test ESL service discovery and basic command handling for the **tag**.
    *   [ ] Test EPD updates for the **tag**.
    *   [ ] Optimize and refactor the **tag** firmware as needed.
*   [ ] **Access Point Firmware:**
    *   [ ] Define requirements and structure for the access point (Future phase).

## 6. Done

*   [X] Created `organized/` directory.
*   [X] Created initial `PRD.md`.

---

*This PRD will be updated as the project progresses.* 