import socket, time, struct

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('192.168.0.43', 3232))

s.send(b'M' + struct.pack('>2h', 25, 0))
print("a")
time.sleep(1)
s.send(b'M' + struct.pack('>2h', 50, 0))
print("a")
time.sleep(1)
s.send(b'M' + struct.pack('>2h', 75, 0))
print("a")
time.sleep(1)
s.send(b'M' + struct.pack('>2h', 0, 0))
print("a")
time.sleep(1)
#s.send(b'M' + struct.pack('>2h', 0, 0))

s.close()
