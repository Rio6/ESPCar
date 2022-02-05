import socket, time, struct, sys

packfmt = '>c2h'

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect(('192.168.0.43', 3232))

s.send(struct.pack(packfmt, b'M', 0, 0))
time.sleep(1)
s.send(struct.pack(packfmt, b'M', 16384, 0))
time.sleep(1)
s.send(struct.pack(packfmt, b'M', 0, 16384))
time.sleep(1)
s.send(struct.pack(packfmt, b'M', 0, 0))
time.sleep(1)
s.send(struct.pack(packfmt, b'X', 0, 0))

s.close()
