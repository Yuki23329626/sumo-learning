#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin

trap "kill 0" INT

prefix_tcl="test10"

cd sumo-learning
git pull
cp ./multi_udp.cc ../scratch/
cp ./${prefix_tcl}/${prefix_tcl}.tcl ../scratch/
cd ..

prefixName="multi_udp_sdn"
isSdnEnabled=true
if [ $isSdnEnabled == false ]
then
        prefixName="multi_udp_lte"
fi

outLogDir="outLog_${prefixName}"
echo "isSdnEnabled = ${isSdnEnabled}"
echo "outLogDir = ${outLogDir}"
mkdir $outLogDir
cp ./sumo-learning/merge_multi.sh $outLogDir
cp ././sumo-learning/log_to_csv.py $outLogDir

logName="log.${prefixName}"
outLogPath="${outLogDir}/${logName}"
echo "logName = ${logName}"
echo "outLogDir = ${outLogDir}"
startUe=0
endUe=100
echo "startUe = ${startUe}"
echo "endUe = ${endUe}"
(./waf --run "scratch/lteudp_multi_udp --simTime=60 --numberOfUes=100 --startUe=${startUe} --endUe=${endUe} --isSdnEnabled=${isSdnEnabled} --isDownlink=true --isUplink=true --interAppInterval=0.001 --interPacketInterval=1 --nPayloadBytes=1036" > $outLogPath 2>&1) &

echo "isSdnEnabled = ${isSdnEnabled}"
echo "outLogDir = ${outLogDir}"

wait
