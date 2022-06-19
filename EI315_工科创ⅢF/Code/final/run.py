from multiprocessing import Process, Queue
from driver import Driver

import numpy as np
import cv2

import time
import sys
import os


class PID:
    def __init__(self, Kp, Ki, Kd):
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        self.Ep = 0
        self.Ei = 0
        self.Ed = 0

    def __call__(self, e):
        self.Ed = e - self.Ep
        self.Ei += e
        self.Ep = e
        return self.Kp * self.Ep + self.Ki * self.Ei + self.Kd * self.Ed


def find_line(img, display=False):
    black_i = 0
    black_v = 255
    y = 270   #250
    h = 50
    for i in range(20, 640, 20):
        v = np.mean(img[y:y+h, i - 20:i + 20])
        if v < black_v:
            black_i, black_v = i, v
    black_x1, black_x2 = black_i - 20, black_i + 20
    
    obs_i = 0
    obs_v = 255
    obs_y = 170
    obs_h = 40
    for i in range(20, 640, 20):
        v = np.mean(img[obs_y:obs_y+obs_h, i - 20:i + 20])
        if v < obs_v:
            obs_i, obs_v = i, v
    obs_flag = obs_v > 120
    obs_flag = True
    

    cross_x1 = max(black_i - 80, 0)
    cross_x2 = max(black_i - 20, 0)
    cross_x3 = min(black_i + 20, 640)
    cross_x4 = min(black_i + 80, 640)
    
    cross_v1 = np.mean(img[y:y+h, cross_x1:cross_x2])
    cross_v2 = np.mean(img[y:y+h, cross_x3:cross_x4])
    cross_flag = cross_v1 < 100 or cross_v2 < 100

    im2show = None
    if display:
        im2show = img.copy()
        try:
            im2show = cv2.rectangle(im2show, (black_x1, y), (black_x2, y+h), (0, 255, 0))
            cv2.putText(im2show, str(int(black_v)), (black_x1 + 10, y+h//2), 1, 1, (0, 255, 0), 2)
            
            im2show = cv2.rectangle(im2show, (cross_x1, y), (cross_x2, y+h), (0, 255, 0))
            cv2.putText(im2show, str(int(cross_v1)), (cross_x1 + 10, y+h//2), 1, 1, (0, 255, 0), 2)
            
            im2show = cv2.rectangle(im2show, (cross_x3, y), (cross_x4, y+h), (0, 255, 0))
            cv2.putText(im2show, str(int(cross_v2)), (cross_x3 + 10, y+h//2), 1, 1, (0, 255, 0), 2)
            
            im2show = cv2.rectangle(im2show, (obs_i - 20, obs_y), (obs_i + 20, obs_y+obs_h), (0, 255, 0))
            cv2.putText(im2show, str(int(obs_v)), (obs_i, obs_y-obs_h//2), 1, 1, (0, 255, 0), 2)
        except:
            print("show ignore")

    return (black_i, 275), cross_flag, obs_flag, im2show


def run_find_tag(img_queue, tag_queue, display=False):
    m = cv2.dnn.readNetFromTensorflow("new_model2/tag_model2.pb")
    x = np.zeros([1,3,32,32],dtype=np.float32)
    m.setInput(x)
    m.forward()
    tag_map = {0: "left", 1: "right", 2: None}
    while True:
        img = img_queue.get()
        if img is None:
            break
        img_gray = img[..., 1]
        circles = cv2.HoughCircles(img_gray, cv2.HOUGH_GRADIENT, 1, 100,
                                   param1=120, param2=20, minRadius=30, maxRadius=80)
        if circles is None:
            tag_queue.put((None, None))
            continue
        circles = np.round(circles).astype(np.int)
        tag = None
        for c in circles[0]:
            x1 = max(c[0] - c[2], 0)
            y1 = max(c[1] - c[2], 0)
            x2 = min(c[0] + c[2], 640)
            y2 = min(c[1] + c[2], 480)
            roi = cv2.resize(img[y1:y2, x1:x2], (32, 32))
            roi_x = (roi[np.newaxis, ..., ::-1] / 128. - 1.).astype(np.float32)
            m.setInput(np.transpose(roi_x, [0, 3, 1, 2]))
            y = m.forward()[0]
            print(y)
            y = int(np.argmax(y))
            tag = (tag_map[y], (x1, y1), (x2, y2))
            if tag is not None:
                break
        if display and tag is not None:
            cv2.rectangle(img, tag[1], tag[2], (0, 255, 0))
            cv2.putText(img, tag[0], tag[1], 1, 1, (0, 255, 0))
        print(img.shape)
        tag_queue.put((None if tag is None else tag[0], img if display else None))


def run_find_obs(img_queue, obs_queue, display=False):
    m_full = cv2.dnn.readNetFromTensorflow("new_model2/detect_model_far.pb")
    m = cv2.dnn.readNetFromTensorflow("new_model/obs_model.pb")
    
    while True:
        img = img_queue.get()
        if img is None:
            break
        img = cv2.resize(img, (-1, -1), fx=0.5, fy=0.5)

        img_small = cv2.resize(img, (32, 32))[np.newaxis, ..., ::-1]
        img_small = (np.transpose(img_small, [0, 3, 1, 2]).astype(np.float32) - 127) / 128.
        m_full.setInput(img_small)
        y = m_full.forward()[0]
        has_obs = y[0] > 0.99
        if not has_obs:
            obs_queue.put((None, None))
            continue

        img_gray = img[..., 2]
        circles = cv2.HoughCircles(img_gray, cv2.HOUGH_GRADIENT, 1, 200,
                                   param1=100, param2=40, minRadius=20, maxRadius=100)
        if circles is None:
            obs_queue.put(("unknown", None))
            continue
        circles = np.round(circles).astype(np.int)
        obs = None
        for c in circles[0]:
            if c[2] < 40:
                continue
            x1 = max(c[0] - c[2], 0)
            y1 = max(c[1] - c[2], 0)
            x2 = min(c[0] + c[2], 640)
            y2 = min(c[1] + c[2], 480)
            roi = cv2.resize(img[y1:y2, x1:x2], (32, 32))
            roi_x = (roi[np.newaxis, ..., ::-1] / 128. - 1.).astype(np.float32)
            m.setInput(np.transpose(roi_x, [0, 3, 1, 2]))
            output = m.forward()[0]
            y = int(np.argmax(output))
            if output[y] < 0.8:
                y = 1
            if y == 0:
                left_roi = img[y1:y2, x1 - 2 * c[2]:x2 - 2 * c[2]]
                right_roi = img[y1:y2, x1 + 2 * c[2]:x2 + 2 * c[2]]
                cv2.rectangle(img, (x1 - 2 * c[2], y1), (x2 - 2 * c[2], y2), (0, 255, 0))
                cv2.rectangle(img, (x1 + 2 * c[2], y1), (x2 + 2 * c[2], y2), (0, 255, 0))
                left_var = np.var(left_roi)
                right_var = np.var(right_roi)
                if left_var < right_var / 1.5:
                    obs = ("left", (x1, y1), (x2, y2))
                elif right_var < left_var / 1.5:
                    obs = ("right", (x1, y1), (x2, y2))
            if obs is not None:
                break
        if display and obs is not None:
            cv2.rectangle(img, obs[1], obs[2], (0, 255, 0))
            cv2.putText(img, obs[0], obs[1], 1, 1, (0, 255, 0))
        obs_queue.put(("unknown" if obs is None else obs[0], img if display else None))


def line_patrol(driver, run=True, display=False, record=False, with_tag=True, with_obs=True, tag_num=1e10, obs_num=1e10):
    tag_img_queue = Queue()
    tag_rst_queue = Queue()
    obs_img_queue = Queue()
    obs_rst_queue = Queue()
    tag_proc = Process(target=run_find_tag, args=(tag_img_queue, tag_rst_queue, display))
    tag_proc.start()
    obs_proc = Process(target=run_find_obs, args=(obs_img_queue, obs_rst_queue, display))
    obs_proc.start()
    tag_running = False
    obs_running = False

    cap = cv2.VideoCapture(1)

    video_writer = cv2.VideoWriter(f"{time.asctime(time.localtime(time.time()))}.avi",
                                   cv2.VideoWriter_fourcc(*"MJPG"), 10, (640, 480), True) if record else None

    tag_miss_cnt = 0
    obs_miss_cnt = 0

    _MAX_MISS_NUM = 10

    pid = PID(0.9, 0, 0.15)
    
    last_tag_time = 0

    obs = tag = None
    
    base_speed = 300

    t1 = time.time()
    fps = 0

    current_tag_num = 0

    current_obs_num = 0

    while True:
        t2 = time.time()
        fps += 1
        if t2 - t1 >= 1:
            print(f"fps={fps}")
            fps = 0
            t1 = t2


        ok, img = cap.read()
        # 录视频
        if video_writer is not None:
            video_writer.write(img)
        # 识别预瞄点，十字，障碍
        (ax, ay), cross_flag, obs_flag, line_img = find_line(img, display=display)
        # print(f"cross:{cross_flag}, obs:{obs_flag}")
        if display:
            cv2.imshow("line", line_img)
        # 如果有十字，异步识别转弯标志
        if with_tag and cross_flag and not tag_running and time.time() - last_tag_time > 3:
            tag_img_queue.put(img[80:340, 120:520])
            tag_running = True
        # 如果有障碍，异步识别障碍方向
        # print(f"with_obs={with_obs}, obs_flag={obs_flag}, obs_running={obs_running}")
        if with_obs and obs_flag and not obs_running:
            obs_img_queue.put(img)
            obs_running = True
        # 如果异步识别转弯标志完成
        if not tag_rst_queue.empty():
            tag_running = False
            tag, tag_img = tag_rst_queue.get()
            print(f"tag={tag}")
            if display and tag_img is not None:
                cv2.imshow("tag", tag_img)
            if tag == "left":
                # 左转标志
                # TODO: 开环左转
                if run:
                    driver.set_speed(100, 0)
                    time.sleep(1.)
                tag_miss_cnt = 0
                last_tag_time = time.time()
                current_tag_num += 1
                if current_tag_num >= tag_num:
                    with_tag = False                
                continue
            elif tag == "right":
                # 右转标志
                # TODO: 开环右转
                if run:
                    driver.set_speed(0, 100)
                    time.sleep(1.)
                tag_miss_cnt = 0
                last_tag_time = time.time()
                current_tag_num += 1
                if current_tag_num >= tag_num:
                    with_tag = False
                continue
            elif tag_miss_cnt < _MAX_MISS_NUM:
                # 没识别到，且重试次数未超过上限
                tag_img_queue.put(img[80:340, 120:520])
                tag_running = True
                tag_miss_cnt += 1
            else:
                # 没识别到，且重试次数已经超过上限
                tag_miss_cnt = 0
        # 如果异步识别障碍方向完成
        if not obs_rst_queue.empty():
            obs_running = False
            obs, obs_img = obs_rst_queue.get()
            print(f"obs={obs}")
            if display and obs_img is not None:
                cv2.imshow("obs", obs_img)
            if obs == "left":
                # 左侧障碍
                # TODO: 开环绕过左侧障碍
                if run:
                    driver.set_speed(-50, 50)
                    time.sleep(1.)
                    for i in range(5):
                        driver.set_speed(100, 100)
                        time.sleep(0.9)
                    driver.set_speed(50, -50)
                    time.sleep(1.)
                    # sys.exit(0)
                    current_obs_num += 1
                    if current_obs_num >= obs_num:
                        with_obs = False
                obs_miss_cnt = 0
                continue
            elif obs == "right":
                # 右侧障碍
                # TODO: 开环绕过右侧障碍
                if run:
                    driver.set_speed(50, -50)
                    time.sleep(1.)
                    for i in range(5):
                        driver.set_speed(100, 100)
                        time.sleep(0.9)
                    driver.set_speed(-50, 50)
                    time.sleep(1.)
                    # sys.exit(0)
                    current_obs_num += 1
                    if current_obs_num >= obs_num:
                        with_obs = False
                obs_miss_cnt = 0
                #continue
            elif obs_miss_cnt < _MAX_MISS_NUM:
                # 没识别到，且重试次数未超过上限
                obs_img_queue.put(img)
                obs_running = True
                obs_miss_cnt = 0
            else:
                # 没识别到，且重试次数已经超过上限
                obs_miss_cnt = 0
        # 正常巡线
        aim_speed = 20 if tag_running else 20 if obs == "unknown" else 140
        base_speed = base_speed * 0.7 + aim_speed * 0.3
        angle = np.arctan2(320 - ax, 480 - ay)
        diff_speed = pid(angle) * base_speed
        left_speed = base_speed + diff_speed
        right_speed = base_speed - diff_speed
        # TODO: 设置轮速
        if run:
            driver.set_speed(left_speed, right_speed)
            pass
        if display:
            cv2.waitKey(10)


if __name__ == "__main__":
    driver = Driver()
    try:
        line_patrol(driver, run=True, display=False, record=False, with_tag=True, with_obs=True, tag_num=1e10, obs_num=1)
    except KeyboardInterrupt:
        driver.set_speed(0, 0)
