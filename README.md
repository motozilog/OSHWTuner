# OSHWTuner

**OSHW Tuner** is an **Open Source Bluetooth & USB music sheet tuner ** designed for musicians and performers. It supports both Bluetooth HID Keyboard mode and USB HID Device mode, making it compatible with a wide range of devices and platforms.

---

## Features

- **Bluetooth HID Keyboard Mode**  
  Turn pages seamlessly in **Piascore (iOS)** and other music sheet apps such as **有谱么 (Android)**.

- **USB HID Device Mode**  
  Works on **Windows 7 to Windows 11** as a plug‑and‑play HID device.

- **Hardware-Based Configuration**  
  Uses a physical **BCD (421 coding) switch** to set each foot pedal's behavior.  
  Provides **8 preset profiles** that can be switched instantly via the hardware encoder — no accidental mode changes during performance.

- **Dual Physical Switches**  
  - **Profile switch**: BCD encoder for reliable, accident‑free input mode changes (Left/Right, Up/Down, PageUP/PageDown, Space/Enter).  
  - **Power switch**: Hard‑wired on/off — eliminates standby power drain and gives you true peace of mind.

- **Always-On Mode**  
  Designed for pre‑performance setup — when you need to check and test equipment well in advance (e.g., 30+ minutes before a show).  
  In **Always-On Mode**, the device never enters sleep, ensuring no unexpected disconnections or re‑pairing issues during sound checks or rehearsals.  
  For daily use, the default **power‑saving mode** automatically puts the MCU into sleep after **30 minutes of inactivity** — saving battery even if you forget to turn off the power.

- **Enhanced Mechanical Design**  
  - Anti‑slip pads on the bottom.  
  - Reserved space for **counterweights** to reduce unwanted movement or slipping during live use.

- **Customizable Key Mapping**  
  Unlike most page turners, OSHWTuner allows you to change the key values assigned to the left and right pedals.  
  A companion **Windows 10+ Python script** is provided for easy profile customization.  
  > **Note:** The configuration tool works **only via USB connection** (not over Bluetooth).

---

## Hardware Open Source

The hardware design (schematics, PCB, and mechanical files) is fully open source and available at:  
🔗 **[OSHWHub - OSHWTuner Hardware Project](https://oshwhub.com/motozilog/project_dgkawstp)**

---

## Development Environment

- **MCU Firmware**  
  - Developed with **MounRiver Studio V2.4.0**  
  - Target MCU: **CH592**  
  - Open the project in: `\CH592\BLE\HID_Keyboard`

- **PC Configuration Tool**  
  - **Python 3.12.2**  
  - Dependency: `hidapi == 0.15.0`  
  - Script location: `\上位机\oshw-tuner-v2.2.py`  
  - **Important:** The tool communicates with the device over **USB HID**, not Bluetooth.

---

## Getting Started

### 1. Build the Firmware
- Launch MounRiver Studio V2.4.0.
- Open the project from `\CH592\BLE\HID_Keyboard`.
- Build and flash to your CH592 device.

### 2. Set Up the Python Configuration Tool
- Install Python 3.12.2.
- Install the required library: `hidapi == 0.15.0`
- **Connect the device via USB** to your computer.
- Run the script `oshw-tuner-v2.2.py` located in the `\上位机\` folder.
- Use the GUI to customize pedal key mappings and save profiles.

### 3. Hardware Usage
- Select one of **8 profiles** using the BCD switch.
- Connect via Bluetooth (HID Keyboard) or USB (HID Device).
- Start turning pages with your feet — reliably and accurately.

---

## Compatibility

| Mode          | Supported Platforms                          |
|---------------|----------------------------------------------|
| Bluetooth HID | iOS (Piascore), Android (YouPuMe, etc.), Windows 7 to 11 (Windows 8 not tested) |
| USB HID       | Windows 7 to 11 (Windows 8 not tested)       |

---

## Why OSHWTuner?

- No more accidental mode switches — physical BCD encoding ensures you stay in control.
- True hardware power switch — no standby current, no hidden battery drain.
- Rock‑solid stability on stage — anti‑slip pads and optional counterweights keep the tuner in place.
- Fully customizable key mappings — adapt the pedals to your preferred workflow.
- Flexible power management — choose Always‑On for stress‑free pre‑show setup, or power‑saving mode for worry‑free daily use.

---

## License

This project is licensed under **CC BY-NC-SA 4.0**.

---

*This README was generated and translated with the assistance of DeepSeek.*