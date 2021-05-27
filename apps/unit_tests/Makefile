OBJECTS = main.o \
          gps.o

EXEC = dev

DEPS = libmspuartlink:gcc libartibeus:gcc libads:gcc libio:gcc libfixed:gcc \
				libmspware:gcc libmsp:gcc libgnss:gcc

include tools/maker/Makefile

ifneq ($(UNIT_TEST_POWER_ON),)
override CFLAGS += -DTEST_POWER_ON
endif

ifneq ($(UNIT_TEST_ECHO),)
override CFLAGS += -DTEST_ECHO
endif

ifneq ($(UNIT_TEST_UART_ADC),)
override CFLAGS += -DTEST_UART_ADC
endif

ifneq ($(UNIT_TEST_MULTI_UART_TX),)
override CFLAGS += -DTEST_MULTI_UART_TX
endif

ifneq ($(UNIT_TEST_MAILBOX),)
override CFLAGS += -DTEST_MAILBOX
endif

ifneq ($(UNIT_TEST_ADC),)
override CFLAGS += -DTEST_ADC
endif

ifneq ($(UNIT_TEST_ACKS),)
override CFLAGS += -DTEST_ACKS
endif

ifneq ($(UNIT_TEST_COMM_REPEAT),)
override CFLAGS += -DTEST_COMM_REPEAT
endif

ifneq ($(UNIT_TEST_COMM_TX_LONG),)
override CFLAGS += -DTEST_COMM_TX_LONG
endif

ifneq ($(UNIT_TEST_BURN_WIRE),)
override CFLAGS += -DTEST_BURN_WIRE
endif

ifneq ($(UNIT_TEST_GNSS),)
override CFLAGS += -DTEST_GNSS
endif

ifneq ($(UNIT_TEST_XFER),)
override CFLAGS += -DTEST_XFER
endif

ifneq ($(UNIT_TEST_MAILBOX_COMMAND_PARSE),)
override CFLAGS += -DTEST_MAILBOX_COMMAND_PARSE
endif

ifneq ($(UNIT_TEST_EXPT_SET_TIME),)
override CFLAGS += -DTEST_EXPT_SET_TIME
endif


