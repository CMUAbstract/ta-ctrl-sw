import serial, time
import binascii

ESP_BYTE0 = 34
ESP_BYTE1 = 105
RF_KILL = 0x3F
ACK = 0x10
ASCII = 0x11
EXPT_WAKE = 0x27
EXPT_DONE = 0x28
KILL_KEYS = b'FORBESavePA15213'
ASCII_MSG = b'YINZ IS JAGOFFS!'

HWID0 =0x61
HWID1 = 0x74
seqnum0 = 67
seqnum1 = 68
dest = 0x02 # 0x1 -- from comm, 0x02 -- to expt

BYTES_PER_CMD = 129

#serWrite0 = serial.Serial('/dev/ttyACM1', 115200);
serWrite0 = serial.Serial('/dev/ttyUSB0', 115200);


SUBPAGE_00 = bytearray([
 0x00,
 0x00, 0x00, 0x04, 0x20,
 0xd5, 0x84, 0x00, 0x08,
 0xd3, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00,
 0xd3, 0x84, 0x00, 0x08,
 0xd3, 0x84, 0x00, 0x08,
 0x00, 0x00, 0x00, 0x00,
 0xd3, 0x84, 0x00, 0x08,
 0xd3, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08
])
SUBPAGE_01 = bytearray([
 0x01,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08
])
SUBPAGE_02 = bytearray([
 0x02,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08
])
SUBPAGE_03 = bytearray([
 0x03,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0x13, 0xb5, 0x02, 0x20,
 0x00, 0xf0, 0x18, 0xf9,
 0x02, 0x20, 0x00, 0xf0,
 0xe9, 0xf8, 0x01, 0x20,
 0x00, 0xf0, 0x3e, 0xf9,
 0x00, 0x20, 0x00, 0xf0,
 0x59, 0xf9, 0x04, 0x20,
 0x00, 0xf0, 0x4c, 0xf9,
 0x00, 0x20, 0x00, 0x24,
 0x00, 0xf0, 0x3e, 0xf9,
 0x00, 0xf0, 0x5e, 0xf8,
 0x04, 0x20, 0x00, 0xf0,
 0x63, 0xf8, 0x00, 0xf0,
 0x6b, 0xf8, 0x00, 0xf0,
 0x71, 0xf8, 0x23, 0x46,
 0x28, 0x22, 0x04, 0x21,
 0xcd, 0xe9, 0x00, 0x44,
 0x02, 0x20, 0x00, 0xf0,
 0x4b, 0xf9, 0x20, 0x46,
 0x00, 0xf0, 0xf4, 0xf8,
 0x20, 0x46, 0x00, 0xf0
])
SUBPAGE_04 = bytearray([
 0x04,
 0xc5, 0xf8, 0x03, 0x20,
 0x00, 0xf0, 0x1a, 0xf9,
 0x20, 0x46, 0x00, 0xf0,
 0xc7, 0xf8, 0x1a, 0x4a,
 0x1a, 0x4b, 0x13, 0x60,
 0x1a, 0x4a, 0x1b, 0x49,
 0x11, 0x60, 0x1b, 0x4a,
 0x40, 0xf6, 0x82, 0x10,
 0x13, 0x60, 0x00, 0xf0,
 0x47, 0xf9, 0x22, 0x46,
 0x18, 0x48, 0x4f, 0xf4,
 0x80, 0x63, 0x01, 0x21,
 0x00, 0xf0, 0x60, 0xf8,
 0x22, 0x46, 0x4f, 0xf4,
 0x80, 0x53, 0x14, 0x48,
 0x13, 0x4c, 0x01, 0x21,
 0x00, 0xf0, 0x58, 0xf8,
 0x11, 0x48, 0x4f, 0xf4,
 0x80, 0x61, 0x00, 0xf0,
 0x45, 0xf8, 0x0f, 0x48,
 0x4f, 0xf4, 0x80, 0x51,
 0x00, 0xf0, 0x42, 0xf8,
 0x0d, 0x4b, 0x00, 0xbf,
 0x01, 0x3b, 0xfc, 0xd1,
 0x20, 0x46, 0x4f, 0xf4,
 0x80, 0x61, 0x00, 0xf0,
 0x3c, 0xf8, 0x4f, 0xf4,
 0x80, 0x51, 0x20, 0x46,
 0x00, 0xf0, 0x37, 0xf8,
 0xf0, 0xe7, 0x00, 0xbf,
 0x00, 0x00, 0x00, 0x20,
 0x00, 0xb4, 0xc4, 0x04
])
SUBPAGE_05 = bytearray([
 0x05,
 0x04, 0x00, 0x00, 0x20,
 0x00, 0x5a, 0x62, 0x02,
 0x08, 0x00, 0x00, 0x20,
 0x00, 0x08, 0x00, 0x48,
 0x00, 0x09, 0x3d, 0x00,
 0x02, 0x4a, 0x13, 0x68,
 0x43, 0xf4, 0x80, 0x73,
 0x13, 0x60, 0x70, 0x47,
 0x00, 0x20, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf0, 0x07, 0x03,
 0x18, 0x43, 0x10, 0x60,
 0x70, 0x47, 0x00, 0xbf,
 0x00, 0x20, 0x02, 0x40,
 0x02, 0x4a, 0x13, 0x68,
 0x43, 0xf4, 0x80, 0x63,
 0x13, 0x60, 0x70, 0x47,
 0x00, 0x20, 0x02, 0x40,
 0x02, 0x4a, 0x13, 0x68,
 0x43, 0xf4, 0x00, 0x73,
 0x13, 0x60, 0x70, 0x47,
 0x00, 0x20, 0x02, 0x40,
 0x81, 0x61, 0x70, 0x47,
 0x09, 0x04, 0x81, 0x61,
 0x70, 0x47, 0x43, 0x69,
 0x01, 0xea, 0x03, 0x02,
 0x21, 0xea, 0x03, 0x01,
 0x41, 0xea, 0x02, 0x41,
 0x81, 0x61, 0x70, 0x47,
 0x2d, 0xe9, 0xf0, 0x41,
 0x05, 0x68, 0xc4, 0x68,
 0x00, 0x26, 0x4f, 0xf0
])
SUBPAGE_06 = bytearray([
 0x06,
 0x03, 0x0e, 0x43, 0xfa,
 0x06, 0xf7, 0xff, 0x07,
 0x0d, 0xd5, 0x77, 0x00,
 0x0e, 0xfa, 0x07, 0xfc,
 0x01, 0xfa, 0x07, 0xf8,
 0x25, 0xea, 0x0c, 0x05,
 0x24, 0xea, 0x0c, 0x04,
 0x02, 0xfa, 0x07, 0xf7,
 0x48, 0xea, 0x05, 0x05,
 0x3c, 0x43, 0x01, 0x36,
 0x10, 0x2e, 0xea, 0xd1,
 0x05, 0x60, 0xc4, 0x60,
 0xbd, 0xe8, 0xf0, 0x81,
 0x06, 0x28, 0x1f, 0xd8,
 0xdf, 0xe8, 0x00, 0xf0,
 0x04, 0x09, 0x0e, 0x13,
 0x18, 0x1a, 0x1c, 0x00,
 0x0d, 0x4b, 0x18, 0x68,
 0xc0, 0xf3, 0x40, 0x60,
 0x70, 0x47, 0x0b, 0x4b,
 0x18, 0x68, 0xc0, 0xf3,
 0x40, 0x40, 0x70, 0x47,
 0x08, 0x4b, 0x18, 0x68,
 0xc0, 0xf3, 0x80, 0x20,
 0x70, 0x47, 0x06, 0x4b,
 0x18, 0x68, 0xc0, 0xf3,
 0x40, 0x00, 0x70, 0x47,
 0x04, 0x4b, 0xf9, 0xe7,
 0x04, 0x4b, 0xf7, 0xe7,
 0x04, 0x4b, 0xf5, 0xe7,
 0x00, 0x20, 0x70, 0x47,
 0x00, 0x10, 0x02, 0x40
])
SUBPAGE_07 = bytearray([
 0x07,
 0x90, 0x10, 0x02, 0x40,
 0x94, 0x10, 0x02, 0x40,
 0x98, 0x10, 0x02, 0x40,
 0x08, 0xb5, 0x02, 0x46,
 0x10, 0x46, 0xff, 0xf7,
 0xcf, 0xff, 0x00, 0x28,
 0xfa, 0xd0, 0x08, 0xbd,
 0x03, 0x28, 0x08, 0xd8,
 0xdf, 0xe8, 0x00, 0xf0,
 0x16, 0x0f, 0x08, 0x02,
 0x0d, 0x4a, 0x13, 0x68,
 0x13, 0xf0, 0x0c, 0x0f,
 0xfb, 0xd1, 0x70, 0x47,
 0x0a, 0x4a, 0x13, 0x68,
 0xc3, 0xf3, 0x81, 0x03,
 0x01, 0x2b, 0xfa, 0xd1,
 0x70, 0x47, 0x07, 0x4a,
 0x13, 0x68, 0xc3, 0xf3,
 0x81, 0x03, 0x02, 0x2b,
 0xfa, 0xd1, 0x70, 0x47,
 0x03, 0x4a, 0x13, 0x68,
 0xc3, 0xf3, 0x81, 0x03,
 0x03, 0x2b, 0xfa, 0xd1,
 0x70, 0x47, 0x00, 0xbf,
 0x08, 0x10, 0x02, 0x40,
 0x06, 0x28, 0x0a, 0xd8,
 0xdf, 0xe8, 0x00, 0xf0,
 0x04, 0x0a, 0x0f, 0x14,
 0x19, 0x1b, 0x1d, 0x00,
 0x0d, 0x4a, 0x13, 0x68,
 0x43, 0xf0, 0x80, 0x73,
 0x13, 0x60, 0x70, 0x47
])
SUBPAGE_08 = bytearray([
 0x08,
 0x0a, 0x4a, 0x13, 0x68,
 0x43, 0xf4, 0x80, 0x33,
 0xf8, 0xe7, 0x08, 0x4a,
 0x13, 0x68, 0x43, 0xf4,
 0x80, 0x73, 0xf3, 0xe7,
 0x05, 0x4a, 0x13, 0x68,
 0x43, 0xf0, 0x01, 0x03,
 0xee, 0xe7, 0x04, 0x4a,
 0xf9, 0xe7, 0x04, 0x4a,
 0xf7, 0xe7, 0x04, 0x4a,
 0xf5, 0xe7, 0x00, 0xbf,
 0x00, 0x10, 0x02, 0x40,
 0x90, 0x10, 0x02, 0x40,
 0x94, 0x10, 0x02, 0x40,
 0x98, 0x10, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf0, 0x03, 0x03,
 0x18, 0x43, 0x10, 0x60,
 0x70, 0x47, 0x00, 0xbf,
 0x08, 0x10, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf4, 0x60, 0x53,
 0x43, 0xea, 0xc0, 0x20,
 0x10, 0x60, 0x70, 0x47,
 0x08, 0x10, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf4, 0xe0, 0x63,
 0x43, 0xea, 0x00, 0x20,
 0x10, 0x60, 0x70, 0x47,
 0x08, 0x10, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf0, 0xf0, 0x03
])
SUBPAGE_09 = bytearray([
 0x09,
 0x43, 0xea, 0x00, 0x10,
 0x10, 0x60, 0x70, 0x47,
 0x08, 0x10, 0x02, 0x40,
 0x10, 0xb5, 0x03, 0x9c,
 0x01, 0x39, 0x64, 0x06,
 0x44, 0xea, 0x01, 0x14,
 0x20, 0x43, 0x02, 0x9c,
 0x40, 0xea, 0x44, 0x50,
 0x03, 0x43, 0x43, 0xea,
 0x02, 0x22, 0x02, 0x4b,
 0x42, 0xf0, 0x80, 0x72,
 0x1a, 0x60, 0x10, 0xbd,
 0x0c, 0x10, 0x02, 0x40,
 0x43, 0x09, 0x03, 0xf1,
 0x80, 0x43, 0x03, 0xf5,
 0x04, 0x33, 0x00, 0xf0,
 0x1f, 0x00, 0x19, 0x68,
 0x01, 0x22, 0x02, 0xfa,
 0x00, 0xf0, 0x08, 0x43,
 0x18, 0x60, 0x70, 0x47,
 0xfe, 0xe7, 0x70, 0x47,
 0x38, 0xb5, 0x1a, 0x4a,
 0x1a, 0x4b, 0x1b, 0x49,
 0x8b, 0x42, 0x1b, 0xd3,
 0x1a, 0x4a, 0x00, 0x21,
 0x93, 0x42, 0x1c, 0xd3,
 0x19, 0x4a, 0x1a, 0x4c,
 0x13, 0x68, 0x1a, 0x4d,
 0x43, 0xf4, 0x00, 0x73,
 0x13, 0x60, 0x53, 0x6f,
 0x43, 0xf4, 0x70, 0x03,
 0x53, 0x67, 0xac, 0x42
])
SUBPAGE_0A = bytearray([
 0x0A,
 0x12, 0xd3, 0x16, 0x4c,
 0x16, 0x4d, 0xac, 0x42,
 0x12, 0xd3, 0xff, 0xf7,
 0x4f, 0xfe, 0x15, 0x4c,
 0x15, 0x4d, 0xac, 0x42,
 0x10, 0xd3, 0x38, 0xbd,
 0x52, 0xf8, 0x04, 0x0b,
 0x43, 0xf8, 0x04, 0x0b,
 0xdc, 0xe7, 0x43, 0xf8,
 0x04, 0x1b, 0xdd, 0xe7,
 0x54, 0xf8, 0x04, 0x3b,
 0x98, 0x47, 0xe6, 0xe7,
 0x54, 0xf8, 0x04, 0x3b,
 0x98, 0x47, 0xe6, 0xe7,
 0x54, 0xf8, 0x04, 0x3b,
 0x98, 0x47, 0xe8, 0xe7,
 0x6c, 0x85, 0x00, 0x08,
 0x00, 0x00, 0x00, 0x20,
 0x0c, 0x00, 0x00, 0x20,
 0x0c, 0x00, 0x00, 0x20,
 0x14, 0xed, 0x00, 0xe0,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x00, 0x09, 0x3d, 0x00,
 0x00, 0x09, 0x3d, 0x00,
 0x00, 0x09, 0x3d, 0x00,
 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff
])

