# PROGRAMMER.md

A journal of development in this repo, and long-term memory accessible to agents, but LLMs do not modify this file.

## What we're building and why

To introduce myself to firmware development, I'm working on writing programs for the raspberry pi PICO H 1. This project in particular is for learning how to use I2C. We shall build up the program from first principles, to eventually create a device that can do something interesting with sensor data from the MPU-6050

### Current hardware list

- RaspberryPi PICO-H 1

- MPU-6050

### Current circuit diagram
```ASCII

 ┌─────────────────────────┐
 │PICO                     │
 │                         │
 │phys:  6    7    36   38 │
 │Name: SDA  SCL   3V3  GND│
 │GPIO: GP4  GP5    │    │ │
 └───────┬────┬─────┼────┼─┘
         │    │     │    │  
         │    │     │    │  
         │    │     │    │  
      ┌──┼────┼─────┼────┼─┐
      │ SDA  SCL   VCC  GND│
      │                    │
      │      MPU-6050      │
      └────────────────────┘
```
## Structure / Architecture

 > This is highly TBD, we'll see what becomes necessary as the complexity grows. However for now, I'll base it off what worked for my RainbowPico project

A basic one-file C program ought to be sufficient for now. However, this C program should follow a basic structure:

### Hardware configuraion definitions

This section should define constants, enums, or structs that are dependent on the actual physical hardware and how I have it wired up.

Could also have functions related to how the hardware interacts with the real world, like gamma correction for RGB LEDs.

### Component Initialization 

Define the functions that will be used for initializing components. If it has init_ in its name, it should probably go here.

### Component Control

Define functions used to read signals from components, or send signals to components.

### Main Program

The actual thing I want the device to do. It may be a good idea to wait for `stdio_usb_connected()` to return true before we print anything. This only returns true if the host has asserted DTR, which in this practice pretty much amounts to "a serial monitor is watching", even `cat`ing the character device should work for this.

The main program can also follow this simple structure:

**Main Program Structure**
- Turn on the default LED on the pico
  - Not strictly necessary, but I like to do this to show that the program is running.
- Actually initialize the components by calling my functions from **Component Initialization**
- Initialize state variables and run pre-computations
- Main loop
  - Update program state based on input to the PICO (eg poll hardware, do something with that)
  - Update components based on program state (eg, change the color of an RGB LED)
  
## What we have done so far and why:

### Toolchain Setup part 1: nvim inital setup, serial monitor setup

- Chose text editor: neovim
- Fixedup neovim config/plugins/lsp a bit (huge timesink btw)
- Install Serial terminal: tio
- Verify machine state when PICO is plugged in to usb
  - When plugged in with BOOTSEL `lsusb | rg 2e8a` should show `Bus 001 Device 040: ID 2e8a:0003 Raspberry Pi RP2 Boot` - This is the PICO device is BOOT mode. Note that product ID (the part after the colon in the ouput of the `lsusb` command) is 0003.
    - When running firmware, the same command will show a product ID of 000a.
    - There are other product IDs for other modes the pico might be in, but these are the main two I'll be dealing with.
    - Permissions are given to picotool to access these devices via `/etc/udev/rules.d/60-picotool.rules`, which I copied out of the picotool repository. 
      - This works by adding a UNIX ACL extended permission to the /dev/ file, indicated by a "+" at the end of the file's permission list.
      - The exact mechanic is that the rules file tags the file that matches the pico's known vendor (2e8a) and product ids (eg: 0003) with "uaccess", which is then later picked up a udev-rule in `/usr/lib/udev/rules.d/73-seat-late.rules` to add permission for the "user logged in at the local active seat right now" to work with the device unprivledged.
  - When plugged in and running firmware, `ls -l /dev/ttyACM* /dev/pico` should show a `/dev/pico` symlink to a `/dev/ttyACM*`  (this gets set from `/etc/udev/rules.d/70-pico.rules`, which also tags the device with uaccess just like whats's described above. This will be necessary to run tio on that character device without sudo, monitoring and enabling input to the PICO's USB CDC.
  - I had actually already flashed an I2C bus scan firmware onto the PICO, so I can now verify that tio works as expected by plugging in the PICO without holding BOOTSEL, and then running `tio /dev/pico`. It prints out the bus address table as expected, and indicates the MPU-6050 is at 0x68.
    - exit tio with `ctrl+t q`, may have to wait a moment for this command to complete.
- During this process, ninja got installed and lib/tinyusb in the pico sdk got initialized. picotool was installed from the AUR (Edit: not anymore, bulding it from source now).

### Toolchain setup part 2: cmake and clangd

- Playing around with cmake before pulling in the pico_sdk_import.cmake to get a basic familiarity.
- Installed clang-format on the system
  - You can run `fd -e c -e h -x clang-format -i` from the repo root to format all .c and .h files at once.
- cmake setup with pico-sdk put together. 
  - we had to add `set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)` to tell the cmake try-compile step to skip linking, since our toolchain will depend on linking the implementation of newlib's portability layer with pico_stdlib, which the cmake's test program won't have access to.
  - symlinked compile_commands.json from build to repo root for clangd support (TODO: this should be scripted on the build or something)
  - called pico-sdk-provided cmake functions in the cmakelists.txt file in order to adjust the meta-build to work for the PICO board.
- We now have a system that can build the .uf2 file, ready to be flashed onto a PICO!

### Hello world from my manually setup toolchain on linux:

- Verified full toolchain: neovim's lsp is playing nice, we can build with cmake, flash with pico tool, and just leave a tio pane open somewhere to monitor our /dev/pico symlink and it's smart enough to handle disconnects/new flashes, etc, without tio needing to be restarted itself. 

### Toolchain loose-ends
- picotool had a bug with `picotool info` that was fixed on its master branch but hadn't made it into a release yet. Changed the install from yay to build-from-source. This requires running its install script with sudo.
- Added `CompileDatabase: build` in .clangd to get around the compile_commands symlink thing.

## Current goal

Toolchain loose ends:
- we should have a command in neovim to quickly recompile and flash our program onto the pico
- We should have a setup document that describes what the repo depends on in order to do development, and script as much of that setup as possible. 
