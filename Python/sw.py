import threading
import time
import RPi.GPIO as GPIO  
import ports
import seg7

GPIO.setmode(GPIO.BCM)  
  
GPIO.setup(ports.Ports["S1"], GPIO.IN)
GPIO.setup(ports.Ports["S2"], GPIO.IN)
GPIO.setup(ports.Ports["a"], GPIO.OUT)
GPIO.setup(ports.Ports["b"], GPIO.OUT)
GPIO.setup(ports.Ports["c"], GPIO.OUT)
GPIO.setup(ports.Ports["d"], GPIO.OUT)
GPIO.setup(ports.Ports["e"], GPIO.OUT)
GPIO.setup(ports.Ports["f"], GPIO.OUT)
GPIO.setup(ports.Ports["g"], GPIO.OUT)
GPIO.setup(ports.Ports["DIS1"], GPIO.OUT)
GPIO.setup(ports.Ports["DIS2"], GPIO.OUT)

run = 0
digit = 0
curtime = 0

def out_seg(x):
 GPIO.output(ports.Ports[x], GPIO.HIGH)

def flush_seg():
 for i in ["a","b","c","d","e","f","g","DIS1","DIS2"]:
  GPIO.output(ports.Ports[i], GPIO.LOW)


def show_digit():
 global digit, curtime
 while True:
  flush_seg()
  for i in seg7.num2seg[(curtime//(digit*9+1))%10]:
   out_seg(i)
  GPIO.output(ports.Ports[("DIS2","DIS1")[digit]], GPIO.HIGH)
  digit = 1 - digit
  time.sleep(0.005)


def count_time():
 global curtime, run
 while True:
  if run !=0:
   curtime = curtime + 1
  time.sleep(1.0)

t=threading.Timer(0.1, count_time)
d=threading.Timer(0.1, show_digit)
t.start()
d.start()

curtime = 0
  
def my_callback(channel):  
    global run
    run = 1 - run
    print("run = "+str(run))
  
  
GPIO.add_event_detect(ports.Ports["S1"], GPIO.FALLING, callback=my_callback, bouncetime=300)  
  
try:  
  while True:
    GPIO.wait_for_edge(ports.Ports["S2"], GPIO.RISING)  
    curtime = 0
  
except KeyboardInterrupt:  
    t._stop()
    d._stop()
    GPIO.cleanup()       # clean up GPIO on CTRL+C exit  
    quit()
