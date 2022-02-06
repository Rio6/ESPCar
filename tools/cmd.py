import socket, time, struct, sys, threading, signal, subprocess

cmdfmt = '>c2h'
headerfmt = '>Bc6s'

running = False
conn = None
ffplay = None

lx = 0
rx = 0
ly = 0
ry = 0

def recv():
    global rx, ry

    while running:
        size = int.from_bytes(conn.recv(1), 'big')

        data = conn.recv(size)

        if chr(data[1]) == 'S':
            status = struct.unpack(headerfmt + '2h', data)
            rx = status[3]
            yx = status[4]
        elif chr(data[1]) == 'V':
            ffplay.communicate(data[8:])

def send():
    lastsend = 0
    while running:
        now = time.time()
        if lx != rx or ly != ry or now - lastsend > 1:
            conn.send(struct.pack(cmdfmt, b'M', lx, ly))
            lastsend = now
        time.sleep(0.05)

def main():
    global conn, running, ffplay

    ffplay = subprocess.Popen(['ffplay', '-f', 'mjpeg', '-i', '-'], stderr=subprocess.DEVNULL)

    conn = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    conn.connect(('192.168.0.43', 3232))
    conn.send(struct.pack(cmdfmt, b'.', 0, 0))
    print(conn.getsockname())

    print("starting")
    running = True

    recv_thread = threading.Thread(target=recv)
    send_thread = threading.Thread(target=send)

    recv_thread.start()
    send_thread.start()

    signal.sigwait([signal.SIGINT])

    print("stopping")
    running = False

    recv_thread.join()
    send_thread.join()

    conn.send(struct.pack(cmdfmt, b'X', 0, 0))
    conn.close()

    ffplay.terminate()
    ffplay.wait()

if __name__ == '__main__':
    main()
