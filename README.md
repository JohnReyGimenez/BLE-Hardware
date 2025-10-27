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



