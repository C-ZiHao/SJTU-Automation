#!/usr/bin/env python
# -*- coding: utf-8 -*-

import wave
from pyaudio import PyAudio,paInt16

from aip import AipSpeech
import os
import numpy as np

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

		
def stt(filename):
    result = client.asr(get_file_content(filename), 'wav', 16000, {'dev_pid': 1537,})
    #print result

    if result['err_msg']=='success.':
        word = result['result'][0].encode('utf-8')
        if word!='':
            if word[len(word)-3:len(word)]==',':
                print word[0:len(word)-3]
                with open('demo.txt','w') as f:
                    f.write(word[0:len(word)-3])
                f.close()
            else:
#                print (word.decode('utf-8').encode('gbk'))
                print word
                with open('demo.txt','w') as f:
                    f.write(word)
                f.close()
    
    
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
def record():
    pa = PyAudio()     
    stream = pa.open(format = paInt16, channels=1,rate=16000, input=True, frames_per_buffer=NUM_SAMPLES)
    audioBuffer = []   # 录音缓存数组
    count = 0

    # 录制40s语音
    while True:
        string_audio_data = stream.read(NUM_SAMPLES) #一次性录音采样字节的大小
        audioBuffer.append(string_audio_data)
        
        audio_data = np.fromstring(string_audio_data, dtype=np.short)
        temp = np.max(audio_data)
        if(temp > 550):
            break_flag = 5
            while(break_flag > 0):
                print("recoding",temp)
                string_audio_data = stream.read(NUM_SAMPLES) #一次性录音采样字节的大小
                audioBuffer.append(string_audio_data)
                
                audio_data = np.fromstring(string_audio_data, dtype=np.short)
                temp = np.max(audio_data)
                
                if(temp < 200):
                    break_flag = break_flag - 1
            
                count +=1
            save_wave_file('audio.wav',audioBuffer)
            stt('audio.wav')
        else:
            audioBuffer = []
        print('.'),  #加逗号不换行输出
	
        
        
    stream.close()

# 播放后缀为wav的音频文件
def play():

    wf = wave.open(r"audio.wav",'rb') # 打开audio.wav
    p = PyAudio()                     # 实例化pyaudio
	
    # 打开流
    stream = p.open( format=p.get_format_from_width(wf.getsampwidth()),
                     channels=wf.getnchannels(),
                     rate=wf.getframerate(),
                     output=True)
    # 播放音频
    while True:
        
        data = wf.readframes(chunk)
        if data == "":break
        stream.write(data)
	
    # 释放IO
    stream.stop_stream()
    stream.close()
    p.terminate()

# main函数 录制40s音频并播放
if __name__ == '__main__':
    print('record ready...')
    record()
    print('record over!') 
    play()
    
    
    


