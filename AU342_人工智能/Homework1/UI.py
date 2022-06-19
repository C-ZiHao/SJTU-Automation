# uncompyle6 version 3.4.0
# Python bytecode 3.5 (3350)
# Decompiled from: Python 3.5.6 |Anaconda, Inc.| (default, Aug 26 2018, 21:41:56) 
# [GCC 7.3.0]
# Embedded file name: /Users/yuegao/Downloads/code_new-1/code_new/UI.py
# Compiled at: 2018-07-30 08:20:56
# Size of source mod 2**32: 4971 bytes
import tkinter as tk

class GameBoard(tk.Frame):

    def __init__(self, parent, rows, columns, board, size=50, color1='blue', color2='red', color3='white', color4='yellow', color5='green'):
        """size is the size of a square, in pixels"""
        self.rows = rows * 2
        self.columns = columns
        self.size = size
        self.color1 = color1
        self.color2 = color2
        self.color3 = color3
        self.color4 = color4
        self.color5 = color5
        self.board = board
        self.pieces = {}
        canvas_width = columns * size
        canvas_height = rows * size
        tk.Frame.__init__(self, parent)
        self.canvas = tk.Canvas(self, borderwidth=0, highlightthickness=0, width=canvas_width, height=canvas_height, background='bisque')
        self.canvas.pack(side='top', fill='both', expand=True, padx=2, pady=2)
        self.canvas.bind('<Configure>', self.refresh)

    def refresh(self, event):
        """Redraw the board, possibly in response to window being resized"""
        xsize = 31
        ysize = 24
        self.size = min(xsize, ysize)
        self.canvas.delete('square')
        color = self.color2
        for row in range(1, self.board.size + 1):
            for col in range(1, self.board.getColNum(row) + 1):
                if str(self.board.board_status[(row, col)]) == '0':  ##0--ÎÞ
                    color = self.color3
                if str(self.board.board_status[(row, col)]) == '1':   ##1-À¶
                    color = self.color1
                if str(self.board.board_status[(row, col)]) == '2':   ##2-ºì
                    color = self.color2
                if str(self.board.board_status[(row, col)]) == '3':  #3-»Æ
                    color = self.color4
                if str(self.board.board_status[(row, col)]) == '4':   #4-ÂÌ
                    color = self.color5
                x1 = (col + (self.board.size - row)) * self.size
                y1 = row * self.size
                x2 = x1 + self.size
                y2 = y1 + self.size
                if col != 1:
                    x1 = x1 + (col - 1) * self.size
                    x2 = x2 + (col - 1) * self.size
                self.canvas.create_oval(x1, y1, x2, y2, outline='black', fill=color, tags='square')

        for row in range(self.board.size + 1, self.board.size * 2):
            for col in range(1, self.board.getColNum(row) + 1):
                if str(self.board.board_status[(row, col)]) == '0':
                    color = self.color3
                if str(self.board.board_status[(row, col)]) == '1':
                    color = self.color1
                if str(self.board.board_status[(row, col)]) == '2':
                    color = self.color2
                if str(self.board.board_status[(row, col)]) == '3':
                    color = self.color4
                if str(self.board.board_status[(row, col)]) == '4':
                    color = self.color5
                x1 = (col + (row - self.board.size)) * self.size
                y1 = row * self.size
                x2 = x1 + self.size
                y2 = y1 + self.size
                if col != 1:
                    x1 = x1 + (col - 1) * self.size
                    x2 = x2 + (col - 1) * self.size
                self.canvas.create_oval(x1, y1, x2, y2, outline='black', fill=color, tags='square')

    def draw(self):
        """Redraw the board"""
        xsize = 31
        ysize = 24
        #self.board.printBoard()
        self.size = min(xsize, ysize)
        self.canvas.delete('square')
        color = self.color2
        for row in range(1, self.board.size + 1):
            for col in range(1, self.board.getColNum(row) + 1):
                if str(self.board.board_status[(row, col)]) == '0':
                    color = self.color3
                if str(self.board.board_status[(row, col)]) == '1':
                    color = self.color1
                if str(self.board.board_status[(row, col)]) == '2':
                    color = self.color2
                if str(self.board.board_status[(row, col)]) == '3':
                    color = self.color4
                if str(self.board.board_status[(row, col)]) == '4':
                    color = self.color5
                x1 = (col + (self.board.size - row)) * self.size
                y1 = row * self.size
                x2 = x1 + self.size
                y2 = y1 + self.size
                if col != 1:
                    x1 = x1 + (col - 1) * self.size
                    x2 = x2 + (col - 1) * self.size
                self.canvas.create_oval(x1, y1, x2, y2, outline='black', fill=color, tags='square')

        for row in range(self.board.size + 1, self.board.size * 2):
            for col in range(1, self.board.getColNum(row) + 1):
                if str(self.board.board_status[(row, col)]) == '0':
                    color = self.color3
                if str(self.board.board_status[(row, col)]) == '1':
                    color = self.color1
                if str(self.board.board_status[(row, col)]) == '2':
                    color = self.color2
                if str(self.board.board_status[(row, col)]) == '3':
                    color = self.color4
                if str(self.board.board_status[(row, col)]) == '4':
                    color = self.color5
                x1 = (col + (row - self.board.size)) * self.size
                y1 = row * self.size
                x2 = x1 + self.size
                y2 = y1 + self.size
                if col != 1:
                    x1 = x1 + (col - 1) * self.size
                    x2 = x2 + (col - 1) * self.size
                self.canvas.create_oval(x1, y1, x2, y2, outline='black', fill=color, tags='square')
# okay decompiling UI.pyc
