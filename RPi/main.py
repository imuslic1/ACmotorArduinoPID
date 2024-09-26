from machine import Pin, PWM
from time import sleep
import utime

zeroCross = Pin(0, Pin.IN)
count = 0



def isr(Pin):
    global count
    count += 1
    print("Zero cross interrupt triggered ", count)

zeroCross.irq(trigger=Pin.IRQ_RISING, handler=isr)

while(True):
    utime.sleep(1)