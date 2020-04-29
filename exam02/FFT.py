import matplotlib.pyplot as plt
import numpy as np
import serial
import time


t = np.arange(0,10,0.1) 
X = np.arange(0,10,0.1)
Y = np.arange(0,10,0.1)
Z = np.arange(0,10,0.1)
distance = np.arange(0,10,0.1)

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev,115200)

for x in range(0, 100):
    
    linex=s.readline() 
    liney=s.readline()
    linez=s.readline()
    linedistance=s.readline()
    # print line
    X[x] = float(linex)
    Y[x] = float(liney)
    Z[x] = float(linez)
    distance[x] = float(linedistance)

fig, ax = plt.subplots(2, 1)
ax[0].plot(t,X,label = "x")
ax[0].plot(t,Y,label = "y")
ax[0].plot(t,Z,label = "z")
ax[0].legend(loc='lower left')
ax[0].set_xlabel('Time')
ax[0].set_ylabel('Acc Vector')
plt.stem(t,distance,use_line_collection=True)
ax[1].set_xlabel('Time')
ax[1].set_ylabel('distance')

plt.show()
s.close()