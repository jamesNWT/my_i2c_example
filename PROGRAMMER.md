# PROGRAMMER.md

sort of like a CLAUDE.md, but written exclusively by a programmer. LLMs do not modify this file.

## What we're building and why

To introduce myself to firmware development, I'm working on writing programs for the raspberry pi PICO H 1. This project in particular is for learning how to use I2C. We shall build up the program from first principles, to eventually create a device that can do something interesting with sensor data from the MPU-6050

### Current hardware list

- RaspberryPi PICO-H 1

- MPU-6050

### Current circuit diagram
```ASCI

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
  - When plugged in with BOOTSEL `lsusb | rg 2e8a` should show `Bus 001 Device 040: ID 2e8a:0003 Raspberry Pi RP2 Boot` - This is the PICO device is BOOT mode. Note that product ID (the part after the colon in the ouput of the `lsusb` command) is 0003. This changes between a PICO in boot mode and when it is running firmware. `/usr/lib/udev/rules.d/60-picotool.rules` (copied out of the pico sdk by the aur install script) handles giving picotool permissions to this device, in a method similar to what is described below. There are several other product IDs the PICO can take, and that file handles udev-rules for all of them, but the other ones are out of scope for now. 
  - When plugged in and running firmware `ls -l /dev/ttyACM* /dev/pico` should show a `/dev/pico` symlink to a `/dev/ttyACM*` character device which has a + at the end of its permissions enumeration indicating a UNIX ACL extended permission (this gets set from `/etc/udev/rules.d/70-pico.rules`, which claude taught me how to write - it basically matches on whatever has the tty subsystem, 2e8a idVendor, and 000a idProduct, creates a "pico" symlink to that device, and tags that device with uaccess, which then gets picked up by a udev-rule in `/usr/lib/udev/rules.d/73-seat-late.rules` to add permission for the "user logged in at the local active seat right now" to work with the ttyACM unprivledged. This will be necessary to run tio on that character device without sudo, monitoring and enabling input to the PICO's USB CDC.
  - I had actually already flashed an I2C bus scan firmware onto the PICO, so I can now verify that tio works as expected by plugging in the PICO without holding BOOTSEL, and then running `tio /dev/pico`. It prints out the bus address table as expected, and indicates the MPU-6050 is at 0x68.
    - exit tio with `ctrl+t q`, may have to wait a moment for this command to complete.
- During this process, ninja got installed and lib/tinyusb in the pico sdk got initialized.

### Toolchain setup part 2: cmake and clangd

- Playing around with cmake before pulling in the pico_sdk_import.cmake to get a basic familiarity.

## Current goal

Write a hello_world.c program to flash on to the PICO, in order to verify building a PICO-ready program in this environment. 
- Verify syntax highlighting and LS features work as expected in a C program.
- We're going to have to include header files from the pico sdk, will we need to set anything up to make clangd and treesitter play nice?
- CMakeLists.txt will generate our CMake file, but I have no idea the actual mechanics of making this work.
- What will the build artifact be?
- How do I copy the build artifact to the PICO? Can a picotool command do it?
- Can I configure commands in neovim for compiling and flashing/running the PICO, to get close to how easy this is to do in vscode with the PICO extension? Do we want to run tio in neovim or is switching to a terminal ok? Should I look into the harpoon2 plugin? 
