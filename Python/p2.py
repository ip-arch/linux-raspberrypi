import time
import threading
import RPi.GPIO as GPIO
import ports
import seg7

def dic(p):
 return ports.Ports[p]

GPIO.setmode(GPIO.BCM)
outlist=['a','b','c','d','e','f','g','dp','DIS1','DIS2','LED1','LED2']
GPIO.setup(map(dic,outlist),GPIO.OUT)
GPIO.setup([ports.Ports['S1'],ports.Ports['S2']],GPIO.IN)

global i,j

def dsp(n,p):
   dsctl={0:[GPIO.HIGH,GPIO.LOW],1:[GPIO.LOW,GPIO.HIGH]}
   GPIO.output([ports.Ports['DIS1'], ports.Ports['DIS2']],dsctl[p])
   outchan = map(dic,seg7.num2seg[8])
   GPIO.output(outchan, GPIO.LOW)
   outchan = map(dic,seg7.num2seg[n])
   GPIO.output(outchan, GPIO.HIGH)

flag=0

def dxx():
   global flag
   while True:
     num=[i,j]
     dsp(num[flag], flag)
     flag=1-flag
     time.sleep(0.01)

t = threading.Timer(0.01,dxx)
t.start()

while True:
 for i in range(0,10):
   for j in range(0,10):
    time.sleep(1)
raw_input()
GPIO.cleanup()
quit()


