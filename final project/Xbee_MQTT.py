import paho.mqtt.client as paho
import serial
import time
import matplotlib.pyplot as plt
import numpy as np

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev,9600,timeout=5)

mqttc = paho.Client()

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

host = "localhost"
topic= "final"
port = 1883

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)
# Settings for connection


s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())

s.write("ATMY 158\r\n".encode())
char = s.read(3)
print("Set MY 158.")
print(char.decode())

s.write("ATDL 258\r\n".encode())
char = s.read(3)
print("Set DL 258.")
print(char.decode())

s.write("ATID 1\r\n".encode())
char = s.read(3)
print("Set PAN ID 1.")
print(char.decode())

s.write("ATWR\r\n".encode())
char = s.read(3)
print("Write config.")
print(char.decode())

s.write("ATMY\r\n".encode())
char = s.read(4)
print("MY :")
print(char.decode())

s.write("ATDL\r\n".encode())
char = s.read(4)
print("DL : ")
print(char.decode())

s.write("ATCN\r\n".encode())
char = s.read(3)
print("Exit AT mode.")
print(char.decode())

print("start sending RPC")

action_type = ["go straight", "turn left", "turn right",
                "scanning", "go backwards"]
i = 0
action = []


# send RPC to remote per second (21 in total)
while i<201:
    print(i)
    # get the times 
    ACCcount = s.read(4).decode()
    print(ACCcount)
    action.append(ACCcount)
    time.sleep(1)
    i = i +1


# for i in range(0, 300):
#     mesg = action_type[i]
#     mqttc.publish(topic, mesg)  
#     print(mesg)


s.close()


# send RPC to remote per second (21 in total)


# publish to 

# for i in range(0, 200):
#     mesg = "X" + X[i+1]
#     mqttc.publish(topic, mesg)  
#     print(mesg)

    
# s.close()