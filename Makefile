TOOLS_REL_ROOT = tools
TOOLS =
TOOLCHAINS = gcc

APPS = unit_tests dev flight

export BOARD = artibeus
export BOARD_MAJOR = 0
export BOARD_MINOR = 0
export DEVICE = msp430fr5994

SHARED_DEPS = libmspuartlink:gcc liblsm9ds1:gcc libartibeus:gcc libads:gcc libio:gcc libfixed:gcc \
				 libmspware:gcc libmsp:gcc libgnss:gcc

export MAIN_CLOCK_FREQ = 8000000

export CLOCK_FREQ_ACLK = 32768 # Hz
export CLOCK_FREQ_SMCLK = $(MAIN_CLOCK_FREQ)
export CLOCK_FREQ_MCLK = $(MAIN_CLOCK_FREQ)

export LIBMSP_CLOCK_SOURCE = DCO
export LIBMSP_DCO_FREQ = $(MAIN_CLOCK_FREQ)
export LIBMSP_SLEEP_TIMER = B.0.0
export LIBMSP_SLEEP_TIMER_CLK = ACLK
export LIBMSP_SLEEP_TIMER_DIV = 8*1

# Turn on/off existing processing in the Comm uart handler
export LIBMSPUARTLINK_NO_PROCESS = 0


# COMM UART
export LIBMSPUARTLINK0_UART_IDX = 0
export LIBMSPUARTLINK0_PIN_TX = 2.0
export LIBMSPUARTLINK0_PIN_RX = 2.1
export LIBMSPUARTLINK0_BAUDRATE = 115200
export LIBMSPUARTLINK0_CLK = SMCLK

# EXP UART
export LIBMSPUARTLINK1_UART_IDX = 1
export LIBMSPUARTLINK1_PIN_TX = 2.5
export LIBMSPUARTLINK1_PIN_RX = 2.6
export LIBMSPUARTLINK1_BAUDRATE = 115200
export LIBMSPUARTLINK1_CLK = SMCLK

# GNSS UART
# 9600, 8 data 0 parity
export LIBMSPUARTLINK2_UART_IDX = 2
export LIBMSPUARTLINK2_PIN_TX = 5.4
export LIBMSPUARTLINK2_PIN_RX = 5.5
export LIBMSPUARTLINK2_BAUDRATE = 9600
export LIBMSPUARTLINK2_CLK = SMCLK


# Controls for LIBARTIBEUS initialization
# (Just binary checks for enabling different peripherals)
export LIBARTIBEUS_RUN_UARTLINKS = 1
export LIBARTIBEUS_RUN_I2C = 1
export LIBARTIBEUS_COMM_HWID = 0x7461
export LIBARTIBEUS_CTRL_HWID = 0x0005
export LIBARTIBEUS_CONFIG_WATCHDOG = 1
export LIBARTIBEUS_WATCHDOG_CLK = ACLK
export LIBARTIBEUS_WATCHDOG_INTERVAL = 8192K # 4 minutes


export VOLTAGE = 3200

CONSOLE ?=

ifneq ($(CONSOLE),)
export VERBOSE = $(CONSOLE)
export LIBMSP_SLEEP = 1
export LIBIO_BACKEND = hwuart
#TODO right now we can only use uartlink0 if we're also linking in the uartlink
# library
export LIBMSP_UART_IDX = 0
export LIBMSP_UART_PIN_TX = 2.0
export LIBMSP_UART_BAUDRATE = 115200
export LIBMSP_UART_CLOCK = SMCLK
override CFLAGS += -DCONSOLE=$(CONSOLE)
endif


SHARED_DEPS +=

export CFLAGS
include tools/maker/Makefile
