# My PICO / I2C Example

Manually configured toolchain and doing cool things with a PICO using I2C.

A dev journal exists at `PROGRAMMER.md` and commits containing LLM-generated text are tagged with claude as co-author.

## Setup

### Requirements:

**Installations**
- pico-sdk, and its requirements
- picotool, and its requirements
  - If building from source, follow its BUILDING.md guide, including the installation step
- tio

**Operations**
- set $PICO_SDK_PATH environment variable
- init the tinyusb submodule in pico-sdk
- Setup udev rules so that tio can read pico's usb serial. Copy `udev/70-pico.rules` to `/etc/udev/rules.d/`.

**Verification**
- `lsusb | grep 2e8a` should show:
  - `Bus XXX Device XXX: ID 2e8a:0003 Raspberry Pi RP2 Boot` when the device is plugged in in BOOTSEL mode
  - `Bus XXX Device XXX: ID 2e8a:000a Raspberry Pi Pico` when the device is plugged in and running some code.
- `ls -l /dev/pico` should shouw a fully permissioned symlink to a file called something like `ttyACM1` or `ttyACM0`, etc. 

### Compiling the firmware

From the repo root, run `mkdir build` `cmake -S source -B build` `cmake --build build`

### Flashing the pico

`pico load -x /path/to/binary.uf2` when in BOOTSEL mode.

TODO: When I have the energy and the project is more complete improve this readme.
