full_len = 44
test_pkt = [0x00, 0x4B, 0xFF, 0xC4, 0xFF, 0x0A, 0x21, 0xDD, 0xFF, 0xF7, 0xFF, 0xDA, 0xFF,
0x69, 0x01, 0x9D, 0xFF, 0x3E, 0xF3, 0xAE, 0x07, 0x00, 0x00, 0xEA, 0x0C, 0x01,
0x28, 0x1A, 0x86, 0x18, 0x4F, 0x00, 0x38, 0x00, 0xCA, 0x1C, 0x02, 0x00, 0x09,
0x02, 0x15, 0x0B, 0x3B, 0x2D, 0x00]

test_pkt1 = [0x22,0xFF,0xD1,0xFF,0xE3,0x20,0xD7,0xFF,0xFB,0xFF,0xC8,0xFF,0x9B,0x01,0xBF,0xFF,0x08,0xF3,0xB0,0x07,0x00,0x00,0xEB,0x0C,0x01,0x28,0x1A,0x86,0x18,0x4F,0x00,0x38,0x00,0xCA,0x1C,0x02,0x00,0x09,0x02,0x15,0x0B,0x3B,0x2D,0xC8]


indices = [0,2,4,  6,8,10,  12,14,16,  18,20,22,24,  26,27,28,  30,32,34,
36,37,  38,39,40,  41,42,43]

is_signed = [1,1,1,1,1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0]
xl_x = 0
xl_y = 2
xl_z = 4
g_x = 6
g_y = 8
g_z = 10
m_x = 12
m_y = 14
m_z = 16
pwr0 = 18
pwr1 = 20
pwr2 = 22
pwr3 = 24
gps_degs_lat = 26
gps_min_lat = 27
gps_secs_lat = 28

gps_degs_long = 30
gps_min_long = 32
gps_secs_long = 34
gps_ns_ew = 36
gps_error = 37
month = 38
day = 39
year = 40
hrs = 41
mins = 42
secs = 43

def twos_complement(hexstr,bits):
  value = int(hexstr,16)
  if value & (1 << (bits-1)):
    value -= 1 << bits
  return value

def twos_complement_int(value, bits):
  if value & (1 << (bits-1)):
    value -= 1 << bits
  return value

def process_pkt(pkt=test_pkt):
  for count, ind in enumerate(indices):
    # Concatenate the right bytes into a single var
    if ind < full_len - 1:
      num_bytes = indices[count+1] - ind
      value = 0
      for i in range(num_bytes):
        #print("\t" + hex(pkt[ind + i]))
        value += pkt[ind + i] << 8*i
      #print("Using ",num_bytes*8)
      if (is_signed[count]):
        new_val = twos_complement_int(value,num_bytes*8)
      else:
        new_val = value
      print(new_val)
    else:
      print("Here?")
      new_val = twos_complement_int(value,8)
      print(new_val)
