Software packages and applications for the Tartan Artibeus PocketQube
satellite's Control board.  The code targets the MSP403FR5994 on board.

## Contents:

* Libraries for:
  * GNSS module
  * Exercising the Comm board
  * ADS analog to digital converter
  * Running multiple UART ports simultaneously
  * Initializing the Artibeus control board

* "Unit" tests that demonstrate:
  * Powering up
  * UART tests
  * Comm with OpenLST
  * Power readings using the ADS
  * Exercising multiple UARTs at once
  * Passing data from one UART device to another

* Dev code that demonstrates a full application:
  * Booting up for the first time and deploying antenna
  * Saving and restoring a checkpoint before/after power failures
  * Logging state that could be corrupted by a power failure
  * Reading power status data from the ADC on the power-board
  * Sending and receiving data from the modified OpenLST board over UART
  * Receiving GPS data
  * Selectively transmitting based on GPS location
  * Turning on the experiment board
  * Sleeping to preserve energy

* [Several scripts](https://github.com/CMUAbstract/artibeus-ecosystem/tree/master/scripts) to test
the UART response protocol

* [Maker](https://github.com/CMUAbstract/Maker) A build system to pull together the different libraries

## Installation:

* Clone:
    git clone --recursive <this repo name>

* Install Deps: The following dependencies are required and were tested on Linux Mint,
* Windows Subsystem for Linux (WSL), and OSX on Mac

[mspgcc](https://www.ti.com/tool/download/MSP430-GCC-OPENSOURCE): This code was
tested using version 8.3.1.25 . Download from the link using the all in one
installer (msp430-gcc-full-linux-x64-installer-8.3.1.0.run for linux). You can
use the following commands to do this:

    wget https://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/8_3_1_0/export/msp430-gcc-full-linux-x64-installer-8.3.1.0.run
    chmod +x msp430-gcc-full-linux-x64-installer-8.3.1.0.run
    ./msp430-gcc-full-linux-x64-installer-8.3.1.0.run

Specify the path where mspgcc is installed- it should be installed at
/opt/ti/mspgcc on linux systems -- then define the following shell variable.
Note: it's a good idea to put this line in your .bashrc file so you don't have
to re-run this every time you open a terminal.

    export MSPDEBUG_TILIB_PATH=/path/to/mspgcc/bin

On linux systems, if mspgcc is installed in the usual place, this looks like:

    export MSPDEBUG_TILIB_PATH=/opt/ti/mspgcc/bin

[mspdebug](https://dlbeer.co.nz/mspdebug/): This code was tested using version
0.25 . Only an old version is available in apt, so download and install the
latest from github.

    git clone git@github.com:dlbeer/mspdebug.git
    cd mspdebug
    make
    make install

mspdebug has a number of dependencies that you can install using either apt (on
Linux/WSL) or Homebrew (on Mac):

On Linux/WSL:

    sudo apt install libusb-dev
    sudo apt install libusb-1.0-0

On Mac:

    brew install libusb
    brew install libusb-compat
    brew install hidapi

Once mspgcc and mspdebug as well as the dependencies are installed, test your
setup by running:

    mspdebug tilib

Without hardware connected the output should be something like this:

    MSPDebug version 0.25 - debugging tool for MSP430 MCUs
    Copyright (C) 2009-2017 Daniel Beer <dlbeer@gmail.com>
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    Chip info database from MSP430.dll v3.15.0.1 Copyright (C) 2013 TI, Inc.

    Using new (SLAC460L+) API
    MSP430_GetNumberOfUsbIfs
    No unused FET found.

## Required Hardware:

[TA-CTRL board](http://abstract.ece.cmu.edu/ta-1/overview.html): This repo
contains code that targets the MSP430FR5994 chip on this board.

[MSP-FET](https://www.ti.com/tool/MSP-FET) or the [ez-FET on an MSP430
launchpad](https://www.ti.com/tool/MSP-EXP430FR5994#technicaldocuments): Either
of these programmers can be used to program the TA-CTRL board by connecting to
the Vdd, Gnd, TST and RST pins.  See [this
tutorial](https://cmuabstract.github.io/intermittence_tutorial/) for details on
how to use the launchpad to program hardware external to the launchpad.

Once you have connected the MSP-FET or ez-FET over USB to your machine, and
connected the TA-CTRL board to the programmer, rerun:

    mspdebug tilib

Now MSPDebug should successfully start, and the top of the output will be
something like:

    MSPDebug version 0.25 - debugging tool for MSP430 MCUs
    Copyright (C) 2009-2017 Daniel Beer <dlbeer@gmail.com>
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    Chip info database from MSP430.dll v3.15.0.1 Copyright (C) 2013 TI, Inc.

    Using new (SLAC460L+) API
    MSP430_GetNumberOfUsbIfs
    MSP430_GetNameOfUsbIf
    Found FET: ttyACM0
    MSP430_Initialize: ttyACM0
    Firmware version is 31400000
    MSP430_VCC: 3000 mV
    MSP430_OpenDevice
    MSP430_GetFoundDevice
    Device: MSP430FR5994 (id = 0x01e9)
    3 breakpoints available
    MSP430_EEM_Init
    Chip ID data:
      ver_id:         82a1
      ver_sub_id:     0000
      revision:       21
      fab:            55
      self:           5555
      config:         10
      fuses:          55
    warning: unknown chip


## Building and Running Code:

* Cleaning the app:
    make apps/<app name>/bld/<toolchain>/clean
* Cleaning the dependency libraries:
    make apps/<app name>/bld/<toolchain>/depclean
* Building the dependency libraries:
    make apps/<app name>/bld/<toolchain>/dep
* Building everything:
    make apps/<app name>/bld/<toolchain>/all
* To build and run a specific unit tests:
    make apps/unit_test/bld/gcc/clean
    make apps/unit_test/bld/gcc/all UNIT_TEST_GNSS=1
Note: apps/unit_tests/README provides a description of each test and the
associated flags.

* Programming:  Use an [MSP-FET](https://www.ti.com/tool/MSP-FET) or
[MSP430FR5994 launchpad](https://www.ti.com/tool/MSP-EXP430FR5994) to program
the Control board. 
  * Connect 3.3V, Gnd, TST and RST on either programmer to the
corresponding pins on the Control board.
  * Ground the DBG1 pin on the Control board-- this stops the first boot from
starting when the programmer resets the MCU on the control board
  * Run the following from the command prompt:
    make apps/<app name>/bld/<toolchain>/prog
* Examples:
    make apps/dev/bld/gcc/all
    make apps/dev/bld/gcc/prog

* Unit test example: (See the [Unit tests README](apps/unit_tests/README) for
* more details and test options)
    make apps/unit_tests/bld/gcc/depclean
    make apps/unit_tests/bld/gcc/clean
    make apps/unit_tests/bld/gcc/all UNIT_TEST_ECHO=1
    make apps/unit_tests/bld/gcc/prog
