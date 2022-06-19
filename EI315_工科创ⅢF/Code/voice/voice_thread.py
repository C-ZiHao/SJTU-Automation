#!/usr/bin/env python
# -*- coding: utf-8 -*-

import wave
from pyaudio import PyAudio,paInt16

from aip import AipSpeech
import os
import numpy as np

from multiprocessing import Process, Queue
import time
import sys
import os
import vlc


import requests

from driver import driver
car = driver()


APP_ID = '24082147'
API_KEY = 'p2On7jfr9I7KbMhNEzB6vyqY'
SECRET_KEY = '9yDZz2Riw1YSGxE3Ixw6IGLy2peo6Gzx'

NUM_SAMPLES = 2000
TIME = 2
chunk = 1024


# AipSpeech
client = AipSpeech(APP_ID, API_KEY, SECRET_KEY)


def get_file_content(filePath):
    with open(filePath, 'rb') as fp:
        return fp.read()

		
def voice_to_word(filename):
    result = client.asr(get_file_content(filename), 'wav', 16000, {'dev_pid': 1537,})
    #print(result)
    if result['err_msg']=='success.':
        word = result['result'][0]
        return word
        '''
        if word!='':
            if word[len(word)-3:len(word)]==',':
                print(word[0:len(word)-3])
                with open('record_word.txt','wb+') as f:
                    f.write(word[0:len(word)-3])
                f.close()
                return word
            else:
                print(word)
                with open('record_word.txt','wb+') as f:
                    f.write(word)
                f.close()
                return word
        '''
    return None

    
def car_run(control_queue):
    base_speed = 20
    sleep_time = 0.15
    while True:
        mode = control_queue.get()
        #print(mode)
        if mode == 1:
            
            car.set_speed(base_speed , base_speed)
           
            time.sleep(sleep_time)
            voice_display('./voice/forward.wav')
        elif mode == 2:
            car.set_speed(-base_speed , -base_speed)
            voice_display('./voice/backward.wav')
            time.sleep(sleep_time)
        elif mode == 3:
            car.set_speed(base_speed , -base_speed)
            voice_display('./voice/left.wav')
            time.sleep(sleep_time)
        elif mode == 4:
            car.set_speed(-base_speed , base_speed)
            voice_display('./voice/right.wav')
            time.sleep(sleep_time)
        
def read_wave_file(filename):
    fp = wave.open(filename,'rb')
    nf = fp.getnframes()    
    print('sampwidth:',fp.getsampwidth())  
    print('framerate:',fp.getframerate())
    print('channels:',fp.getnchannels())
    f_len = nf*2
    audio_data = fp.readframes(nf)

# save wav file to filename 
def save_wave_file(filename,data):  
    wf = wave.open(filename,'wb')
    wf.setnchannels(1)      # set channels  1 or 2
    wf.setsampwidth(2)      # set sampwidth 1 or 2
    wf.setframerate(16000)  # set framerate 8K or 16K
    wf.writeframes(b"".join(data))  # write data
    wf.close()

#recode audio to audio.wav
def record(record_queue):
    pa = PyAudio()     
    stream = pa.open(format = paInt16, channels=1,rate=16000, input=True, frames_per_buffer=NUM_SAMPLES)
    audioBuffer = []   # 录音缓存数组
    count = 0

    # 录制语音
    while True:
        string_audio_data = stream.read(NUM_SAMPLES) #一次性录音采样字节的大小
        audioBuffer.append(string_audio_data)
        
        audio_data = np.fromstring(string_audio_data, dtype=np.short)
        temp = np.max(audio_data)
        if(temp > 600):
            break_flag = 5
            while(break_flag > 0):
                #print("recoding",temp)
                string_audio_data = stream.read(NUM_SAMPLES) #一次性录音采样字节的大小
                audioBuffer.append(string_audio_data)
                
                audio_data = np.fromstring(string_audio_data, dtype=np.short)
                temp = np.max(audio_data)
                
                if(temp < 400):
                    break_flag = break_flag - 1
            save_wave_file('audio.wav',audioBuffer)
            record_queue.put(True)
        else:
            audioBuffer = []
        record_queue.put(None) 
    stream.close()

def recognize_wav(record_queue, control_queue ,voice_queue):
    while True:
        flag = record_queue.get()
        #print(flag)
        if flag == True:
            word = voice_to_word('audio.wav')
            print(word)
            if(word == "前进。"):
                control_queue.put(1)
                voice_queue.put(1)
            if(word == "后退。"):
                control_queue.put(2)
                voice_queue.put(2)
            if(word == "左转。"):
                control_queue.put(3)
                voice_queue.put(3)
            if(word == "右转。"):
                control_queue.put(4)
                voice_queue.put(4)

def voice_run(voice_queue):
    while True:
        mode = voice_queue.get()
        print(mode)
        if mode == 1:
            voice_display('./voice/forward.wav')
        elif mode == 2:
            voice_display('./voice/backward.wav')
        elif mode == 3:
            voice_display('./voice/left.wav')
        elif mode == 4:
            voice_display('./voice/right.wav')

def voice_display(filename):
    #os.system('vlc '+filename)
    #os.system("ffplay "+filename)
    p = vlc.MediaPlayer(filename)
    p.audio_set_volume(100)
    p.play()
    time.sleep(3)
    p.stop()
    
if __name__ == '__main__':
    
    #voice_display('./voice/answer.wav')
    #time.sleep(20)
    record_queue = Queue()
    control_queue  = Queue()
    voice_queue =  Queue()
    record_proc = Process(target=record, args=(record_queue,))
    record_proc.start()
    
    recognize_proc = Process(target=recognize_wav, args=(record_queue,control_queue,voice_queue))
    recognize_proc.start()
    
    control_proc = Process(target=car_run, args=(control_queue,))
    control_proc.start()
    
    #voice_proc = Process(target=voice_run, args=(voice_queue,))
    #voice_proc.start()

    
    
    



