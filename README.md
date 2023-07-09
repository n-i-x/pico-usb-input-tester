# pico-usb-input-tester

WIP - An RP2040 based input latency tester

## Description

This is a repository that contains code and instructions for building a system to measure input latency. The code provided is designed to run on the Raspberry Pi Pico or other RP2040 based boards and is written in C. 

## Getting Started

### Dependencies

* [Raspberry Pi Pico C/C++ SDK](https://github.com/raspberrypi/pico-sdk)
* [Pico-PIO-USB ](https://github.com/sekigon-gonnoc/Pico-PIO-USB/)
* Text Editor (Visual Studio Code, Notepad++, Vim, Emacs, etc.)

### Build

1. Install CMake (at least version 3.13), and GCC cross compiler

Debian/Ubuntu
```
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

Arch
```
sudo pacman -Sy minicom openocd cmake arm-none-eabi-gcc arm-none-eabi-newlib gcc
```

2. Clone the Raspberry Pi Pico SDK to your local computer

```
mkdir ~/pico
cd ~/pico
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
```

3. Clone pico-usb-input-tester to your local computer

```
git clone https://github.com/n-i-x/pico-usb-input-tester
cd pico-usb-input-tester
git submodule update --init
```

4. Create build folder in root

```
mkdir build
cd build
```

5. Set the Pico-SDK path and Build Platform according to your configurationa and board

```
export PICO_SDK_PATH={path_to_pico-sdk} 
export PICO_BOARD={board}
```

1. Make and Build
```
cmake ..
make -j4
```

After a successful build, the generated binary file `pico-usb-input-tester.uf2` will be located in the build directory.

### Installation

Flash the program onto the Raspberry Pi Pico:

1. Push and hold the BOOTSEL button
2. Plug the Pico into the USB port of the computer
3. Raspberry Pi Pico will mount as a Mass Storage Device called RPI-RP2
4. Drag and Drop or Copy and Paste the `pico-usb-input-tester.uf2` UF2 Binary into the RPI-RP2 Drive

### Executing

1. Lorem ipsum dolor sit amet, consectetur adipiscing elit.
2. Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium.
3. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit.
4. Sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt.
5. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit.
6. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam.
7. Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur.
   
```
code blocks for commands
```

## Help

Any advise for common problems or issues.
```
command to run if program contains helper info
```

## Authors

* [n-i-x](https://github.com/n-i-x)

## Version History

- 0.1
  - Proof of Concept