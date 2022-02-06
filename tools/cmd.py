import socket, time, struct, sys, threading, subprocess, select
from pynput import mouse, keyboard

cmdfmt = '>c2h'
headerfmt = '>Bc'

running = False
conn = None
ffplay = None

lx = 0
rx = 0
ly = 0
ry = 0
origin = None

def clamp(n, low, high):
    return min(max(n, low), high)

def recv():
    global rx, ry

    while running:
        i, o, e = select.select([conn], [], [], 0.5)
        if not i: continue

        data = conn.recv(1024)

        if chr(data[1]) == 'S':
            status = struct.unpack(headerfmt + '2h', data)
            rx = status[2]
            ry = status[3]
        elif chr(data[1]) == 'V':
            try:
                ffplay.stdin.write(data[2:])
            except BrokenPipeError:
                pass

def send():
    lastsend = 0
    while running:
        now = time.time()
        if lx != rx or ly != ry or now - lastsend > 1:
            conn.send(struct.pack(cmdfmt, b'M', lx, ly))
            lastsend = now
        time.sleep(0.05)

def onmove(x, y):
    global lx, ly
    if origin is not None:
        lx = int(clamp(x - origin[0], -500, 500) /  500 * 2**15)
        ly = int(clamp(y - origin[1], -500, 500) / -500 * 2**15)

def onclick(x, y, btn, press):
    global origin, lx, ly
    if press:
        origin = (x, y)
    else:
        lx = ly = 0
        origin = None

def onkey(key):
    global lx, ly
    if key == keyboard.KeyCode.from_char('r'):
        conn.send(struct.pack(cmdfmt, b'R', 0, 0))
    elif key == keyboard.Key.space:
        lx = ly = 0

def main():
    global conn, running, ffplay

    ffplay = subprocess.Popen(
        ['ffplay', '-f', 'mjpeg', '-vf', 'hflip,vflip', '-i', '-'],
        stdin=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
    )

    conn = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    conn.connect(('192.168.0.43', 3232))
    conn.send(struct.pack(cmdfmt, b'.', 0, 0))

    print("starting")
    running = True

    recv_thread = threading.Thread(target=recv)
    send_thread = threading.Thread(target=send)
    mouse_listener = mouse.Listener(on_move=onmove, on_click=onclick)
    key_listener = keyboard.Listener(on_press=onkey)

    recv_thread.start()
    send_thread.start()
    mouse_listener.start()
    key_listener.start()

    try:
        ffplay.wait()
    except KeyboardInterrupt:
        ffplay.terminate()

    print("stopping")
    running = False

    mouse_listener.stop()
    key_listener.stop()

    recv_thread.join()
    send_thread.join()
    mouse_listener.join()
    key_listener.join()

    conn.send(struct.pack(cmdfmt, b'X', 0, 0))
    conn.close()

    ffplay.wait()

if __name__ == '__main__':
    main()
