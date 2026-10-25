from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure
from tkinter import *
import socket
import matplotlib
import matplotlib.pyplot as plt

matplotlib.use("TkAgg")

UDP_IP = "127.0.0.1"
UDP_PORT_SEND = 5387
UDP_PORT_RECEIVE = 5005


def send_command(code):
    print("command: %s" % code)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Internet  # UDP
    sock.sendto(code, (UDP_IP, UDP_PORT_SEND))


def send_command_number(x):
    send_command(bytes(str(x), "utf-8"))


def send_command_char(c):
    send_command_number(ord(c))


class DynamicUpdate:
    # Suppose we know the x range
    min_x = 0
    max_x = 10

    def on_launch(self):
        # set up udp

        self.sock = socket.socket(
            socket.AF_INET, socket.SOCK_DGRAM)  # Internet  # UDP
        self.sock.setblocking(0)
        self.sock.bind((UDP_IP, UDP_PORT_RECEIVE))

        # Set up plot
        self.figure, (self.ax, self.ax_psnr) = plt.subplots(1, 2)
        (self.lines,) = self.ax.plot(
            [],
            [],
            "r+--",
        )
        # Autoscale on unknown axis and known lims on the other
        self.ax.set_autoscaley_on(True)
        # Other stuff
        self.ax.grid()

        (self.lines_psnr,) = self.ax_psnr.plot(
            [],
            [],
            "b+--",
        )
        self.ax_psnr.set_autoscaley_on(True)
        # Other stuff
        self.ax_psnr.grid()
        ...

    def on_running(self, xdata, ydata, ydata2):
        # Update data (with the new _and_ the old points)
        self.lines.set_xdata(xdata)
        self.lines.set_ydata(ydata)
        # Need both of these in order to rescale
        self.ax.relim()
        self.ax.autoscale_view()

        # Update data (with the new _and_ the old points)
        self.lines_psnr.set_xdata(xdata)
        self.lines_psnr.set_ydata(ydata2)
        # Need both of these in order to rescale
        self.ax_psnr.relim()
        self.ax_psnr.autoscale_view()

        # We need to draw *and* flush
        self.figure.canvas.draw()
        self.figure.canvas.flush_events()

    # Example
    def __call__(self):
        import numpy as np
        import time

        self.on_launch()
        xdata = []
        ydata = []
        ydata2 = []
        i = 0
        while True:
            try:
                data, addr = self.sock.recvfrom(1024)
            except socket.error:
                pass
            else:
                try:
                    data_str = data.decode("utf-8")
                    print("receive " + data_str)
                    data_f = [float(x) for x in (data_str.split())]
                    print(data_f)
                    # #xdata.append(dt.datetime.now().strftime('%H:%M:%S.%f'))
                    i += 1
                    xdata.append(i)
                    ydata.append(data_f[0])
                    ydata2.append(data_f[1])
                    xdata = xdata[-PLOT_POINT_COUNT:]
                    ydata = ydata[-PLOT_POINT_COUNT:]
                    ydata2 = ydata2[-PLOT_POINT_COUNT:]
                except Exception as error:
                    # handle the exception
                    print("An exception occurred:", error)

            self.on_running(xdata, ydata, ydata2)
            # time.sleep(0.5)
        return xdata, ydata


if __name__ == "__main__":
    print("UDP target IP: %s" % UDP_IP)
    print("UDP target port: %s" % UDP_PORT_SEND)
    print("UDP receive port: %s" % UDP_PORT_RECEIVE)

    root = Tk()  # create root window
    root.title("ICOV remote controller")
    root.config(bg="gray")
    root.geometry("900x600")
    root.resizable(False, False)

    # Create Frame widget
    left_frame = Frame(root, width=200)

    left_frame.pack(side=LEFT, fill=Y, padx=10, pady=5)

    # Create label above the tool_bar
    Label(left_frame, text="Commands").pack(side=TOP, fill=X, padx=5, pady=5)

    # Create frame within left_frame
    tool_bar = Frame(left_frame, width=200, height=185)
    tool_bar.pack(side=TOP, fill=X, padx=5, pady=5)

    Button(tool_bar, text="↑", command=lambda: send_command_char("z")).grid(
        row=0, column=1
    )
    Button(tool_bar, text="←", command=lambda: send_command_char("q")).grid(
        row=1, column=0
    )
    Button(tool_bar, text="↓", command=lambda: send_command_char("d")).grid(
        row=1, column=1
    )
    Button(tool_bar, text="→", command=lambda: send_command_char("s")).grid(
        row=1, column=2
    )

    Button(left_frame, text="⏯︎", command=lambda: send_command_char("p")).pack(side=TOP, fill=X, padx=5, pady=5)

    Button(left_frame, text="Decoder \n ON/OFF", command=lambda: send_command_char("c")).pack(side=TOP, fill=X, padx=5, pady=5)

    Button(left_frame, text="Switch \n ERP view", command=lambda: send_command_char("w")).pack(side=TOP, fill=X, padx=5, pady=5)

    Button(left_frame, text="Switch \n Source view", command=lambda: send_command_char("v")).pack(side=TOP, fill=X, padx=5, pady=5)

    Button(left_frame, text="Switch \n Reco view", command=lambda: send_command_char("x")).pack(side=TOP, fill=X, padx=5, pady=5)

    plt.ion()
    d = DynamicUpdate()
    d()

    root.mainloop()


# from tkinter import *

# root = Tk()
# frame = Frame(root)
# root.rowconfigure(0, weight=1)
# root.columnconfigure(0, weight=1)
# frame.grid(row=0, column=0, sticky="news")
# grid = Frame(frame)
# grid.grid(sticky="news", column=0, row=7, columnspan=2)
# frame.rowconfigure(7, weight=1)
# frame.columnconfigure(0, weight=1)

# #example values
# for x in range(10):
#     for y in range(5):
#         btn = Button(frame)
#         btn.grid(column=x, row=y, sticky="news")

# frame.columnconfigure(tuple(range(10)), weight=1)
# frame.rowconfigure(tuple(range(5)), weight=1)

# root.mainloop()


# import threading
# interval = 15

# def myPeriodicFunction():
#     print "This loops on a timer every %d seconds" % interval

# def startTimer():
#     threading.Timer(interval, startTimer).start()
#     myPeriodicFunction()
