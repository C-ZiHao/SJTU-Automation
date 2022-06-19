#!/usr/bin/env python
# -*- coding: utf-8 -*-

import requests
from requests.api import request

from playsound import playsound

from aip import AipSpeech
import os
import wave

import time
import vlc
from pyaudio import PyAudio,paInt16
import numpy as np

from multiprocessing import Process, Queue
import sys
import requests

from driver import driver
car = driver()

import json
import urllib


APP_ID = '24082147'
API_KEY = 'p2On7jfr9I7KbMhNEzB6vyqY'
SECRET_KEY = '9yDZz2Riw1YSGxE3Ixw6IGLy2peo6Gzx'


NUM_SAMPLES = 2000
TIME = 2
chunk = 1024


api_url = "http://openapi.tuling123.com/openapi/api/v2"

def tuling_replay(text_input):
    req = {
        "perception":
        {
            "inputText":
            {
                "text": text_input
            },

            "selfInfo":
            {
                "location":
                {
                    "city": "上海",
                    "province": "上海",
                    "street": "东川路"
                }
            }
        },

        "userInfo": 
        {
            "apiKey": "7fee2d93e6b74f8d96197d699f58d31a",
            "userId": "OnlyUseAlphabet"
        }
    }

    req = json.dumps(req).encode('utf8')

    http_post = urllib.request.Request(api_url, data=req, headers={'content-type': 'application/json'})
    response = urllib.request.urlopen(http_post)
    response_str = response.read().decode('utf8')
    response_dic = json.loads(response_str)

    intent_code = response_dic['intent']['code']
    results_text = response_dic['results'][0]['values']['text']
    return results_text

# 新建一个AipSpeech
client = AipSpeech(APP_ID, API_KEY, SECRET_KEY)

def get_answer(text):
    data = {
        "key": "275115c5c241ff5521eceecb484ade99",
        "question": text,
    }

    url = 'http://api.tianapi.com/txapi/robot/index'  # API接口
    response = requests.post(url=url, data=data) 
    response.encoding = 'utf-8'
    result = response.json()
    if(result['msg'] == "success"):
        return result
    else:
        return None

def text_reply(msg):
    result = get_answer(msg)  # 收到信息时，调用思知机器人并获取机器人的回复信息
    try:
        answer = result['newslist'][0]['reply']
        if len(answer) != 0:
            word = answer
            return word
    except:
        return None

def word_to_voice(word,filename):
    # 语音合成
    #print(word)
    result  = client.synthesis(word ,'zh',1, {
        'vol': 50,'per':4,
    })
    
    #print(result)
    if not isinstance(result, dict):
        
        with open(filename, 'wb') as f:
            f.write(result)
        #f.close()
        #print("convert finished")
    
    
# 播放后缀为wav的音频文件
def play(filename):

    wf = wave.open(filename,'rb') # 打开audio.wav
    p = PyAudio()                     # 实例化pyaudio
    # 打开流
    stream = p.open( format=p.get_format_from_width(wf.getsampwidth()),
                     channels=wf.getnchannels(),
                     rate=wf.getframerate(),
                     output=True)
    # 播放音频
    while True:
        data = wf.readframes(chunk)
        #print(data)
        if data == " ":break
        stream.write(data)

    #print(stream)
    stream.stop_stream()
    stream.close()
    p.terminate()
    print("finished")
def read_wave_file(filename):
    fp = wave.open(filename,'rb')
    nf = fp.getnframes()    
    #print('sampwidth:',fp.getsampwidth())  
    #print('framerate:',fp.getframerate())
    #print('channels:',fp.getnchannels())
    f_len = nf*2
    audio_data = fp.readframes(nf)

def voice_display(filename,display_queue):
    #pygame.mixer.init()
    #pygame.mixer.music.load(filename)
    #pygame.mixer.music.play()
    #os.system('vlc '+filename)
    #os.system("ffplay "+filename)
#     playsound(filename)
    p = vlc.MediaPlayer(filename)
    p.audio_set_volume(60)
    p.play()
    while(p.is_playing()==0):
        #print(p.is_playing())
        time.sleep(0.3)
    while(p.is_playing()):
        #print(p.is_playing())
        time.sleep(0.3)
    display_queue.put(True)
    


def voice_to_word(filename):
    result = client.asr(get_file_content(filename), 'wav', 16000, {'dev_pid': 1537,})
    #print(result)
    if result['err_msg']=='success.':
        word = result['result'][0]
        return word
    return None


def read_wave_file(filename):
    fp = wave.open(filename,'rb')
    nf = fp.getnframes()    
    #print('sampwidth:',fp.getsampwidth())  
    #print('framerate:',fp.getframerate())
    #print('channels:',fp.getnchannels())
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

def get_file_content(filePath):
    with open(filePath, 'rb') as fp:
        return fp.read()
    
#recode audio to audio.wav
def record(record_queue,display_queue):
    pa = PyAudio()     
    stream = pa.open(format = paInt16, channels=1,rate=16000, input=True, frames_per_buffer=NUM_SAMPLES)
    audioBuffer = []   # 录音缓存数组
    count = 0

    # 录制语音
    while True:
        audioBuffer = []
        string_audio_data = stream.read(NUM_SAMPLES) #一次性录音采样字节的大小
        audioBuffer.append(string_audio_data)
        
        audio_data = np.fromstring(string_audio_data, dtype=np.short)
        temp = np.max(audio_data)
        if(temp > 1200):
            break_flag = 5
            while(break_flag > 0):
                string_audio_data = stream.read(NUM_SAMPLES) #一次性录音采样字节的大小
                audioBuffer.append(string_audio_data)
                
                audio_data = np.fromstring(string_audio_data, dtype=np.short)
                temp = np.max(audio_data)
                
                if(temp < 400):
                    break_flag = break_flag - 1
            
            #print("record_finish")
            save_wave_file('audio.wav',audioBuffer)
            record_queue.put(True)
            stream.close()
            
            while True:
                time.sleep(0.3)
                if not display_queue.empty():
                    flag = display_queue.get()
                    stream = pa.open(format = paInt16, channels=1,rate=16000, input=True, frames_per_buffer=NUM_SAMPLES)
                    break
        #record_queue.put(None) 
    stream.close()


def answer_get(record_queue,display_queue):
    
    while True:
        flag = record_queue.get()
        if flag == True:
            word = voice_to_word('audio.wav')
            print("识别到语音: ",word)
            if(word != None):
                #answer = text_reply(word)
                
                answer = tuling_replay(word)
                print("回答:", answer)
                
                word_to_voice(answer , './voice/answer.wav')
                #print("ok")
                time.sleep(0.2)
                voice_display('./voice/answer.wav',display_queue)
                #display_queue.put(True)

'''
def sound_play(display_queue):
    while True:
        flag = display_queue.get()
        if flag == True:
            voice_display('./voice/answer.wav')
'''
if __name__ == '__main__':

    record_queue = Queue()

    request_queue  = Queue()

    display_queue =  Queue()
    record_proc = Process(target=record, args=(record_queue, display_queue))
    record_proc.start()

    request_proc = Process(target=answer_get, args=(record_queue,display_queue))
    request_proc.start()

    #display_proc = Process(target=sound_play, args=(display_queue,))
    #display_proc.start()
    
