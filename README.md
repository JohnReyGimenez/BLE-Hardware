# BLE Attendance Hardware

This repository contains the firmware and hardware details for the **ESP32 and nRF52840 prototypes** of a Bluetooth Low Energy (BLE) attendance system.

This project is designed to automate attendance by assigning students a wearable **BLE Tag** (beacon) and using a network of **Classroom Scanners** to detect their presence. The scanners log entry/exit events, timestamp them, and forward them to a central backend server.

---

## System Architecture

The system consists of three main parts:

1.  **BLE Tag (nRF52840):** Beacon that continuously broadcasts its unique ID.
2.  **Classroom Scanner (Dual ESP32):** A two-part node that receives BLE signals and sends them to the network.
    - **ESP32-A (Scanner):** Scans for BLE broadcasts, filters for known tags, and adds an accurate timestamp (via an RTC Module).
    - **ESP32-B (Wi-Fi):** Receives data from ESP32-A via **ESP-NOW** and POSTs it (as JSON) to the backend API.
3.  **[Backend Server (Rails API)](https://github.com/JohnReyGimenez/BLE-Attendance-API):** A central server that receives the JSON data and records the attendance.

### Data Flow

`[nRF52840 Tag]` $\to$ `(BLE Broadcast)` $\to$ `[ESP32-A Scanner + RTC]` $\to$ `(ESP-NOW)` $\to$ `[ESP32-B Wi-Fi]` $\to$ `(HTTP POST)` $\to$ `[Rails API]`

---

## Hardware Bill of Materials

### 1. BLE Tag (Beacon)

| Component | Specification |
| :--- | :--- |
| Microcontroller | **nRF52840 ProMicro Dev Board V2** |
| Battery | **3.7V LiPo (501515 80mAh)** |
| Enclosure | 3D Printed Keyfob Case |

### Design
<img width="875" height="401" alt="image" src="https://github.com/user-attachments/assets/e399555c-2fc0-4e56-beba-e35573753c99" />

### 2. Classroom Scanner

| Component | Specification |
| :--- | :--- |
| Scanner Node | **ESP32 Dev Board** |
| Wi-Fi Node | **ESP32 Dev Board** |
| Timekeeping | **DS3231 RTC Module** |
| Enclosure | 3D Printed Case  |

### Design
<img width="611" height="367" alt="image" src="https://github.com/user-attachments/assets/73bc9779-63db-4932-adc3-7b943a1b53f1" />

---

## Firmware

This is a [PlatformIO](https://platformio.org/) project. All firmware source code is located in the `src/` directory.

- **`platformio.ini`**: The project configuration file. It defines the boards (nRF52840, ESP32), framework (Arduino), and all required libraries
  
- **`src/ble_tag_nrf52840.cpp` (BLE Tag):**
    - Firmware for the nRF52840 beacon.
    - It initializes the chip as a BLE server and periodically broadcasts an **iBeacon frame**.
    - This specific frame format allows scanners to efficiently identify the tag.

- **`src/esp32a_scanner.cpp` (Scanner Node):**
    - Firmware for the ESP32-A, which handles all real-time sensing.
    - Continuously scans for BLE advertisements.
    - Filters for the specific iBeacons broadcast by the tags.
    - Queries the DS3231 RTC for an accurate timestamp upon detection.
    - Sends the event data (tag ID, timestamp) to ESP32-B via ESP-NOW.

- **`src/esp32b_wifi.cpp` (Wi-Fi Node):**
    - Firmware for the ESP32-B, which handles all network communication.
    - Receives data packets from ESP32-A via ESP-NOW.
    - Connects to the local Wi-Fi network.
    - Formats the data as a JSON payload and sends it to the Rails API via an HTTP POST request.











