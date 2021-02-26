# This script is hardcoded to work on a launchpad at /dev/ttyACM1 and
# and FTDI cable at ttyUSB0, unfortunately the launchpad didn't like being used
# for read and write
# TODO figure out argparse n'at to easily take in serial ports

# Connect the launchpad pin labeled <<TX to the pin on the control board labeled
# Comm/Expt RX (we're using the launchpad as a stand in for the comm/expt board)

# Connect the FTDI converter TX pin to the pin on the control board labeled
# Comm/Expt TX
import serial, time

serRead = serial.Serial('/dev/ttyACM1',115200,timeout=1);

serWrite = serial.Serial('/dev/ttyUSB0',115200);

count = 0

words = [b'testerss',b'12345678',b'artibeus',b'hilltops',b'baseball']
while True:
  serWrite.write(words[count]);
  time.sleep(1);
  serData = serRead.readline();
  print("Output is: ", serData);
  if (count + 1 < 5):
    count = count + 1
  else:
    count = 0



