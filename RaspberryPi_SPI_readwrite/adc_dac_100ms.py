# adc-dac-7.py : every 100msec, a point is recorded for dac and adc
#5.0 should be accurate in power supply...&...
#for value>4096, the bits roll off to zero and count

import spidev
from time import sleep
import matplotlib.pyplot as plt
import threading
__adcrefvoltage = 3.3 # reference voltage for the ADC chip.
spiADC = spidev.SpiDev();spiADC.open(0,0);spiADC.max_speed_hz = (4000000)
spiDAC = spidev.SpiDev();spiDAC.open(0,1);spiDAC.max_speed_hz = (4000000)
  

def measure(value,speed=4000000,delay=0):
  lowByte = value & 0xff
  highByte = ((value >> 8) & 0xff) | 0 << 7 | 0x0 << 5 | 1 << 4 # 0-channel 1
  value=value+20#5:cool data
  spiDAC.writebytes([highByte, lowByte])
  r = spiADC.xfer2([1,(2)<<6,highByte,lowByte],speed,delay)#4 000 000
  ret = ((r[1]&0x0F) << 8) + (r[2])
  if(ret>5):
    dac.append(value*4.096/4096);adc.append(ret*5.00/4096)
  global measure_over;measure_over=True;
  M_t=threading.Timer(0.1,measure,[value,speed,delay]);M_t.start();
   
measure_over=False;value=0;dac=[];adc=[];speed=1000000;delay=300
M=threading.Timer(0.1,measure,[value,speed,delay]);M.start();
