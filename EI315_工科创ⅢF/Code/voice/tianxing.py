#!/usr/bin/env python
# -*- coding: utf-8 -*-

import requests

from playsound import playsound

from aip import AipSpeech
import os
import wave

import time
import vlc
from pyaudio import PyAudio,paInt16
#import pygame

''' 你的APPID AK SK  参数在申请的百度云语音服务的控制台查看'''
APP_ID = '11472625'
API_KEY = 'NYIvd23qqGAZ1ZPpVpCthENs'
SECRET_KEY = 'DcQWQ9HVc0sqoD091gFxWiCP1i0oNa6u'


NUM_SAMPLES = 2000
TIME = 2
chunk = 1024

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
        print(answer)
        if len(answer) != 0:
            word = answer
            return word
    except:
        return None

def word_to_voice(word,filename):
    # 语音合成
    result  = client.synthesis(word ,'zh',1, {
        'vol': 50,'per':4,
    })
    
    if not isinstance(result, dict):
        with open(filename, 'wb') as f:
            f.write(result)
        #f.close()
        print("convert finished")
    
    
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
        print(data)
        if data == " ":break
        stream.write(data)

    print(stream)
    stream.stop_stream()
    stream.close()
    p.terminate()
    print("finished")
def read_wave_file(filename):
    fp = wave.open(filename,'rb')
    nf = fp.getnframes()    
    print('sampwidth:',fp.getsampwidth())  
    print('framerate:',fp.getframerate())
    print('channels:',fp.getnchannels())
    f_len = nf*2
    audio_data = fp.readframes(nf)

def voice_display(filename):
    #pygame.mixer.init()
    #pygame.mixer.music.load(filename)
    #pygame.mixer.music.play()
    
    #os.system('vlc '+filename)
    #os.system("ffplay "+filename)
    
#     playsound(filename)

    p = vlc.MediaPlayer(filename)
    p.audio_set_volume(100)
    p.play()

#read_wave_file('audio.wav')

def main():

    answer = text_reply("你好")
    print(answer)
    word_to_voice(answer , './voice/sh.wav')
    print("ok")
    time.sleep(1)
    voice_display('./voice/sh.wav')
    time.sleep(1)

#word_to_voice("我输了，我出的布" ,'./voice/paper_fail.wav')

# word_to_voice("收到命令，右转" ,'./voice/right2.mp3')
#voice_display('./voice/right.wav')

#voice_display('./voice/forward.wav')
