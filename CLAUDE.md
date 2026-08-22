# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

Additional guidance on the state and history of the project can be read at PROGRAMMER.md.

## Project purpose

A learning project, not a product. The goal is for the user to learn **I2C** on a Raspberry Pi Pico 1
(RP2040, no wireless) talking to an **MPU-6050** accel/gyro, while also learning the **bare CMake +
command-line toolchain** on Arch Linux with nvim (deliberately leaving behind the VS Code Pico
extension used in the previous project).

Planned progression — each stage is its own learning milestone, don't skip ahead:

1. Toolchain setup + hello world over **USB CDC** (no UART hardware available).
2. I2C bus scan (sweep 7-bit addresses, print an ACK table).
3. Read a single register: `WHO_AM_I` (MPU-6050 reg `0x75`, expected `0x68`).
4. Stream real-time accel/gyro data.
5. Something fun driven by the sensor — e.g. orientation-controlled RGB LED.

## Tutoring contract (important)

The user is here to *learn*, not to receive a finished program.

- **Do not write complete code blocks** for something the user hasn't attempted.
- Hints, pointers to datasheet sections, API names, and conceptual explanations are encouraged.
- Once the user has made their own attempt and explicitly asks "how would you have written this?",
  a fuller code block is fine.
- Explain the *why* at the register/protocol level (start/stop conditions, ACK/NACK, repeated start,
  pull-ups, register auto-increment), not just which SDK function to call.
- The user asked specifically to build up an understanding of **CMake** beyond "it's a build script" —
  when touching `CMakeLists.txt`, explain what each command actually does (target vs. directory scope,
  what `pico_sdk_init()` generates, why link libraries are per-target).

### User's background

Comp-sci / web dev. Strong C fundamentals — the previous project (`../RainbowPico/RainbowPico.c`) uses
enums, designated-initializer lookup tables, structs, precomputed gamma tables, PWM slice/channel
config, and manual button debouncing. Assume competence in software; **do not** assume electrical
engineering knowledge. Ohm's law and basic circuits are understood; things like pull-up resistor
sizing, open-drain buses, logic levels, and bus capacitance are worth explaining.

## Environment on this machine

Arch Linux, nvim, bash. Pico SDK **2.3.0** at `/home/james/repos/pico-sdk`, already exported as
`$PICO_SDK_PATH`. Installed: `arm-none-eabi-{gcc,binutils,gdb,newlib}`, `cmake`, `make`, `picotool`.

Known gaps as of project start (the user should resolve these as part of stage 1 — coach, don't
silently fix):

- **SDK submodules are not initialized** (`git submodule status` in the SDK shows all five unchecked;
  `lib/tinyusb` is empty). USB stdio will not build without TinyUSB.
- **No `ninja`** — use the Make generator, or install ninja.
- **No serial terminal** (`minicom`/`picocom`/`tio`/`screen` all absent). `cat /dev/ttyACM0` works for
  read-only output in a pinch.
- **No udev rules** in `/etc/udev/rules.d/`, and the user is not in `uucp`/`dialout`. Expect permission
  problems on `/dev/ttyACM0` and on `picotool` accessing the BOOTSEL USB device without sudo.

## Build / flash / monitor

Out-of-tree CMake build, standard Pico SDK layout:

```sh
cmake -B build -S .          # configure; add -G Ninja if ninja is installed
cmake --build build          # produces build/<target>.elf, .uf2, .bin
```

Flashing (no debug probe — BOOTSEL only):

```sh
picotool load -x build/<target>.uf2   # -x runs after loading; needs the Pico in BOOTSEL
# or: hold BOOTSEL while plugging in, then copy the .uf2 onto the RPI-RP2 mass-storage volume
```

Since stdio goes over USB CDC, the Pico re-enumerates as `/dev/ttyACM0` after boot. Reconnect the
terminal after every flash.

`CMAKE_EXPORT_COMPILE_COMMANDS ON` plus a `compile_commands.json` symlink at the repo root is what
makes clangd work in nvim — worth setting up early.

## Hardware notes

- Pico 1 / RP2040: two I2C controllers, `i2c0` and `i2c1`, each mappable to several GPIO pairs
  (SDA on GPIO where `pin % 4 == 0 or 1`; see the RP2040 datasheet pin-function table). The SDK's
  `i2c_default` is `i2c0` on GP4 (SDA) / GP5 (SCL).
- MPU-6050 breakout boards usually include their own pull-ups and a 3.3 V regulator; the `AD0` pin
  selects address `0x68` (low) or `0x69` (high).
- The Pico's internal pull-ups (`gpio_pull_up`) are weak (~50 kΩ) — fine at 100 kHz on a short bus,
  but a reason for flaky behavior at higher speeds.

## Reference material in sibling directories

- `../RainbowPico/` — the previous project. VS Code/Windows-configured; its `CMakeLists.txt` contains
  a VS Code extension block (`~/.pico-sdk/cmake/pico-vscode.cmake`) that should **not** be copied here.
  It does show the shape of a working Pico CMake file, and the `pico_enable_stdio_usb(<target> 1)` /
  `pico_enable_stdio_uart(<target> 0)` pair that this project needs.
- `../i2c/bus_scan.c` — a copy of the SDK's bus-scan example. Useful as a last-resort reference; prefer
  letting the user write their own scan first.
- `../pico-sdk-blink/` — a Wokwi-simulator-oriented blink project.
