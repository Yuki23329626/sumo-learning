import csv
import sys
import shlex
import os
filename = " ".join(map(shlex.quote, sys.argv[1:]))
print("filename: ", filename)

if(filename == "" or not os.path.exists(filename)):
    print("please type the correct file name of the log")
    exit()

data = open(filename, 'r')
lines = data.readlines()

count_sent = 0
count = 0
sum = 0
nMaxBytes = 1048576 # Bytes

sendPacketStart = {}
sinkPacketEnd = {}
packetReceived = {}
routeTx = ""
routeRx = ""

with open("csv_"+filename+".csv", 'w') as csvfile:
    for line in lines:
        #if(count == 10):
        #    break
        string1 = line.split(" ")
        #print("\nstring1[0]", string1[0])
        if(string1[0] == "TxTrace:"):
            sourceAddress = string1[8]
            destinationAddress = string1[10]
            #print("sourceAddress:", sourceAddress)
            #print("destinationAddress:", destinationAddress)
            routeTx = sourceAddress + " " + destinationAddress
            if routeTx in sendPacketStart:
                continue
            sendPacketStart[routeTx] = string1[2] # time start
            #print("routeTx", routeTx)
            #print("sendPacketStart[routeTx]:", sendPacketStart[routeTx])
        if(string1[0] == "RxTrace:"):
            sourceAddress = string1[7]
            destinationAddress = string1[9]
            #print("sourceAddress:", sourceAddress)
            #print("destinationAddress:", destinationAddress)
            routeRx = sourceAddress + " " + destinationAddress
            sinkPacketEnd[routeRx] = string1[2] # time end
            #print("routeRx", routeRx)
            #print("sinkPacketEnd[routeRx]:", sinkPacketEnd[routeRx])
            if routeRx in packetReceived:
                if ( packetReceived[routeRx] >= nMaxBytes ):
                    continue
                #print("packetReceived[routeRx]:", packetReceived[routeRx])
                #print("string1[5]:", string1[5])
                packetReceived[routeRx] = packetReceived[routeRx] + int(string1[5])
                continue
            packetReceived[routeRx] = 1024


print("sendPacketStart", sendPacketStart)
print("sinkPacketEnd", sinkPacketEnd)

totalReceived = 0
totalDelay = 0.0
uploadReceived = 0
uploadDelay = 0.0
downloadReceived = 0
downloadDelay = 0.0

index = 0
for key in sinkPacketEnd:
    #if(key == list(sinkPacketEnd.keys())[-2]):
    #    break
    index = index + 1
    print("index:", index)
    print("source:", key.split(" ")[0])
    print("destination:", key.split(" ")[1])
    delay = float(sinkPacketEnd[key]) - float(sendPacketStart[key])
    print("delay:", delay)
    print("packetLoss:", (1 - packetReceived[key] / nMaxBytes)*100, "%")
    print("")
    totalReceived = totalReceived + packetReceived[key]
    totalDelay = totalDelay + delay
    if( key.split(" ")[0][0] == "1" ):
        downloadReceived = downloadReceived + packetReceived[key]
        downloadDelay = downloadDelay + delay
    else:
        uploadReceived = uploadReceived + packetReceived[key]
        uploadDelay = uploadDelay + delay

half = index/2

print("\ntotal packet loss:", (1 - totalReceived / (index*nMaxBytes))*100, "%")
print("average delay(s):", totalDelay / index)

print("\ndownload video packet loss:", (1 - downloadReceived / (half*nMaxBytes))*100, "%")
print("downlaod video average delay(s):", downloadDelay / half)

print("\nupload video packet loss:", (1 - uploadReceived / (half*nMaxBytes))*100, "%")
print("upload video average delay(s):", uploadDelay / half)

print("downlaod average delay(s):", (downloadDelay / half) / 1024)

print("upload average delay(s):", (uploadDelay / half) / 1024)

print("")

# print("packet_sent: ", count_sent)
# print("packet_received: ", count)
# print("average delay(s): ", sum/count)
