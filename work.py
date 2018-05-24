'''
Prototype Scrolling Ticker Algorithm in Python
For Google Summer of Code 2018 (Red Hen Labs)
Burhan Ul Tayyab
'''

import cv2
import numpy as np 
import sys

cap = cv2.VideoCapture(sys.argv[1])
thresh = 70
kernel = np.ones((8,5),np.uint8)
name = "img"
k = 0
t = 0


while(cap.isOpened()):
    #Reading Frames
    ret, frame = cap.read()
    #Creating a Rectangle (Bounding Box) over Scrolling Tickers
    cv2.rectangle(frame, (100, 525), (715, 550), (255,0,0), 2)
    #cv2.imshow('frame',frame)
    #Cropping the image
    crop_img = frame[525:525+25, 100:100+615]
    cv2.imshow('framex',crop_img)
    
    #Detecting Blue Areas for Ticker Stopping Point
    hsv = cv2.cvtColor(crop_img, cv2.COLOR_BGR2HSV)
    lower_red = np.array([110,50,50])
    upper_red = np.array([130,255,255])
    mask = cv2.inRange(hsv, lower_red, upper_red)
    res = cv2.bitwise_and(crop_img,crop_img, mask= mask)

    #Graying and Thresholding for finding Contours
    gray=cv2.cvtColor(res,cv2.COLOR_BGR2GRAY)
    ret,thresh1 = cv2.threshold(gray,thresh,255,cv2.THRESH_BINARY)
    closing = cv2.morphologyEx(thresh1, cv2.MORPH_CLOSE, kernel)

    t = t + 1
    #t,contours, hier = cv2.findContours(thresh1,cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)
    m, contours, _ = cv2.findContours(closing, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
    for cnt in contours:
        k = k + 1
        rect = cv2.boundingRect(cnt)
        if rect[0] > 300 and rect[0] < 450:
          print(rect[0])
          print(cnt[0])
          cv2.drawContours(crop_img,[cnt],0,(0,255,0),2)
          record = 1
          cv2.imshow('framxds', crop_img)
        elif rect[0] < 150:
          #vis = np.concatenate((crop_img, crop_img), axis=1)
          save = name + str(k) + str(t) + ".jpg"
          cv2.imwrite(save, crop_img)


          cv2.imshow('framxdss', vis)
    cv2.imshow('res',closing)
    cv2.imshow('fx',frame)



    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
