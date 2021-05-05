# This script works with a bundle of different devices connected to the control
# board.
# TODO: still need to add argparse stuff

import serial,time

serRead0 = serial.Serial('/dev/ttyACM1', 115200,timeout = 1);

serWrite0 = serial.Serial('/dev/ttyUSB0', 115200);
serWrite1 = serial.Serial('/dev/ttyACM3',115200);

count = 0;

words = [b'TESTERS1', b'ORIOLES1', b'STEELERS', b'BASEBALL', \
        b'FOOTBALL', b'CAMPSITE']

words1 = [b'testers1', b'orioles1', b'steelers', b'baseball', \
        b'football', b'campsite']

while True:
  serWrite0.write(words[count])
  serWrite1.write(words1[count])
  time.sleep(1)
  serData0 = serRead0.readline()
  print("Output is: ", serData0)
  if (count + 1 < 6):
    count = count + 1
  else:
    count = 0
