import serial,time

ESP_BYTE0 = 34
ESP_BYTE1 = 105
RF_KILL = 0x3F
ASCII = 0x11
EXPT_WAKE = 0x27
EXPT_DONE = 0x28
KILL_KEYS = b'FORBESavePA15213'
ASCII_MSG = b'YINZ IS JAGOFFS!'

hwid0 =65
hwid1 = 66
seqnum0 = 67
seqnum1 = 68
dest = 1
serWrite0 = serial.Serial('/dev/ttyUSB0', 115200);

total_len = 19

ascii_message = bytearray([ESP_BYTE0, ESP_BYTE1, total_len, hwid0, hwid1,
seqnum0, seqnum1, dest, ASCII, 0x59, 0x49, 0x4e ,0x5a,0x20,0x4a, 0x41,0x47,
0x4f, 0x46, 0x46, 0x53,0x21])

total_len = 15
xfer_message = bytearray([ESP_BYTE0, ESP_BYTE1, total_len, hwid0, hwid1,
seqnum0, seqnum1, dest, ASCII, EXPT_WAKE,0x53,0x63,0x6f,0x74,0x74,0x79,0x54,0x41])

total_len = 7
done_message = bytearray([ESP_BYTE0, ESP_BYTE1, total_len, hwid0, hwid1,
seqnum0, seqnum1, dest, ASCII, EXPT_DONE])

# send turn on twice
serWrite0.write(xfer_message);
time.sleep(1);
serWrite0.write(xfer_message);
time.sleep(1);
for i in range(2):
  serWrite0.write(ascii_message);
  time.sleep(1)
  # Now open /dev/ttyACM1 and look for output

# Now disable
serWrite0.write(done_message);
time.sleep(1)
serWrite0.write(done_message);
time.sleep(1)

# Now make sure we don't output anything
for i in range(3):
  serWrite0.write(ascii_message);
  time.sleep(1)
