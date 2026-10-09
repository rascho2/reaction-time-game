# Reaction Time Game Console 🏁

A two-player reaction time game inspired by **Formula 1 start lights**.
Five LEDs light up one after another, then all go out at once, and whoever hits their button first wins.
False starts are detected and disqualified, results are ranked and shown on a TFT display together with a leaderboard.

Built as a team project for the embedded systems course (ESHO1) at **TU Darmstadt**.


https://github.com/user-attachments/assets/cb83d2c6-36fd-4885-ba2b-9fdcee5f53a8


<img width="1200" height="1600" alt="WhatsApp Image 2026-10-09 at 23 41 44" src="https://github.com/user-attachments/assets/216a8861-f7b8-47f1-b41d-9add579dc3eb" />

---

## ✨ Features

- F1-style start sequence: 5 LEDs light up one by one, then switch off together as the start signal
- Two players with dedicated arcade-style buttons
- **False start detection**: pressing before the lights go out disqualifies the player
- Reaction times measured and ranked by the STM32
- TFT display with game states, 3-letter player name entry and a **top-5 leaderboard**
- Custom **3D-printed housing**

---

## 🏗️ Architecture

The system is split across two boards that talk over UART:

```
┌────────────────────────┐   UART (9600 baud)   ┌──────────────────────┐
│ STM32F030 (ESHO1 board)│ ───── "L1".."L5" ───▶│ Raspberry Pi         │
│                        │ ───── "l1".."l5" ───▶│                      │
│ • game logic & timing  │                      │ • drives 5 LEDs      │
│ • false start check    │ ◀──── "B1"/"B2" ──── │ • reads 2 buttons    │
│ • ranking/leaderboard  │                      │   (GPIO)             │
│ • TFT display (SPI)    │                      │                      │
└────────────────────────┘                      └──────────────────────┘
```

**Why this split?** Most free STM32 pins on the extension board were already occupied by on-board peripherals (I2C, SPI, SWD, UART). We therefore kept the game logic and timing on the STM32 and used the Raspberry Pi's GPIO header as the I/O interface for the LEDs and buttons.

### UART protocol

| Direction  | Command        | Meaning                    |
|------------|----------------|----------------------------|
| STM32 → Pi | `L1` … `L5`    | turn LED 1–5 on            |
| STM32 → Pi | `l1` … `l5`    | turn LED 1–5 off           |
| Pi → STM32 | `B1` / `B2`    | player 1 / player 2 pressed |

---

## 🔧 Hardware

- **STM32F030CCT6** on the TU Darmstadt *ESHO1 Extension Board*
- **Raspberry Pi** <!-- TODO: Modell eintragen -->
- TFT display (SPI) mounted on the extension board
- 5 LEDs with series resistors, driven directly from Raspberry Pi GPIOs (on/off only, no PWM needed)
- 2 × RJT-12-27-067 clicky switches (soldered connections)
- 3D-printed housing (STL files in [`cad/`](cad/))

### Pin mapping

| Component | GPIO (BCM) | Physical pin |
|-----------|------------|--------------|
| LED 1     | GPIO17     | 11           |
| LED 2     | GPIO18     | 12           |
| LED 3     | GPIO22     | 15           |
| LED 4     | GPIO23     | 16           |
| LED 5     | GPIO24     | 18           |
| Button 1  | GPIO25     | 22           |
| Button 2  | GPIO27     | 13           |

| TFT signal | STM32 pin |
|------------|-----------|
| CS         | PB11      |
| RST        | PB10      |
| DC         | PB0       |
| Backlight  | PB1       |

---

## 📁 Repository structure

```
reaction-time-game/
├── stm32/      # Game logic, display driver, Makefile (C, STM32 HAL)
├── pi/         # reaction_game.py – LED and button I/O on the Raspberry Pi
├── cad/        # STL files for the 3D-printed housing
└── docs/       # Photos and videos
```

---

## 🚀 Build & run

### STM32

Requirements: `arm-none-eabi-gcc` toolchain and the [STM32CubeF0](https://github.com/STMicroelectronics/STM32CubeF0) package (CMSIS + HAL).

```bash
git clone --recursive https://github.com/STMicroelectronics/STM32CubeF0.git ~/STM32CubeF0
cd stm32
make STM32CUBE=~/STM32CubeF0
```

Then flash the resulting firmware to the board via SWD.

### Raspberry Pi

Requirements: Python 3 with `pyserial` and `RPi.GPIO`, and the serial port enabled via `raspi-config`.

```bash
sudo apt install python3-serial python3-rpi.gpio
cd pi
python3 reaction_game.py
```

---

## 🧩 Challenges & lessons learned

- **Unreliable LEDs:** Several LEDs did not light up. Systematic testing traced it back to individual faulty or loose-contact resistors, not to the code. Swapping them fixed the issue.
- **Wiring the Pi header:** Accessing the Pi's GPIOs through the extension board's small solder points with crocodile clips was too unreliable. After consulting our tutor, we removed the extension board and wired the 40-pin header directly with jumper cables.
- **Switch connections:** The switches don't fit a breadboard and crocodile clips kept slipping off, so the connections were soldered.
- **Housing iteration:** The first version of the top housing didn't fit and was redesigned.
- **Pin constraints:** Limited free pins on the extension board led to the STM32 + Pi split architecture described above.

---

## 👥 Team

| Member | Contribution |
|--------|--------------|
| Rashid Brown ([@rascho2](https://github.com/rascho2)) | TODO: dein Teil, z. B. LED sequencing, Pi I/O, wiring & soldering, integration |
| Team member | Display & UI |
| Team member | 3D-printed housing |
