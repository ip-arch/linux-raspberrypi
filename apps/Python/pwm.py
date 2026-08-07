import timer
import ports
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(ports.Ports["LED1"], GPIO.OUT)
pwm=GPIO.PWM(ports.Ports["LED1"], 100)
pwm.start(0)
x=0
try:
 while True:
   x=x+1
   if x>100:
     x=0
   pwm.ChangeDutyCycle(x)
   time.sleep(0.01)
except KeyboardInterrupt:
 print("Key Interrupt")
quit()

