import csv
import sys
import shlex
import os
import glob
import numpy as np
import matplotlib.pyplot as plt
import shutil
import json

# get each packet according to their uid to locate its source and destination
# in order to collect its info [From, To, TxTime, RxTime, Delay]
def get_packet(fin_dir):
    uid  = {}
    for filename in glob.glob(os.path.join(fin_dir, "log.*")):
        print("getting packet info in file:", filename)
        uid[filename] = {}
        data = open(filename, 'r')
        lines = data.readlines()
        for line in lines:
            string = line.split(" ")
            if string[0] == "TraceDelay" :
                if int(string[7]) not in uid[filename]:
                    uid[filename][int(string[7])] = ["", string[5], 0, 0, 0] # To Address
            if string[0] == "TraceDelay:":
                if int(string[10]) in uid[filename]:
                    uid[filename][int(string[10])][0] = string[5] # From Address
                    uid[filename][int(string[10])][2] = eval(string[12][:-2])/1000000000 # TX in sec
                    uid[filename][int(string[10])][3] = eval(string[14][:-2])/1000000000 # RX in sec
                    uid[filename][int(string[10])][4] = eval(string[16][:-3])/1000000000 # Delay in sec
        #break # uncomment this line if you want to do a quick test
    packet = {}
    for filename in uid.keys():
        for item in uid[filename]:
            #print("item:",item)
            #print(uid[filename][item])
            if uid[filename][item][0] == '':
                continue
            key = uid[filename][item][0] + " " + uid[filename][item][1]
            if key not in packet:
                packet[key] = []
                packet[key].append([uid[filename][item][2], uid[filename][item][3], uid[filename][item][4]])
            else:
                packet[key].append([uid[filename][item][2], uid[filename][item][3], uid[filename][item][4]])
    return packet

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
    #for item in list_handover_time:
        #print("IMSI:", item[0])
        #print("HO_time:", item[1])
    return list_handover_time # list_handover_time[IMSI] = handover_time

def get_avg_delay_all(packet_dict):
    route_upload = 0
    route_download = 0

    upload_1k_delay_sum = 0
    upload_1m_delay_sum = 0

    download_1k_delay_sum = 0
    download_1m_delay_sum = 0

    for route in packet_dict:
        #print("route:", route)
        start = packet_dict[route][0][0]
        end = packet_dict[route][-1][1]
        num = len(packet_dict[route])
        #print("time spent:", (end-start))
        #print("packet received:", num)
        throughput_per_sec = num*1024/(end-start)
        #print("thoughput per sec:", throughput_per_sec)
        delay_1k = 1024/throughput_per_sec
        #print("1k byte packet delay:", delay_1k)
        delay_1m = 1024*1024/throughput_per_sec
        #print("1M byte packet delay", delay_1m)
        if route.split(" ")[0] == "1.0.0.2":
            route_download = route_download + 1
            download_1k_delay_sum = download_1k_delay_sum + delay_1k
            download_1m_delay_sum = download_1m_delay_sum + delay_1m
        else:
            route_upload = route_upload + 1
            upload_1k_delay_sum = upload_1k_delay_sum + delay_1k
            upload_1m_delay_sum = upload_1m_delay_sum + delay_1m
    
    print("average upload delay(1k byte):")
    print(upload_1k_delay_sum/route_upload)
    print("average upload delay(1m byte):")
    print(upload_1m_delay_sum/route_upload)
    print("average download delay(1k byte):")
    print(download_1k_delay_sum/route_download)
    print("average download delay(1m byte):")
    print(download_1m_delay_sum/route_download)

