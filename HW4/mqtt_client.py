import paho.mqtt.client as paho
import time
import matplotlib.pyplot as plt
import numpy as np

# MQTT broker hosted on local machine
mqttc = paho.Client()
# Settings for connection
host = "localhost"
topic = "acc"
X = []
Y = []
Z = []
T = []

def result():
      timestamp = np.arange(0, 20, 1) 
      fig, ax = plt.subplots(1, 1)
      ax[0].set_xlabel('timestamp')
      ax[0].set_ylabel('acc value')
      ax[0].plot(timestamp, X, color = 'red')
      ax[0].plot(timestamp, Y, color = 'blue')
      ax[0].plot(timestamp, Z, color = 'green')
      ax[0].legend(['x-acc', 'y-acc', 'z-acc'], loc = 'upper right')
      ax[0].set_title('Acceleration Plot')
      
    #   ax[1].plot(timestamp, T) # plotting the spectrum
    #   ax[1].set_xlabel('timestamp')
    #   ax[1].set_ylabel('Tilt')
    #   ax[1].set_title('Tilt Plot')
      plt.show()

# Callbacks
def on_connect(self, mosq, obj, rc):
      print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
      print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")
      tmp = str(msg.payload)  
      print(tmp)
      l = len(msg.payload)
      if(tmp[2] == 'X'):
            X.append(float(tmp[3:3+l-1]))
            i_count = i_count + 1
      elif(tmp[2]== 'Y'):
            Y.append(float(tmp[3:3+l-1]))
      elif(tmp[2]== 'Z'):
            Z.append(float(tmp[3:3+l-1]))
    #   elif(tmp[2]== 't'):
    #         T.append(float(tmp[3]))
    #   print(X)
    #   print(Y)
    #   print(Z)
    #   print(T)
    #   print(len(X))

      


def on_subscribe(mosq, obj, mid, granted_qos):
      print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
      print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)


# Publish messages from Python
num = 0

while len(T)<20:
      mqttc.loop()

if(len(T) == 20):
      result()
      