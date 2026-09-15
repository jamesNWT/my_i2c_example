# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## PROGRAMMER.md

`PROGRAMMER.md` is the user's own development journal and doubles as a relatively up-to-date,
relatively accurate long-term memory of the project: what has been built, what was learned, why
decisions were made, the current hardware wiring, and the current goal. **Read it whenever you need
history or context that isn't in the code.**

**Never write to `PROGRAMMER.md`.** It is authored by the user only. Treat it as read-only, even when
asked to "update the docs" — offer to update `CLAUDE.md` or `README.md` instead. It may also lag
reality slightly; when it conflicts with what you can observe on disk, trust the observation and say so.

## Project purpose

A learning project, not a product. The goal is for the user to learn **I2C** on a Raspberry Pi Pico 1
(RP2040, no wireless) talking to an **MPU-6050** accel/gyro, while also learning the **bare CMake +
command-line toolchain** on Arch Linux with nvim (deliberately leaving behind the VS Code Pico
extension used in the previous project).

Planned progression — each stage is its own learning milestone, don't skip ahead:

1. ~~Toolchain setup + hello world over **USB CDC** (no UART hardware available).~~ **Done.**
2. **← current:** I2C bus scan (sweep 7-bit addresses, print an ACK table).
3. Read a single register: `WHO_AM_I` (MPU-6050 reg `0x75`, expected `0x68`).
4. Stream real-time accel/gyro data.
5. Something fun driven by the sensor — e.g. orientation-controlled RGB LED.

Note on stage 2: the user flashed a *prebuilt* SDK bus-scan example early on, while validating `tio`,
and it reported the MPU-6050 at `0x68`. So the hardware and wiring are known-good — but the user has
**not** yet written their own scan. That is the actual milestone; don't treat it as already achieved.

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

Arch Linux, nvim, bash.

- Pico SDK **2.3.0** at `/home/james/repos/oss-from-source/pico-sdk`, exported as `$PICO_SDK_PATH`
  from `~/.bashrc`. (It used to live at `~/repos/pico-sdk`; a shell started before that move may still
  carry the old value — check `echo $PICO_SDK_PATH` against the path above if a configure fails.)
- SDK submodules: `lib/tinyusb` (0.18.0) is initialized — that's the one USB stdio needs. `btstack`,
  `cyw43-driver`, `lwip`, and `mbedtls` are still unchecked, which is fine for a wireless-free Pico 1.
- Installed: `arm-none-eabi-{gcc,binutils,gdb,newlib}`, `cmake`, `make`, `ninja`, `clang-format`,
  `fd`, `rg`, `tio`.
- `picotool` is **built from source** at `/usr/local/bin/picotool`, not the AUR package — the AUR
  release had a `picotool info` bug fixed only on master. Its install step needs sudo.
- udev rules are in place: `/etc/udev/rules.d/60-picotool.rules` (copied from the picotool repo) and
  `/etc/udev/rules.d/70-pico.rules` (tracked in this repo at `udev/70-pico.rules`). Both tag the
  device `uaccess`, so the locally-seated user gets ACL permission without sudo and without being in
  `uucp`/`dialout` — the user is in `users docker wheel` only, and that's intentional.
- `70-pico.rules` also creates the `/dev/pico` symlink to whichever `/dev/ttyACM*` the Pico enumerates
  as. Always use `/dev/pico`, not a hardcoded `ttyACM0`.

Useful state checks:

```sh
lsusb | rg 2e8a      # 2e8a:0003 = BOOTSEL/boot mode; 2e8a:000a = running firmware
ls -l /dev/pico      # symlink present, permissions end in "+" (an ACL is set)
```

## Build / flash / monitor

Out-of-tree CMake build. Note the layout: **`CMakeLists.txt` lives in `source/`, not the repo root**,
and the build tree is `build/` at the repo root.

```sh
cmake -S source -B build     # configure (existing build/ was configured with Unix Makefiles)
cmake --build build          # produces build/i2c.elf, build/i2c.uf2, build/i2c.bin
```

