<div align="center">

# 🔐 VLock+ - Personal Smart Door Lock System v3.1
<img src="https://img.shields.io/badge/Status-Working-brightgreen?style=for-the-badge&logo=arduino" alt="Status Badge" />
<img src="https://img.shields.io/badge/Built%20with-Arduino-blue?style=for-the-badge&logo=arduino" alt="Arduino Badge" />
<img src="https://img.shields.io/badge/Language-C%23-orange?style=for-the-badge" alt="License Badge" />

</div>

## 📌 Overview

**VLock+** is a **smart door lock system** that allows secure access using:
- Keypad passcode entry with secret admin features.
- Manual override buttons.
- Door handle button for unlocking.
- An LCD for visual interface and interaction.
- Safety features including manual reset and retry mechanism if door fails to close.
- **Permanent Passcode Storage** — passcode is saved in EEPROM and persists even after power loss.
- **Self-initializing EEPROM** — first boot automatically stores default passcode if EEPROM is blank.

If EEPROM is blank on first boot, a default passcode is stored automatically.

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

## 🛠 Architecture
- **Keypad** → User input
- **LCD** → System messages
- **EEPROM** → Stores passcode after power loss
- **Relay** → Controls door lock

---

## 📋 Requirements

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
| **EEPROM**           | Non-volatile memory to store passcode (inbuild)          |

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

---

## ⚙️ How It Works
1. **Startup**
   - Checks EEPROM for a saved passcode.
   - If blank → saves default `"123456"`.
   - Displays welcome message.
2. **Passcode Entry**
   - If correct → Unlock relay.
   - If wrong → Access denied.
3. **Change Passcode**
   - Enter `#2002#` to start change mode.
   - Enter old → new → confirm.
   - Saves to EEPROM.
4. **Power Safety**
   - Passcode persists even after power loss.

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

## 📷 Working Images

### Serial Monitor
<img width="657" height="515" alt="image" src="https://github.com/user-attachments/assets/42e573d6-d667-417f-a01f-a5c46a4a926d" />
<br>

### LCD Displays
<img width="331" height="147" alt="image" src="https://github.com/user-attachments/assets/2057f47b-4949-4924-ba5b-6bf37e9e6538" />
<img width="309" height="113" alt="image" src="https://github.com/user-attachments/assets/2787fb26-d837-419a-ac9d-cdb4612df69e" />
<img width="311" height="114" alt="image" src="https://github.com/user-attachments/assets/28466028-4e4c-4d49-bf88-afce0c05aff3" />
<img width="318" height="119" alt="image" src="https://github.com/user-attachments/assets/9928c6e1-6352-4b62-bfd9-eb7e163bf3e8" />
<img width="318" height="116" alt="image" src="https://github.com/user-attachments/assets/86b53a91-1ffd-4193-8d07-903c1028b520" />
<img width="316" height="119" alt="image" src="https://github.com/user-attachments/assets/6ddb449e-ab90-472b-a764-2a3e571db2a8" />
<img width="296" height="111" alt="image" src="https://github.com/user-attachments/assets/19725480-3ad1-43c9-b99f-24c99c04da27" />
<img width="300" height="108" alt="image" src="https://github.com/user-attachments/assets/34c10586-cbc0-48df-b207-2f514fdbadab" />
<img width="268" height="110" alt="image" src="https://github.com/user-attachments/assets/830ec58c-5cf8-4e34-81c3-2aab0920509d" />


---

## 🔍 Notes
- Use capacitors to prevent resets during relay switching.
- Avoid excessive EEPROM writes (limited cycles).
- Secure your change passcode.
- Add Capacitors on the Solenoid right in the starting point.
- Add Diodes for Safety

---

> _“If the mind can create, the hands can translate.”_ – Hashtag
