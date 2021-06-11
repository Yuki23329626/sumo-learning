#!/bin/bash

cd sumo-learning
git pull
cp ./ns3lte/lteudp_test3.cc ../scratch/
cp ./test10/test10.tcl ../scratch/
cd ..
isSdnEnabled=true
outputFile=log.lteudp_test3_sdn
echo "isSdnEnabled=${isSdnEnabled}"
echo "outputFile=${outputFile}"
./waf --run "scratch/lteudp_test3 --isSdnEnabled=${isSdnEnabled} --isDownlink=true --isUplink=true --interPacketInterval=50" > $outputFile 2>&1
