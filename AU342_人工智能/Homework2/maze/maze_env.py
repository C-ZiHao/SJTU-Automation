import numpy as np
np.random.seed(1)
import tkinter as tk
import time


UNIT = 40
MAZE_H = 6
MAZE_W = 6


class Maze(tk.Tk, object):
    def __init__(self):
        super(Maze, self).__init__()
        self.action_space = ['u', 'd', 'l', 'r']
        self.n_actions = len(self.action_space)
        self.title('maze')
        self.geometry('{0}x{1}'.format(MAZE_H * UNIT, MAZE_H * UNIT))
        self._build_maze()
        self.bonusFlag = False
        self.eating = True

    def _build_maze(self):
        self.canvas = tk.Canvas(self, bg='white',
                           height=MAZE_H * UNIT,
                           width=MAZE_W * UNIT)

        for c in range(0, MAZE_W * UNIT, UNIT):
            x0, y0, x1, y1 = c, 0, c, MAZE_H * UNIT
            self.canvas.create_line(x0, y0, x1, y1)
        for r in range(0, MAZE_H * UNIT, UNIT):
            x0, y0, x1, y1 = 0, r, MAZE_H * UNIT, r
            self.canvas.create_line(x0, y0, x1, y1)

        origin = np.array([20, 20])

        # hell1
        hell1_center = origin + np.array([UNIT * 1, UNIT * 0])
        self.hell1 = self.canvas.create_rectangle(
            hell1_center[0] - 15, hell1_center[1] - 15,
            hell1_center[0] + 15, hell1_center[1] + 15,
            fill='black')
        # hell2
        hell2_center = origin + np.array([UNIT * 2, UNIT * 1])
        self.hell2 = self.canvas.create_rectangle(
            hell2_center[0] - 15, hell2_center[1] - 15,
            hell2_center[0] + 15, hell2_center[1] + 15,
            fill='black')
        # hell3
        hell3_center = origin + np.array([UNIT * 5, UNIT * 1])
        self.hell3 = self.canvas.create_rectangle(
            hell3_center[0] - 15, hell3_center[1] - 15,
            hell3_center[0] + 15, hell3_center[1] + 15,
            fill='black')
        # hell4
        hell4_center = origin + np.array([UNIT * 1, UNIT * 4])
        self.hell4 = self.canvas.create_rectangle(
            hell4_center[0] - 15, hell4_center[1] - 15,
            hell4_center[0] + 15, hell4_center[1] + 15,
            fill='black')
        # hell5
        hell5_center = origin + np.array([UNIT * 2, UNIT * 4])
        self.hell5 = self.canvas.create_rectangle(
            hell5_center[0] - 15, hell5_center[1] - 15,
            hell5_center[0] + 15, hell5_center[1] + 15,
            fill='black')

        oval_center = origin + np.array([UNIT * 5, UNIT * 0])
        self.oval = self.canvas.create_oval(
            oval_center[0] - 15, oval_center[1] - 15,
            oval_center[0] + 15, oval_center[1] + 15,
            fill='green')

        self.rect = self.canvas.create_rectangle(
            origin[0] - 15, origin[1] - 15,
            origin[0] + 15, origin[1] + 15,
            fill='red')

        # create bonus
        bonus_center = origin + np.array([UNIT * 1, UNIT * 5])
        self.bonus = self.canvas.create_polygon(
            [bonus_center[0]+15, bonus_center[1],
            bonus_center[0], bonus_center[1]-15,
            bonus_center[0]-15, bonus_center[1],
            bonus_center[0], bonus_center[1]+15],
            fill='#CDCD00')
        self.bonus_location = self.canvas.create_polygon(
            bonus_center[0] - 15, bonus_center[1] - 15,
            bonus_center[0] + 15, bonus_center[1] + 15,
            fill='white')

        self.canvas.pack()

    def reset(self):
        self.canvas.delete(self.rect)
        self.canvas.delete(self.bonus)
        origin = np.array([20, 20])
        self.rect = self.canvas.create_rectangle(
            origin[0] - 15, origin[1] - 15,
            origin[0] + 15, origin[1] + 15,
            fill='red')
        bonus_center = origin + np.array([UNIT * 1, UNIT * 5])
        self.bonus = self.canvas.create_polygon(
            [bonus_center[0]+15, bonus_center[1],
            bonus_center[0], bonus_center[1]-15,
            bonus_center[0]-15, bonus_center[1],
            bonus_center[0], bonus_center[1]+15],
            fill='#CDCD00')
        self.bonusFlag = False
        self.eating = True
        return self.canvas.coords(self.rect) + [self.bonusFlag]

    def step(self, action):
        s = self.canvas.coords(self.rect)
        base_action = np.array([0, 0])
        if action == 0:   # up
            if s[1] > UNIT:
                base_action[1] -= UNIT
        elif action == 1:   # down
            if s[1] < (MAZE_H - 1) * UNIT:
                base_action[1] += UNIT
        elif action == 2:   # right
            if s[0] < (MAZE_W - 1) * UNIT:
                base_action[0] += UNIT
        elif action == 3:   # left
            if s[0] > UNIT:
                base_action[0] -= UNIT

        self.canvas.move(self.rect, base_action[0], base_action[1])  # move agent

        s_ = self.canvas.coords(self.rect)  # next state

        if s_ == self.canvas.coords(self.oval):
            reward = 1
            done = True
        elif (s_ == self.canvas.coords(self.bonus_location)) and (self.bonusFlag == False):
            self.bonusFlag = True
            reward = 3
            done = False
        elif s_ in [self.canvas.coords(self.hell1), self.canvas.coords(self.hell2), self.canvas.coords(self.hell3), self.canvas.coords(self.hell4), 
        self.canvas.coords(self.hell5)]:#, self.canvas.coords(self.hell6), self.canvas.coords(self.hell7)]:
            reward = -1
            done = True
        else:
            reward = 0
            done = False
        s_.append(self.bonusFlag)

        return s_, reward, done

    def render(self):
        self.update()
        if self.bonusFlag and self.eating:
            self.eating = False
            time.sleep(0.5)
            self.canvas.delete(self.bonus)


