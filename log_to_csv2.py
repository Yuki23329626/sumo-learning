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

lastType = ""
lastLineAddress = ""
with open("csv_"+filename+".csv", 'w') as csvfile:
    for line in lines:
        #if(count == 10):
        #    break
        string1 = line.split("(")
        # print("string1[0]", string1[0])
        if(string1[0] == "OnOffApplication:SendPacket"):
            clientAddress = string1[1][2:-2]
            #print("clientAddress", clientAddress)
            lastType = "SendPacket"
            lastLineAddress = clientAddress
            continue
        if(string1[0] == "PacketSink:HandleRead"):
            sinkAddress = string1[1][2:-2]
            #print("sinkAddress", sinkAddress)
            lastType = "PacketSink"
            lastLineAddress = sinkAddress
            continue
        if(lastLineAddress != "" and lastType == "SendPacket"):
            string2 = line.split(" ")
            #print("string2", string2)
            if lastLineAddress in sendPacketStart:
                lastType = ""
                lastLineAddress = ""
                continue
            sendPacketStart[lastLineAddress] = string2[2][:-1]
            print("sendPacketStart", sendPacketStart)
            print("sendPacketStart[lastLineAdress]:", sendPacketStart[lastLineAddress])
            lastType = ""
            lastLineAddress = ""
        if(lastLineAddress != "" and lastType == "PacketSink"):
            string2 = line.split(" ")
            clientAddress = lastLineAddress.split(",")[0]
            serverAddress = lastLineAddress.split(",")[1]
            timeReceived = string2[2][:-1] # in second
            totalRxBytes = string2[-2]
            sinkPacketEnd[clientAddress] = clientAddress + " " + timeReceived + " " + totalRxBytes
            #print("Total RX:", string2[-2])
            #print("sinkPacketEnd", sinkPacketEnd)
            #print("sinkPacketEnd[lastLineAdress]:", sinkPacketEnd[lastLineAddress])
            lastType = ""
            lastLineAddress = ""

print("sinkPacketEnd", sinkPacketEnd)

index = 1
for key in sinkPacketEnd:
    print("index:", index)
    print("client:", key)
    print("server:", sinkPacketEnd[key].split(" ")[0])
    print("delay:", eval(sinkPacketEnd[key].split(" ")[1]) - eval(sendPacketStart[key]))
    print("packet loss:", (1 - eval(sinkPacketEnd[key].split(" ")[2]) / nMaxBytes)*100, "%")

# print("packet_sent: ", count_sent)
# print("packet_received: ", count)
# print("average delay(s): ", sum/count)
