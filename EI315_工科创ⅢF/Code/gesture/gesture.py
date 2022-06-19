


import os
import cv2
import numpy as np
import time
import random
import vlc



capture=cv2.VideoCapture(1)

m = cv2.dnn.readNetFromTensorflow("./models/new_model.pb")
x = np.zeros([1,3,32,32],dtype=np.float32)
m.setInput(x)
m.forward()
tag_map = {0: "neg", 1: "stone", 2: "scissor", 3: "paper"}

count = [0 , 0 , 0]


def test(input):
    k = random.randint(1,3)
    if((input == 1 and k == 2) or (input == 2 and k == 3 ) or (input== 3 and k==1)):
        result = 'fail'
    elif(input == k):
        result = 'tie'
    else:
        result = 'win'
        
    return [k,result]

def voice_display(filename):
    p = vlc.MediaPlayer(filename)
    p.audio_set_volume(100)
    p.play()
    time.sleep(4)
    p.stop()
    
while(True):
    ref,frame=capture.read()
    img = frame    
    roi = cv2.resize(img, (32, 32))
    roi_x = (roi[np.newaxis, ..., ::-1] / 128. - 1.).astype(np.float32)
    m.setInput(np.transpose(roi_x, [0, 3, 1, 2]))
    y = m.forward()[0]
    
    tag_num = int(np.argmax(y))
    tag = tag_map[tag_num]
    
    
    score = np.max(y)
   
    if(score>0.75 and tag_num > 0):
        #print(tag,y)
        cv2.putText(img, tag, (200, 400), cv2.FONT_HERSHEY_SIMPLEX, 3,255, 8)
        count[int(np.argmax(y)) - 1] = count[int(np.argmax(y)) - 1] + 1
    else:
        for i in range(0,3):
            count = [0 , 0 , 0]
    for i in range(1,4):
        if(count[i-1] > 20):
            print("recognise : " , tag_map[i])
            [k , result] = test(i)
            print(tag_map[k],result)
            
            sound = './voice/'+tag_map[k]+'_'+result+'.wav'
            #print(sound)
            try:
                voice_display(sound)
                count = [0 , 0 , 0]
            except:
                print("play_wrong")
            
   
            
    cv2.imshow("img", img)
    cv2.waitKey(20)
            
    


