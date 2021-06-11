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

upload_sent = 0
upload_received = 0
download_sent = 0
download_received = 0

upload_delay_sum = 0
download_delay_sum = 0

with open("csv_"+filename+".csv", 'w') as csvfile:
    for line in lines:
        #if(count == 10):
        #    break
        string1 = line.split(" ")
        if(string1[0] == "TraceDelay"):
            if(string1[5] == "1.0.0.2"):
                upload_sent = upload_sent + 1
            else:
                download_sent = download_sent + 1
        if(string1[0] == "TraceDelay:"):
            floatDelay = eval(string1[16][1:-3])/1000000000
            if(string1[5] == "1.0.0.2"):
                download_delay_sum = download_delay_sum + floatDelay
                download_received = download_received + 1
            else:
                upload_delay_sum = upload_delay_sum + floatDelay
                upload_received = upload_received + 1
            #list1 = list(str(floatDelay).split(" "))
            # print(str(eval(string1[16][1:-3])/1000000000))
            #writer = csv.writer(csvfile)
            #writer.writerow(list1)
            #count = count + 1
print("\nupload_sent: ", upload_sent)
print("upload_received: ", upload_received)
print("average upload delay(s): ", upload_delay_sum/upload_received)
print("\ndownload_sent: ", download_sent)
print("download_received: ", download_received)
print("average download delay(s): ", download_delay_sum/download_received)
