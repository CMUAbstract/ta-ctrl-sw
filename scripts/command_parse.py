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


#Command for COMMON_ACK
START_BYTE0 = 0x22
START_BYTE1 = 0x69
hwid_lsb_ack =65
hwid_msb_ack = 66
msgid_lsb_ack = 67
msgid_msb_ack = 68
dest = 1
opcode = 0x10

total_len_ack = 6

common_ack = bytearray([START_BYTE0, START_BYTE1, total_len_ack, hwid_lsb_ack, 
						hwid_msb_ack, msgid_lsb_ack, msgid_msb_ack, dest, opcode])

#Command for COMMON_ASCII
total_len_ascii = 17
hwid_lsb_ascii = 85
hwid_msb_ascii = 86
msgid_lsb_ascii = 87
msgid_msb_ascii = 88
#Ascii data is "Hello World" in hex
common_ascii = bytearray([START_BYTE0, START_BYTE1, total_len_ascii, hwid_lsb_ascii, 
						hwid_msb_ascii, msgid_lsb_ascii, msgid_msb_ascii, dest, opcode,
						0x48,0x65,0x6c,0x6c,0x6f, 
						0x20,0x57,0x6f,0x72,0x6c,0x64])

sent_comm = common_ascii #Change this array to test different commands
#Send first three bytes of command to comm TX so as to retreive length of command
serWrite.write(sent_comm[0:3])
time.sleep(1)
#Send command for parsing to comm TX
serWrite.write(sent_comm)
time.sleep(1)
#Read first start byte ack
serData = serRead.readline()
print(serData)
time.sleep(1)
#Read second start byte ack
serData = serRead.readline()
print(serData)
time.sleep(1)
#Prints length of the command from command byte
serData = serRead.readline()
len_of_command = ord(serData[22])
serData = serData[0:22] + str(ord(serData[22])) + '\n'
print(serData)
time.sleep(1)
#Read HW_ID_LSB byte
serData = serRead.readline()
serData = serData[0:11] + hex(ord(serData[11])) + '\n'
print(serData)
time.sleep(1)
#Read HW_ID_MSB byte
serData = serRead.readline()
serData = serData[0:11] + hex(ord(serData[11])) + '\n'
print(serData)
time.sleep(1)
#Read MSG_ID_LSB byte
serData = serRead.readline()
serData = serData[0:12] + hex(ord(serData[12])) + '\n'
print(serData)
time.sleep(1)
#Read MSG_ID_MSB byte
serData = serRead.readline()
serData = serData[0:12] + hex(ord(serData[12])) + '\n'
print(serData)
time.sleep(1)

#Prints entire message in byte format
serData = serRead.readline()
print(serData[0:20])
time.sleep(1)
message_str = ""
serData = serRead.readline()
#Added 3 for the first three bytes (start_byte0, start_byte1 and length_byte)
for i in range(len_of_command+3):
	message_str += hex(ord(serData[i])) + " "

print(message_str)

