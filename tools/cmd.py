import socket, time, struct, sys

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect(('192.168.0.43', 3232))

s.send(struct.pack('>c2h', b'M', int(sys.argv[1]), int(sys.argv[2])))

s.close()