pages = [SUBPAGE_00, SUBPAGE_01, SUBPAGE_02, SUBPAGE_03, SUBPAGE_04, SUBPAGE_05,
SUBPAGE_06, SUBPAGE_07, SUBPAGE_08, SUBPAGE_09, SUBPAGE_0A]

WAKE_KEYS = "WaKeUpTA"
wake_keys = bytearray(b'\'') + bytearray(b'WaKeUpTA')
BOOTLOADER_WRITE_PAGE = 0x02
BOOTLOADER_JUMP = 0x0b

nil = bytearray('')

class openlst_pkt:
  def __init__(self, hwid0=HWID0, hwid1=HWID1, seq0=0, seq1=0, dest=1,
  msg_type=ACK, payload=nil):
    self.header = bytearray([ESP_BYTE0, ESP_BYTE1, len(payload)+6, hwid0,
    hwid1, seq0, seq1, dest, msg_type])
    self.pkt = self.header + payload




if __name__ == "__main__":
  # Write ascii to wake up expt
  #ascii_cmd =  openlst_pkt(seq0=0,seq1=0,msg_type=ASCII,payload=wake_keys)
  #serWrite0.write(ascii_cmd.pkt)
  #print(binascii.hexlify(ascii_cmd.pkt))
  time.sleep(1)
  # Write pages
  i = 0
  for page in pages:
    i += 1
    page_cmd = openlst_pkt(seq0=i, seq1=0, msg_type=BOOTLOADER_WRITE_PAGE,
    payload=page)
    serWrite0.write(page_cmd.pkt)
    #print(binascii.hexlify(page_cmd.pkt))
    time.sleep(1)
  # Write jump
  jump_cmd = openlst_pkt(seq0=i, seq1=0, msg_type=BOOTLOADER_JUMP,payload=nil)
  i += 1
  #print(binascii.hexlify(jump_cmd.pkt))
  serWrite0.write(jump_cmd.pkt)
  # Write ascii expt_done
  done_cmd = openlst_pkt(seq0=i, seq1=0, msg_type=ASCII, payload=bytearray(EXPT_DONE))
  serWrite0.write(done_cmd.pkt)
  #print(binascii.hexlify(done_cmd.pkt))
  #print(done_cmd.pkt)



