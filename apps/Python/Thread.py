import threading as t
import time
def thr(Name, value):
 print(Name+":"+str(value)+" "+time.ctime())
 time.sleep(value)
 print(Name+" end sleep "+time.ctime())
 quit()
t1=t.Thread(target=thr,args=("1st",10))
t2=t.Thread(target=thr,args=("2nd",5))
t1.start()
t2.start()
t1.join()
t2.join()