def get_section(list_handover_time, packet):
    section = {}
    for imsi_hotimes in list_handover_time:
        for hotime in imsi_hotimes[1]:
            if (hotime < 55) and (hotime > 5):
                start_time = hotime 
                end_time = hotime + 5
                #print("hotime:", hotime)
                ue_address = "7.0.0." + str(imsi_hotimes[0]+1)
                upload_route = ue_address + " 1.0.0.2"
                download_route = "1.0.0.2 " + ue_address
                #print(upload_route)
                #print(download_route)
                #input()
                for item in packet[upload_route]:
                    if (item[0] >= start_time) and (item[0] <= end_time):
                        if upload_route not in section:
                            section[upload_route] = {}
                        if hotime not in section[upload_route]:
                            section[upload_route][hotime] = []
                        section[upload_route][hotime].append([item[0], item[1], item[2]])
                #print(section[upload_route][hotime])
                #input()
                for item in packet[download_route]:
                    if (item[0] >= start_time) and (item[0] <= end_time):
                        if download_route not in section:
                            section[download_route] = {}
                        if hotime not in section[download_route]:
                            section[download_route][hotime] = []
                        section[download_route][hotime].append([item[0], item[1], item[2]])
    return section

def get_avg_delay_from_hotime(section):
    route_upload = 0
    route_download = 0

    upload_1k_delay_sum = 0
    upload_1m_delay_sum = 0

    download_1k_delay_sum = 0
    download_1m_delay_sum = 0
    
    for route in section:
        for hotime in section[route]:
            #print(hotime)
            start = section[route][hotime][0][0]
            end = section[route][hotime][-1][0]
            num = len(section[route][hotime])

            throughput_per_sec = num*1024/(end-start)
            delay_1k = 1024/throughput_per_sec
            delay_1m = 1024*1024/throughput_per_sec
            #print("route:",route)
            #print("num:",num)
            #print("delay_1k:",delay_1k)
            #print("delay_1m:",delay_1m)
            if route.split(" ")[0] == "1.0.0.2":
                route_download = route_download + 1
                download_1k_delay_sum = download_1k_delay_sum + delay_1k
                download_1m_delay_sum = download_1m_delay_sum + delay_1m
            else:
                route_upload = route_upload + 1
                upload_1k_delay_sum = upload_1k_delay_sum + delay_1k
                upload_1m_delay_sum = upload_1m_delay_sum + delay_1m

    upload_1k_delay = upload_1k_delay_sum/route_upload
    upload_1m_delay = upload_1m_delay_sum/route_upload
    download_1k_delay = download_1k_delay_sum/route_download
    download_1m_delay = download_1m_delay_sum/route_download
    print("average delay of upload 1KB(sec):")
    print(upload_1k_delay)
    print("average delay of upload 1MB(sec):")
    print(upload_1m_delay)
    print("average delay of download 1KB(sec):")
    print(download_1k_delay)
    print("average delay of download 1MB(sec):")
    print(download_1m_delay)
    return upload_1k_delay, upload_1m_delay, download_1k_delay, download_1m_delay

def get_avg_delay_from_hotime_2(section):
    
    upload_count = 0
    download_count = 0

    upload_delay_sum = 0
    download_delay_sum = 0

    upload_1m_count = 0
    download_1m_count = 0
    upload_1m_delay_sum = 0
    download_1m_delay_sum = 0

    for route in section:
        for hotime in section[route]:
            for packet in section[route][hotime]:
                if route.split(" ")[0] == "1.0.0.2":
                    download_count = download_count + 1
                    if download_count == 1024:
                        download_1m_count = download_1m_count + 1
                        download_1m_delay_sum = download_1m_delay_sum + (packet[1]-section[route][hotime][0][0])
                    download_delay_sum = download_delay_sum + packet[2]
                else:
                    upload_count = upload_count + 1
                    if upload_count == 1024:
                        upload_1m_count = upload_1m_count + 1
                        upload_1m_delay_sum = upload_1m_delay_sum + (packet[1]-section[route][hotime][0][0])
                    upload_delay_sum = upload_delay_sum + packet[2]
    
    print("single packet upload delay:")
    print(upload_delay_sum/upload_count)
    print("single packet downlaod delay:")
    print(download_delay_sum/download_count)
    print("1m packet upload delay:")
    print(upload_1m_delay_sum/upload_1m_count)
    print("1m packet downlaod delay:")
    print(download_1m_delay_sum/download_1m_count)


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

