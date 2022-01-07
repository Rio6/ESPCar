#!/usr/bin/env python3

ADDR = '192.168.0.33' # TODO move this somewhere
PORT = 8266

import sys, os, socket
if len(sys.argv) <= 1:
    print(f'{sys.argv[0]} bin_file', file=sys.stderr)
    exit(1)

file = sys.argv[1]
size = os.path.getsize(file)

with open(file, 'rb') as f, socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.settimeout(5)
    s.connect((ADDR, PORT))
    s.send(int.to_bytes(size, 4, 'big'))
    sent = 0
    while True:
        data = f.read(512)
        data_size = len(data)
        if data_size == 0:
            break
        s.send(data)
        sent += data_size
        print(f'{sent / size * 100:.2f}%', end='\r')
    print('\n' + s.recv(64).decode())
