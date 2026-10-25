import socket
import datetime as dt
import time

UDP_IP = "127.0.0.1"
UDP_PORT = 5005
PLOT_POINT_COUNT = 20

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.setblocking(0)
sock.bind((UDP_IP, UDP_PORT))

# while True:
#     data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
#     print("received message: %s" % data)


# import datetime as dt
# from random import uniform
# import matplotlib.pyplot as plt
# import matplotlib.animation as animation

# # Create figure for plotting
# fig = plt.figure()
# ax = fig.add_subplot(1, 1, 1)
# xs = []
# ys = []


# # This function is called periodically from FuncAnimation
# def animate(i, xs, ys):

#     temp_c = uniform(18.0, 30.0) 

#     # Add x and y to lists
#     xs.append(dt.datetime.now().strftime('%H:%M:%S.%f'))
#     ys.append(temp_c)

#     # Limit x and y lists to 20 items
#     xs = xs[-20:]
#     ys = ys[-20:]

#     # Draw x and y lists
#     ax.clear()
#     ax.plot(xs, ys)

#     # Format plot
#     plt.xticks(rotation=45, ha='right')
#     plt.subplots_adjust(bottom=0.30)
#     plt.title('ICOV')
#     plt.ylabel('Y')

#     plt.show()

# # Set up plot to call animate() function periodically
# ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=1000)
# plt.show()


import matplotlib.pyplot as plt
class DynamicUpdate():
    #Suppose we know the x range
    min_x = 0
    max_x = 10

    def on_launch(self):
        #Set up plot
        self.figure, (self.ax, self.ax_psnr) = plt.subplots(1,2)
        self.lines, = self.ax.plot([],[], 'r+--',)
        #Autoscale on unknown axis and known lims on the other
        self.ax.set_autoscaley_on(True)
        #Other stuff
        self.ax.grid()

        self.lines_psnr, = self.ax_psnr.plot([],[], 'b+--',)
        self.ax_psnr.set_autoscaley_on(True)
        #Other stuff
        self.ax_psnr.grid()
        ...

    def on_running(self, xdata, ydata, ydata2):

        #Update data (with the new _and_ the old points)
        self.lines.set_xdata(xdata)
        self.lines.set_ydata(ydata)
        #Need both of these in order to rescale
        self.ax.relim()
        self.ax.autoscale_view()

        #Update data (with the new _and_ the old points)
        self.lines_psnr.set_xdata(xdata)
        self.lines_psnr.set_ydata(ydata2)
        #Need both of these in order to rescale
        self.ax_psnr.relim()
        self.ax_psnr.autoscale_view()

        #We need to draw *and* flush
        self.figure.canvas.draw()
        self.figure.canvas.flush_events()

    #Example
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
                data,addr = sock.recvfrom(1024)
            except socket.error:
                pass
            else: 
                try:
                    data_str = data.decode('utf-8')
                    print("receive " + data_str)
                    data_f =  [float(x) for x in (data_str.split())]
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
            time.sleep(0.5)
        return xdata, ydata


plt.ion()
d = DynamicUpdate()
d()




# import sys
# import socket
# from time import sleep

# s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# s.connect(('127.0.0.1',9999))
# s.settimeout(2)

# while True:
#     try:
#         msg = s.recv(4096)
#     except socket.timeout, e:
#         err = e.args[0]
#         # this next if/else is a bit redundant, but illustrates how the
#         # timeout exception is setup
#         if err == 'timed out':
#             sleep(1)
#             print 'recv timed out, retry later'
#             continue
#         else:
#             print e
#             sys.exit(1)
#     except socket.error, e:
#         # Something else happened, handle error, exit, etc.
#         print e
#         sys.exit(1)
#     else:
#         if len(msg) == 0:
#             print 'orderly shutdown on server end'
#             sys.exit(0)
#         else:
#             # got a message do something :)
