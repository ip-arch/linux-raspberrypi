# script by Alex Eames http://RasPi.tv  
# http://RasPi.tv/how-to-use-interrupts-with-python-on-the-raspberry-pi-and-rpi-gpio-part-3  

import RPi.GPIO as GPIO  
import ports

GPIO.setmode(GPIO.BCM)  
  
# GPIO 2 set up as inputs, pulled up to avoid false detection.  
# Both ports are wired to connect to GND on button press.  
# So we'll be setting up falling edge detection for both  

GPIO.setup(ports.Ports["S1"], GPIO.IN, pull_up_down=GPIO.PUD_UP)  
  
# GPIO 3 set up as an input, pulled up, connected to 3V3 on button press  

GPIO.setup(ports.Ports["S2"], GPIO.IN, pull_up_down=GPIO.PUD_UP)  
  
# now we'll define two threaded callback functions  
# these will run in another thread when our events are detected  

def my_callback(channel):  
    print( "falling edge detected on S1"  )
  
raw_input("Press Enter when ready\n>")  
  
# when a falling edge is detected on port 17, regardless of whatever   
# else is happening in the program, the function my_callback will be run  

GPIO.add_event_detect(ports.Ports["S1"], GPIO.FALLING, callback=my_callback, bouncetime=300)  
  
try:  
    print( "Waiting for rising edge on port S2" ) 
    GPIO.wait_for_edge(ports.Ports["S2"], GPIO.RISING)  
    print( "Rising edge detected on port S2. Here endeth the third lesson."  )
  
except KeyboardInterrupt:  
    GPIO.cleanup()       # clean up GPIO on CTRL+C exit  
GPIO.cleanup()           # clean up GPIO on normal exit  
