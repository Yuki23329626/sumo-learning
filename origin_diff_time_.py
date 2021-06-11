import csv
import sys
import shlex
import os
import glob
import matplotlib.pyplot as plt
import shutil

def merge(finDir, fout):
    print("\nmerge files in", finDir, "into", fout)
    with open(fout, "w") as outfile:
        for filename in glob.glob(os.path.join(finDir, "log.*")):
            print(filename)
            with open(filename, 'r') as fin:
                shutil.copyfileobj(fin, outfile)
            fin.close()
    outfile.close()

def getHOTime(finDir):
    handover_time = {}
    list_handover_time = []
    files = glob.glob(os.path.join(finDir, "*0"))
    with open(files[0], "r") as fin:
        lines = fin.readlines()
        for line in lines:
            string1 = line.split(" ")
            if string1[0] != "NotifyHandoverEndOkUe":
                continue
            if int(string1[5][:-1]) in handover_time:
                handover_time[int(string1[5][:-1])].append(float(string1[2][:6]))
            else:
                handover_time[int(string1[5][:-1])]=[float(string1[2][:6])]
        fin.close()
        list_handover_time = (sorted(handover_time.items(), key=lambda x:x[0]))
    for item in list_handover_time:
        print("IMSI:", item[0])
        print("HO_time:", item[1])
    return list_handover_time


prefix_name = " ".join(map(shlex.quote, sys.argv[1:]))
print("prefix_name: ", prefix_name)

dir_udp_sdn = prefix_name + "udp_sdn"

dir_udp_lte = prefix_name + "udp_lte"

dir_output = prefix_name

if(dir_udp_sdn == "" or not os.path.exists(dir_udp_sdn)):
    print("please check if the dir_udp_sdn is exist")
    exit()

if(dir_udp_lte == "" or not os.path.exists(dir_udp_lte)):
    print("please check if the dir_udp_lte is exist")
    exit()

if not os.path.exists(dir_output):
    print("\nmaking dir for output files:", dir_output)
    os.makedirs(dir_output)
print("\ndir_output:", dir_output, "has exist!")


log_total_sdn = dir_output + "/log.total_sdn"
merge(dir_udp_sdn, log_total_sdn)
log_total_lte = dir_output + "/log.total_lte"
merge(dir_udp_lte, log_total_lte)

list_hotime_sdn = getHOTime(dir_udp_sdn)

# delay of each pair

fin = open(log_total_sdn, "r")
lines = fin.readlines()

delay_sdn= {}

for line in lines:
    string1 = line.split(" ")
    if(string1[0] == "TraceDelay"):
        if int(string1[7]) in delay_sdn:
            continue
        else:
            delay_sdn[int(string1[7])] = ["",string1[5],0,0,0] # To
    if(string1[0] == "TraceDelay:"):
        if int(string1[10]) in delay_sdn:
            #print("delay[int(string1[10])][2]:",delay[int(string1[10])][2])
            #print("string1[12][-2]:",string1[12][:-2])
            delay_sdn[int(string1[10])][0] = string1[5] # From
            delay_sdn[int(string1[10])][2] = eval(string1[12][:-2])/1000000000 # TX sec
            delay_sdn[int(string1[10])][3] = eval(string1[14][:-2])/1000000000 # RX sec
            delay_sdn[int(string1[10])][4] = eval(string1[16][:-3])/1000000000 # Delay sec

list_delay_sdn = (sorted(delay_sdn.items(), key=lambda x:x[1][2]))
print(list_delay_sdn)


