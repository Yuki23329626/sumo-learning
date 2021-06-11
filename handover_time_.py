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

handover_time= {}

for line in lines:
    string1 = line.split(" ")
    if( string1[0] != "NotifyHandoverEndOkUe"):
        continue
    print("string1",string1)
    print("string1[5]:",string1[5][:-1])
    print("string1[2]:",string1[2][:6])
    if string1[5] in handover_time:
        handover_time[int(string1[5][:-1])].append(string1[2][:6])
    else:
        handover_time[int(string1[5][:-1])]=[string1[2][:6]]

for key in handover_time:
    print("IMSI:",key)
    handover_time[key] = sorted(handover_time[key])
    for value in handover_time[key]:
        print("handover_time:", value)

print("\n")

list_handover_time = (sorted(handover_time.items(), key=lambda x:x[0]))

for item in list_handover_time:
    print("IMSI:", item[0])
    for time in item[1]:
        print("handover_time:", time)