# sacn each log in dir

path_packet_sdn = dir_output + "/packet_sdn_.json"
path_packet_lte = dir_output + "/packet_lte_.json"

if(not os.path.exists(path_packet_sdn)):
    packet_sdn = get_packet(dir_udp_sdn)
    json.dump(packet_sdn, open(path_packet_sdn,'w'))
else:
    print("get packet info from file:", path_packet_sdn)
    packet_sdn = json.load(open(path_packet_sdn,'r'))

if(not os.path.exists(path_packet_lte)):
    packet_lte = get_packet(dir_udp_lte)
    json.dump(packet_lte, open(path_packet_lte,'w'))
else:
    print("get packet info from file:", path_packet_lte)
    packet_lte = json.load(open(path_packet_lte,'r'))

# packet["From To"] = [[TX, RX, Delay],...]

# get handover time in sdn

list_handover_time = getHOTime(dir_udp_sdn)

#print("len(list_handover_time):", len(list_handover_time))

#for item in list_handover_time:
#    print("IMSI:", item[0])
#    for handover_time in item[1]:
#        print("handover_time:", handover_time)

# average delay SDN
#print("\nSDN:\n")
#get_avg_delay_all(packet_sdn)
#print("\nLTE:\n")
#get_avg_delay_all(packet_lte)

# average delay from sdn_hotime to sdn_hotime+6(sec)

section_sdn = get_section(list_handover_time, packet_sdn)
section_lte = get_section(list_handover_time, packet_lte)

# calculate average delay in section
print("\nSDN after handover:\n")
upload_1k_sdn, upload_1m_sdn, download_1k_sdn, download_1m_sdn = get_avg_delay_from_hotime(section_sdn)
print("\nLTE after handover:\n")
upload_1k_lte, upload_1m_lte, download_1k_lte, download_1m_lte = get_avg_delay_from_hotime(section_lte)

# single packet average delay and cumulate 1024 packet delay
#print("\nSDN cumulate 1024 packet before and after handover:\n")
#get_avg_delay_from_hotime_2(section_sdn)
#print("\nLTE cumulate 1024 packet before and after handover:\n")
#get_avg_delay_from_hotime_2(section_lte)

# draw picture

# 1MB
fig2 = plt.figure(2, figsize=(8,6))
condition = ['Upload', 'Downlaod']
lte_delay = [upload_1m_lte, download_1m_lte]
sdn_delay = [upload_1m_sdn, download_1m_sdn]
x = np.arange(len(condition))
width = 0.4
plt.bar(x, lte_delay, width, color='blue', label='LTE')
plt.bar(x+width, sdn_delay, width, color='green', label='SDN')
plt.xticks( x + width / 2, condition)
plt.ylabel('Average Delay(sec)')
plt.title('1MB file transmission delay between LTE and SDN')
plt.legend( loc='upper right')
fig2.show()

# 1KB
fig1 = plt.figure(1, figsize=(8,6))
axes = plt.gca()
axes.set_ylim([0,0.006])
condition = ['Upload', 'Downlaod']
lte_delay = [upload_1k_lte, download_1k_lte]
sdn_delay = [upload_1k_sdn, download_1k_sdn]
x = np.arange(len(condition))
width = 0.4
plt.bar(x, lte_delay, width, color='blue', label='LTE')
plt.bar(x+width, sdn_delay, width, color='green', label='SDN')
plt.xticks( x + width / 2, condition)
plt.ylabel('Average Delay(sec)')
plt.title('1KB file transmission delay between LTE and SDN')
plt.legend( loc='upper right')
fig1.show()
input()


