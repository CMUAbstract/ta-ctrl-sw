# A python script that utilizes the unit test mailbox command parse
# This script is hardcoded to work on a launchpad at /dev/ttyACM1 and
# and FTDI cable at ttyUSB0. 
#
# Optional setup: Connect FTDI TX to comm/TX (FTDI as a stdin for transmitting data)
#                 Connect expt/RX to TX on launchpad (launchpad as a stdin 
#                                                               for receiving data)
#
# Usage: python scripts/command_parse.py
# Assumptions:
#  - You are in the ta-ctrl folder and you have ran the make file with the
#    UNIT_TEST_MAILBOX_COMMAND_PARSE flag enabled
# Arguments:
#  - None; this script assumes the git repo directory structure. Note that, due
#    to this assumption, this script should be executed from the top-level
#    scripts directory (i.e. tartan-artibeus-sw/ta-ctrl/)
# Results:
#  - Should parse the hardcoded commands to acknowlege the start byte 1 and 2 and
#    then the length of the command (This is all done on the c code side). This
#    script is simply for sending the commands and recieving and printing the parsed
#    information
#
# Written by Chad Taylor
# Other contributors: None
#
# See the top-level LICENSE file for the license.



import serial, time

serRead = serial.Serial('/dev/ttyACM1',115200,timeout=1);

serWrite = serial.Serial('/dev/ttyUSB0',115200);


START_BYTE0 = 0x22
START_BYTE1 = 0x69
hwid_lsb =65
hwid_msb = 66
msgid_lsb = 67
msgid_msb = 68
dest = 1
ACK = 0x10

total_len = 6
#Command for COMMON_ACK
common_ack = bytearray([START_BYTE0, START_BYTE1, total_len, hwid_lsb, hwid_msb, 
						msgid_lsb, msgid_msb, dest, ACK])

#Send command for parsing to comm TX
serWrite.write(common_ack);
time.sleep(1);
#Read first start byte ack
serData = serRead.readline();
print("Output is: ", serData);
time.sleep(1);
#Read second start byte ack
serData = serRead.readline();
print("Output is: ", serData);
time.sleep(1);
#Prints length of the command from command byte
serData = serRead.readline();
print("Output is: ", serData);