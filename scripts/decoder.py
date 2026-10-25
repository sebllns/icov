import tkinter as tk
import matplotlib
from tkinter import *


matplotlib.use('TkAgg')

from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import (
    FigureCanvasTkAgg,
    NavigationToolbar2Tk
)


class App(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title('ICOV Monitoring')

        # prepare data
        data = {
            'Python': 11.27,
            'C': 11.16,
            'Java': 10.46,
            'C++': 7.5,
            'C#': 5.26
        }
        languages = data.keys()
        popularity = data.values()

        # create a figure
        figure = Figure(figsize=(6, 4), dpi=100)

        # create FigureCanvasTkAgg object
        figure_canvas = FigureCanvasTkAgg(figure, self)

        # create the toolbar
        NavigationToolbar2Tk(figure_canvas, self)

        # create axes
        axes = figure.add_subplot()

        # create the barchart
        axes.bar(languages, popularity)
        axes.set_title('Top 5 Programming Languages')
        axes.set_ylabel('Popularity')

        figure_canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        CheckVar1 = IntVar()
        CheckVar2 = IntVar()
        C1 = Checkbutton(self, text = "Music", variable = CheckVar1, \
        onvalue = 1, offvalue = 0, height=5, \
        width = 20, )
        C2 = Checkbutton(self, text = "Video", variable = CheckVar2, \
        onvalue = 1, offvalue = 0, height=5, \
        width = 20)
        C1.pack(side=tk.LEFT)
        C2.pack()


if __name__ == '__main__':
    app = App()
    app.mainloop()