`.nvim.lua` sets `makeprg` to `cmake --build build`, so `:make` rebuilds from inside nvim.

Flashing (no debug probe — BOOTSEL only):

```sh
picotool load -x build/i2c.uf2   # -x runs after loading; needs the Pico in BOOTSEL
# or: hold BOOTSEL while plugging in, then copy the .uf2 onto the RPI-RP2 mass-storage volume
```

Monitoring — stdio goes over USB CDC, so the Pico re-enumerates after every flash:

```sh
tio /dev/pico    # exit with ctrl+t then q (can take a moment)
```

`tio` survives disconnects and re-flashes on its own, so the usual workflow is to leave a `tio` pane
open permanently and just rebuild/reflash in another pane.

Formatting: `fd -e c -e h -x clang-format -i` from the repo root formats everything. Style is in
`.clang-format` (LLVM base, 2-space indent, 80 columns).

### Toolchain details worth not "fixing"

- `set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)` in `source/CMakeLists.txt` is deliberate. It
  makes CMake's compiler-check try-compile skip the link step, which would otherwise fail: newlib's
  portability layer is only implemented once `pico_stdlib` is linked, and the probe program has no
  access to it.
- clangd is wired up via `.clangd` (`CompileDatabase: build`), which points clangd at the build tree
  directly. This superseded the earlier `compile_commands.json` symlink at the repo root — the
  symlink is gitignored and no longer the mechanism.

## Hardware notes

Current wiring (also drawn in `PROGRAMMER.md`):

| Pico phys pin | Signal | MPU-6050 |
|---|---|---|
| 6  | GP4 / SDA | SDA |
| 7  | GP5 / SCL | SCL |
| 36 | 3V3(OUT)  | VCC |
| 38 | GND       | GND |

- Pico 1 / RP2040: two I2C controllers, `i2c0` and `i2c1`, each mappable to several GPIO pairs
  (SDA on GPIO where `pin % 4 == 0 or 1`; see the RP2040 datasheet pin-function table). GP4/GP5 above
  is the SDK's `i2c_default` — `i2c0`.
- MPU-6050 breakout boards usually include their own pull-ups and a 3.3 V regulator; the `AD0` pin
  selects address `0x68` (low) or `0x69` (high). This board answers at **`0x68`**, confirmed by a scan.
- The Pico's internal pull-ups (`gpio_pull_up`) are weak (~50 kΩ) — fine at 100 kHz on a short bus,
  but a reason for flaky behavior at higher speeds.
- `stdio_usb_connected()` returns true only once the host asserts DTR, which in practice means "a
  serial monitor is attached" (`tio` counts). Waiting on it before the first `printf` avoids losing
  early output.

## Reference material in sibling directories

- `../RainbowPico/` — the previous project. VS Code/Windows-configured; its `CMakeLists.txt` contains
  a VS Code extension block (`~/.pico-sdk/cmake/pico-vscode.cmake`) that should **not** be copied here.
  It does show the shape of a working Pico CMake file, and the `pico_enable_stdio_usb(<target> 1)` /
  `pico_enable_stdio_uart(<target> 0)` pair that this project uses.
- `../i2c/bus_scan.c` — a copy of the SDK's bus-scan example. Useful as a last-resort reference; prefer
  letting the user write their own scan first.
- `../pico-sdk-blink/` — a Wokwi-simulator-oriented blink project.

## Datasheets

- **MPU-6000/6050 Register Map** (InvenSense doc RM-MPU-6000A) — `~/Documents/RM-MPU-6000A.pdf`.
  The authoritative source for register addresses, bit fields, reset values, and scaling (e.g.
  `WHO_AM_I` = `0x75`, `PWR_MGMT_1` = `0x6B`, accel/gyro data registers from `0x3B`). Point the user at
  specific sections/pages of it rather than quoting values from memory, and check it before stating
  any register detail.
