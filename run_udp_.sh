#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin

trap "kill 0" INT

prefixTCL="2021-06-08-16-43-01"
maxSpeed="_max-speed-40"

cd sumo-learning
git pull
cp ./multi_udp.cc ../scratch/
cp ./${prefixTCL}/${prefixTCL}${maxSpeed}.tcl ../scratch/
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
mkdir -p $outLogDir
cp ./sumo-learning/merge_multi.sh $outLogDir
cp ././sumo-learning/log_to_csv.py $outLogDir

logName="log.${prefixName}${maxSpeed}"
outLogPath="${outLogDir}/${logName}"
animFile="anim.${prefixName}_${i}"
echo "logName = ${logName}"
echo "outLogDir = ${outLogDir}"
echo "animFile = ${animFile}"
startUe=0
endUe=100
echo "startUe = ${startUe}"
echo "endUe = ${endUe}"
(./waf --run "scratch/lteudp_multi_udp --simTime=60 --numberOfUes=100 --startUe=${startUe} --endUe=${endUe} --isSdnEnabled=${isSdnEnabled} --isDownlink=true --isUplink=true --animFile=${animFile} --interAppInterval=0.001 --interPacketInterval=1 --nPayloadBytes=1036" > $outLogPath 2>&1) &

echo "isSdnEnabled = ${isSdnEnabled}"
echo "outLogDir = ${outLogDir}"

wait
