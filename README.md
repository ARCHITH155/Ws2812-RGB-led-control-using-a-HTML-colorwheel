
# 🌈 ESP32 Web-Controlled LED Strip (FastLED)

A robust, web-based controller for WS2812B (NeoPixel) LED strips using an ESP32. This project hosts a modern, dark-mode web interface directly on the ESP32, allowing you to control brightness, effects, and specific LED zones wirelessly from any smartphone or computer.

## ✨ Features

* **Dark Mode UI:** A sleek, responsive web interface stored in program memory (PROGMEM).
* **Zone Control:** Independently color 4 distinct segments (0-9, 10-19, 20-29, 30-39) or the entire strip.
* **Live Effects:** Switch between **Solid**, **Rainbow**, and **Raindrop** animations.
* **Brightness Slider:** Real-time global brightness control (safe-guarded for USB power).
* **Non-Blocking:** Animations run smoothly without freezing the web server using non-blocking delays.
* **Instant Feedback:** Visual feedback on the web UI when modes change.

## 🛠️ Hardware Required

* **ESP32 Development Board** (NodeMCU, ESP-WROOM-32, etc.)
* **WS2812B LED Strip** (NeoPixel) - *Code configured for 40 LEDs*
* **Connecting Wires** (Jumper wires)
* **Micro-USB Cable** (for power & programming)
* *(Optional)* 5V External Power Supply (Recommended for >60 LEDs)

## 🔌 Wiring Diagram

**⚠️ CAUTION:** Do NOT connect the 5V LED strip to the 3.3V pin.

| Component | Pin | Connection on ESP32 |
| :--- | :--- | :--- |
| **LED Strip Red** | **5V / VCC** | **VIN** (if using USB power) or External 5V PSU |
| **LED Strip White** | **GND** | **GND** |
| **LED Strip Green** | **DIN / Data** | **GPIO 5** (D5) |

> **Note on Power:** If powering via USB, the code is set to a safe brightness limit (150/255) to prevent brownouts. If using an external 5V 10A power supply, you can increase `MAX_SAFE_BRIGHTNESS` in the code.

## 💾 Software Prerequisites

You need the **Arduino IDE** to flash this code.

1.  **Install ESP32 Board Manager:**
    * *File > Preferences > Additional Boards Manager URLs:* `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
    * *Tools > Board > Boards Manager*, search for **esp32** and install.

2.  **Install Libraries:**
    * Go to *Sketch > Include Library > Manage Libraries...*
    * Search for and install: **FastLED** by Daniel Garcia.

## ⚙️ Configuration

Open the `.ino` file and modify the following lines to match your setup:

```cpp
// 1. WiFi Credentials
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// 2. LED Settings
#define LED_PIN     5      // The GPIO pin connected to Data In
#define LED_COUNT   40     // Total number of LEDs
