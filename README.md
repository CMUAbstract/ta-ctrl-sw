Software packages and applications for the Tartan Artibeus PocketQube
satellite's Control board.  The code targets the MSP403FR5994 on board.

This repo contains

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

Using this repo:

* Clone:
    git clone --recursive <this repo name>

* Deps: The following dependencies are required and were tested on Linux Mint

[mspgcc](https://www.ti.com/tool/download/MSP430-GCC-OPENSOURCE): This code was
tested using version 8.3.1.25 . Download from the link using the all in one
installer (msp430-gcc-full-linux-x64-installer-8.3.1.0.run for linux). Double
check the path- it should be installed at /opt/ti/mspgcc -- then add the
following to your .bashrc file:

    export MSPDEBUG_TILIB_PATH=/opt/ti/mspgcc/bin

[mspdebug](https://dlbeer.co.nz/mspdebug/): This code was tested using version
0.25 . It's available in apt, the package name is just mspdebug

    sudo apt-get install mspdebug

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

