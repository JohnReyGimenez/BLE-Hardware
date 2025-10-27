# BLE Attendance Hardware

This repository contains the firmware and hardware details for the **ESP32 and nRF52840 prototypes** of a Bluetooth Low Energy (BLE) attendance system.

This project is designed to automate attendance by assigning students a wearable **BLE Tag** (beacon) and using a network of **Classroom Scanners** to detect their presence. The scanners log entry/exit events, timestamp them, and forward them to a central backend server.

---

## System Architecture

The system consists of three main parts:

1.  **BLE Tag (nRF52840):** A compact, battery-powered beacon that continuously broadcasts its unique ID.
2.  **Classroom Scanner (Dual ESP32):** A two-part node that receives BLE signals and sends them to the network.
    * **ESP32-A (Scanner):** Scans for BLE broadcasts, filters for known tags, and adds an accurate timestamp (via an RTC Module).
    * **ESP32-B (Wi-Fi):** Receives data from ESP32-A via **ESP-NOW** and POSTs it (as JSON) to the backend API.
3.  **Backend Server (Rails API):** (Not in this repo) A central server that receives the JSON data and records the attendance.

### Data Flow

`[nRF52840 Tag]` $\to$ `(BLE Broadcast)` $\to$ `[ESP32-A Scanner + RTC]` $\to$ `(ESP-NOW)` $\to$ `[ESP32-B Wi-Fi]` $\to$ `(HTTP POST)` $\to$ `[Rails API]`

---

## Hardware Bill of Materials

### 1. BLE Tag (Beacon)

| Component | Specification | Purpose |
| :--- | :--- | :--- |
| Microcontroller | **nRF52840 ProMicro Dev Board V2** | BLE Broadcasting |
| Battery | **3.7V LiPo (e.g., 501515 80mAh)** | Power (Must have JST-PH Female plug) |
| Battery Cable 1 | **JST-PH 2.0 Male** (with bare wires) | Connects to battery |
| Battery Cable 2 | **Male-to-Female Dupont Wires** (x2) | No-solder adapter |
| Enclosure | 3D Printed Keyfob Case | Portability & Protection |

### 2. Classroom Scanner

| Component | Specification | Purpose |
| :--- | :--- | :--- |
| Scanner Node | **ESP32 Dev Board** | BLE Scanning (ESP32-A) |
| Wi-Fi Node | **ESP32 Dev Board** | Wi-Fi & API Comms (ESP32-B) |
| Timekeeping | **DS3231 RTC Module** | Accurate, persistent timestamping |
| RTC Battery | **CR1220 or CR2032 Coin Cell** | RTC backup power |
| Status Light | **Common LEDs** (e.g., Red, Green) | Visual feedback (Scanning, Sent, etc.) |
| Enclosure | 3D Printed Case (w/ vents) | Housing for dual ESP32s |
| Power | 5V USB-C Power Supply (x2) | Main power for scanners |

---

## Assembly Notes

### BLE Tag: No-Solder Battery Connection

This prototype is designed for assembly **without a soldering iron or breadboard**.

1.  Plug the **JST-PH Male Plug** (Cable 1) into the **LiPo battery's** female connector.
2.  Take one **Male-to-Female Dupont Jumper Wire** (Cable 2). Firmly insert the **bare red wire** (from the JST cable) into the **female (socket) end** of the jumper.
3.  Repeat with a second jumper wire for the **bare black wire**.
4.  You will be left with two **male Dupont pins**.
5.  Plug the **red male pin** directly into the **`BATTERY+`** pin hole on the nRF52840 board.
6.  Plug the **black male pin** directly into the **`BATTERY-`** pin hole on the nRF52840 board.
7.  The connection is secured by the friction fit of the pins. The 3D-printed case will immobilize all components to prevent wires from pulling loose.

### 3D Enclosure Design

* **BLE Tag:** The case is designed to be fully sealed. The nRF52840 generates negligible heat and does not require ventilation.
* **Classroom Scanner:** The case **must include ventilation slots**. The dual ESP32s, especially the Wi-Fi node (ESP32-B), will generate heat. Vents are required for passive convection to ensure stability.

---

## Firmware

This repository includes the code for:

* **/ble_tag_nrf52840:** Firmware for the nRF52840. It initializes the chip and continuously advertises its BLE MAC address at a set interval.
* **/ble_scanner_esp32:** Firmware for the dual ESP32 scanner.
    * `esp32a_scanner.ino`: Handles BLE scanning, filtering, and event logic (`entered`/`exited`) using the RTC for timestamps. Sends data to ESP32-B via ESP-NOW.
    * `esp32b_wifi.ino`: Receives ESP-NOW data and forwards it as a JSON payload to the Rails API via an HTTP POST request.
