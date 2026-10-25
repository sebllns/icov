import socket

UDP_IP = "127.0.0.1"
UDP_PORT = 5387

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)


def send_command(code):
    #code = bytes(str(ord('q')), "utf-8")
    print("message: %s" % code)
    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    sock.sendto(code, (UDP_IP, UDP_PORT))


def send_command_number(x):
    send_command(bytes(str(x), "utf-8"))

def send_command_char(c):
    send_command_number(ord(c))

import tkinter as tk
from tkinter import *
from tkinter import ttk

# root window
root = tk.Tk()
root.geometry('320x600')
root.resizable(False, False)
root.title('ICOV REMOTE CONTROLLER')


button = ttk.Button(
    root,
    text='←',
    command=lambda: send_command_char('q')
)

button.pack(
    side = LEFT
)

button = ttk.Button(
    root,
    text='→',
    command=lambda: send_command_char('d')
)

button.pack(
    side = RIGHT
)

button = ttk.Button(
    root,
    text='Switch play / pause',
    command=lambda: send_command_char('p')
)

button.pack(
    side = TOP,
    expand=True
)

button = ttk.Button(
    root,
    text='Switch viewport / full view',
    command=lambda: send_command_char('w')
)

button.pack(
    side = TOP,
    expand=True
)

button = ttk.Button(
    root,
    text='Switch source / decoded view',
    command=lambda: send_command_char('x')
)

button.pack(
    side = TOP,
    expand=True
)

button = ttk.Button(
    root,
    text='Show video',
    command=lambda: send_command_number(-66)
)

button.pack(
    side = TOP,
    expand=True
)

button = ttk.Button(
    root,
    text='Show block rates',
    command=lambda: send_command_number(-65)
)

button.pack(
    side = TOP,
    expand=True
)

button = ttk.Button(
    root,
    text='Show viewport rate',
    command=lambda: send_command_number(-64)
)

button.pack(
    side = TOP,
    expand=True
)

button = ttk.Button(
    root,
    text='Show predictions',
    command=lambda: send_command_number(-63)
)

button.pack(
    side = TOP,
    expand=True
)

# exit button
button = ttk.Button(
    root,
    text='Close this app',
    command=lambda: root.quit()
)

button.pack(
    ipadx=5,
    ipady=5,
    expand=True
)

root.mainloop()