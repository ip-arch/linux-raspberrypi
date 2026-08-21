import RPi.GPIO as GPIO  
import ports
import seg7
import time
import PyTimer

GPIO.setwarnings(False)

segfull = {"a","b","c","d","e","f","g","dp"}
def getport(key):
  return ports.Ports[key]
  

GPIO.setmode(GPIO.BCM)  
GPIO.setup(ports.Ports["S1"], GPIO.IN, pull_up_down=GPIO.PUD_UP)  
GPIO.setup(ports.Ports["S2"], GPIO.IN, pull_up_down=GPIO.PUD_UP)  
GPIO.setup(ports.Ports["DIS1"], GPIO.OUT)
GPIO.setup(ports.Ports["DIS2"], GPIO.OUT)
GPIO.setup(list(map(getport, segfull)), GPIO.OUT )

def disp(NUM,dig):
          D = (NUM // (10**dig)) % 10
          clrseg = segfull - set(seg7.num2seg[D])
          portlist = list(map(getport, seg7.num2seg[D]))
          clrlist  = list(map(getport, clrseg))
          GPIO.output(portlist, 1)
          GPIO.output(clrlist,  0)

dig=0
num=0
def digsw(x):
   global dig, num;
   dig = 1 - dig
   GPIO.output([ports.Ports["DIS1"],ports.Ports["DIS2"]], [dig,1-dig])
   disp(num,dig)

t0 = PyTimer.create(PyTimer.CLOCK_MONOTONIC, digsw)
PyTimer.settime(t0, 1, 0.01)

try:  
	for num in range(100):
          time.sleep(1)
          num = num +1
except KeyboardInterrupt:  
          GPIO.cleanup()       # clean up GPIO on CTRL+C exit  
quit()
