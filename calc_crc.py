import binascii, sys
in_file, out_file = sys.argv[1:3]
with open(in_file, "rb") as f:
    data = f.read()
crc = binascii.crc32(data)
with open(out_file, "wb") as f:
    f.write(crc.to_bytes(4, "little"))