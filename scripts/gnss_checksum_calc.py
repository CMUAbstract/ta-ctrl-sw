import numpy as np

mask = 0xFF
test = 'PSRF103,00,01,00,01'
gll_turn_off = 'PSRF103,01,00,00,00'
gsa_turn_off = 'PSRF103,02,00,00,00'
gsv_turn_off = 'PSRF103,03,00,00,00'
rmc_turn_off = 'PSRF103,04,00,00,00'
vtg_turn_off = 'PSRF103,05,00,00,00'

msgs = [
test,
gll_turn_off,
gsa_turn_off,
gsv_turn_off,
rmc_turn_off,
vtg_turn_off,
]
# I'm assuming the $ and * are already stripped
def calc_nmea_checksum(msg):
  ck = 0
  for char in msg:
    ck = (ord(char) & mask) ^ ck
  return ck


if __name__ == "__main__":
  for msg in msgs:
    ck = calc_nmea_checksum(msg)
    print(msg,"*",hex(ck))
