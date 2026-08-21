import RPi.GPIO as GPIO  
import ports

GPIO.setmode(GPIO.BCM)  
GPIO.setup(ports.Ports["S1"], GPIO.IN, pull_up_down=GPIO.PUD_UP)  
GPIO.setup(ports.Ports["S2"], GPIO.IN, pull_up_down=GPIO.PUD_UP)  
GPIO.setup(ports.Ports["LED1"], GPIO.OUT)
GPIO.setup(ports.Ports["LED2"], GPIO.OUT)
  
try:  
	while True:
		GPIO.output(ports.Ports["LED1"], 1 - GPIO.input(ports.Ports["S1"]))
		GPIO.output(ports.Ports["LED2"], 1 - GPIO.input(ports.Ports["S2"]))
except KeyboardInterrupt:  
    GPIO.cleanup()       # clean up GPIO on CTRL+C exit  
quit()
