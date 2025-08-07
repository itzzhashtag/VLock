<div align="center">

# 🔐 VLock+ - Personal Smart Door Lock System

**by [Aniket Chowdhury](mailto:micro.aniket@gmail.com) (aka `#Hashtag`)**

<img src="https://img.shields.io/badge/Status-Working-brightgreen?style=for-the-badge&logo=arduino" alt="Status Badge" />
<img src="https://img.shields.io/badge/Built%20with-Arduino-blue?style=for-the-badge&logo=arduino" alt="Arduino Badge" />
<img src="https://img.shields.io/badge/License-Personal--Use-orange?style=for-the-badge" alt="License Badge" />

</div>

---

Welcome to **VLock+**, a personal home automation project designed, developed, and deployed by **Aniket Chowdhury (Hashtag)**.  
This project automates door locking/unlocking using a servo-controlled mechanism with multiple control options, including buttons and a remote keypad system, and includes advanced features like passcode entry, LCD display, and fault safety handling.

---

## 📌 Overview

**VLock+** is a **smart door lock system** that allows secure access using:
- Keypad passcode entry with secret admin features.
- Manual override buttons.
- Door handle button for unlocking.
- An LCD for visual interface and interaction.
- Safety features including manual reset and retry mechanism if door fails to close.
- **Permanent Passcode Storage** — passcode is saved in EEPROM and persists even after power loss.
- **Self-initializing EEPROM** — first boot automatically stores default passcode if EEPROM is blank.

The project is installed in **my personal room** and is built with **modular expansion in mind**, with a secondary Nano board handling keypad input over serial.

---

## 💡 Features

- 🔒 **Secure Lock/Unlock using 6-digit Passcode**
- ♻ **Passcode Change Mode** with Secret Admin Code
- 💾 **EEPROM Passcode Storage** — saves any changed passcode permanently until updated again
- 🛡 **Automatic EEPROM Initialization** with default passcode on first boot
- 💡 **Backlight Toggle** on LCD via Keypad
- 🆘 **Manual Reset Button** (hidden for safety)
- 🎛️ **Multi-Button Unlock** from different positions
- 🚪 **Servo-controlled Door Mechanism**
- 🔁 **Auto Retry** if door fails to close properly
- 🔄 **Soft Restart** functionality via keypad
- 🔊 **Buzzer Feedback** for all actions and alerts
- 🔐 **Relay Trigger (Solenoid) Control** for enhanced locking
- 🧠 **Smart UI Timeout and Override Logic**

---

## 🧰 Hardware Components

| Component           | Description                                      |
|---------------------|--------------------------------------------------|
| **Arduino UNO**    | Main controller board                            |
| **Arduino Nano**    | Secondary board (keypad input via Serial)        |
| **20x4 I2C LCD**     | Display for passcode UI and system feedback     |
| **Servo Motor**      | Controls door lock mechanism                    |
| **Buzzer**           | Audio feedback                                  |
| **Relay Module**     | Controls solenoid or magnetic lock              |
| **Keypad (4x4)**     | External keypad for passcode entry              |
| **Buttons**          | Multiple buttons for manual unlock              |
| **Hinge Switch (BTN)**     | Safety sensor to confirm door closure           |
| **LEDs (Red/Green/Blue)** | Status indicators                         |
| **EEPROM**           | Non-volatile memory to store passcode           |

---

## 🔑 Default Credentials (Change Accordingly)

- **Main Passcode**: `123456` (stored in EEPROM after first boot)
- **Change Passcode Code**: `#1234#`

---

## 🛠️ Code Structure

- `setup()`: Initializes all hardware components, loads passcode from EEPROM (or sets default on first boot), and starts UI.
- `loop()`: Main execution loop to listen for keypad input and button presses.
- `keypad()`: Handles all key inputs and UI logic.
- `unlocking()`: Validates passcode and handles passcode change flow.
- `door_open()` / `door_close()`: Controls servo and relay for door operations.
- `CheckDoorSafety()`: Ensures door is physically closed and retries if not.
- `keygen()`: Receives keypad input via SoftwareSerial from Nano.
- `debounceButton()`: Safe button press handler with long press detection.
- `homescreen()` / `startup()`: Display UI management.
- `warningBuzz()` / `RestartBuzz()`: Buzzer alerts for feedback.
- `loadOrInitPasscode()`: Loads saved passcode from EEPROM, or initializes it with default on first boot.
- `savePasscodeToEEPROM()`: Updates stored passcode without overwriting unchanged data.

---

## 🎮 Keypad Controls

| Key | Action                            |
|-----|------------------------------------|
| A   | Lock the door                      |
| B   | Unlock (asks for passcode)         |
| C   | Toggle LCD backlight               |
| D   | Restart system (only when open)    |
| #1234# | Initiate passcode change mode  |

---

## ⚠️ Safety & Fail-safes

- **EEPROM corruption prevention** — passcode updates use `EEPROM.update()` to avoid unnecessary wear.
- **Manual Reset Button** is hidden behind door panel.
- **Retry Mechanism**: If the hinge switch doesn’t confirm door closure, the system retries up to 3 times.
- **Timeout**: UI auto-closes passcode input after 60 seconds.
- **Override**: Unlock button can bypass passcode in emergencies.

---

## 🚀 Getting Started

1. Upload this code to **Arduino UNO**.
2. Upload [Nano Keypad Code](link-to-nano-code) to your **Nano board**.
3. Wire the Nano to UNO via SoftwareSerial (Nano TX → UNO RX D2).
4. Connect components as per the pin definitions in the code.
5. On **first boot**, the default passcode will be automatically saved to EEPROM.
6. Power up and observe startup melody + door auto-unlock on boot.
7. Use keypad to lock/unlock and test passcode change — new codes are stored permanently in EEPROM.

---

## 👤 Author & Contact

👨 **Name:** Aniket Chowdhury (aka Hashtag)  
📧 **Email:** [micro.aniket@gmail.com](mailto:micro.aniket@gmail.com)  
💼 **LinkedIn:** [itzz-hashtag](https://www.linkedin.com/in/itzz-hashtag/)  
🐙 **GitHub:** [itzzhashtag](https://github.com/itzzhashtag)  
📸 **Instagram:** [@itzz_hashtag](https://instagram.com/itzz_hashtag)

---

## 📜 License

This project is released under a Modified MIT License.
It is intended for personal and non-commercial use only.

🚫 Commercial use or distribution for profit is not permitted without prior written permission.
🤝 For collaboration, reuse, or licensing inquiries, please contact the author.

📄 View Full License <br>
[![License: MIT–NC](https://img.shields.io/badge/license-MIT--NC-blue.svg)](./LICENSE)

---

## ❤️ Acknowledgements

This is a solo passion project, built with countless nights of tinkering, testing, and debugging.  
If you find it useful or inspiring, feel free to ⭐ the repository or connect with me on social media!

---

> _“If the mind can create, the hands can translate.”_ – Hashtag
