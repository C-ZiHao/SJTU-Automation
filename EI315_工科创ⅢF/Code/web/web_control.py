#!/usr/bin/env python
# -*- coding: utf-8 -*-


import cv2
import time
from driver import driver

msg = """
---------------------------
Moving around:
       w
  a    s    d
w/s : increase/decrease base speeds by 10
a/d : increase/decrease diff speed by 10
space key : force stop

C to quitM
"""


def constrain(input,lower,upper):
    if input>upper:
        return upper
    elif input<lower:
        return lower
    else:
        return input



def main(moving_queue, speed_queue):
    car = driver()

    cap1 = cv2.VideoCapture(1)

    base_speed = 0
    diff_speed = 0

    k = "space"
    speed = [0,0]

    while(1):
        if not moving_queue.empty():
            k = moving_queue.get()
        else:
            k = None
            time.sleep(0.1)
        print("get: ", k)
        # if not speed_queue.empty():


        _, img = cap1.read()
        # cv2.imshow("cap1", img)
        # k = cv2.waitKey(10)

        if(k == "w"):
            base_speed = base_speed + 10
        elif(k=="s"):
            base_speed = base_speed - 10
        elif(k=="a"):
            diff_speed = diff_speed + 10
        elif(k=="d"):
            diff_speed = diff_speed - 10
        elif(k=="space"):
            last_speed = 0
            base_speed = 0
            diff_speed = 0
        elif(k=="c"):
            break
        
        base_speed = constrain(base_speed, -200, 200) 
        diff_speed = constrain(diff_speed, -200, 200)
        
        speed[0] =  base_speed + diff_speed
        speed[1] =  base_speed - diff_speed

        print(f"base={base_speed}, diff={diff_speed}, s0={speed[0]}, s1={speed[1]}")

        car.set_speed(speed[0], speed[1])
        
        
