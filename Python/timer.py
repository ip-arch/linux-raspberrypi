import threading

def hello():
    t = threading.Timer(1.0, hello)
    t.start()
    print("hello, world") 
    quit()
t = threading.Timer(1.0, hello)
t.start() 
