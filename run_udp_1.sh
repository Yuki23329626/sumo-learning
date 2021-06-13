#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin

trap "kill 0" INT

prefixTCL="2021-06-08-16-43-01"
maxSpeed="_max-speed-80"

cd sumo-learning
git pull
cp ./multi_udp.cc ../scratch/
cp ./${prefixTCL}/${prefixTCL}${maxSpeed}.tcl ../scratch/
cd ..

prefixName="udp_sdn"
isSdnEnabled=true
if [ $isSdnEnabled = false ]
then
	prefixName="udp_lte"
fi

outLogDir="outLog_${prefixName}"
echo "isSdnEnabled = ${isSdnEnabled}"
echo "outLogDir = ${outLogDir}"
mkdir -p $outLogDir
cp ./sumo-learning/merge_multi.sh $outLogDir
cp ./sumo-learning/log_to_csv.py $outLogDir
cp ./run_udp_1.sh $outLogDir

for i in $(seq 0 19)
do
    logName="log.${prefixName}_${i}"
    outLogPath="${outLogDir}/${logName}"
    animFile="anim.${prefixName}_${i}"
    echo "logName = ${logName}"
    echo "outLogDir = ${outLogDir}"
    echo "animFile = ${animFile}"
    startUe=$(($i*5))
    endUe=$(($i*5+5))
    echo "startUe = ${startUe}"
    echo "endUe = ${endUe}"
    (./waf --run "scratch/lteudp_multi_udp --simTime=60 --numberOfUes=100 --startUe=${startUe} --endUe=${endUe} --isSdnEnabled=${isSdnEnabled} --isDownlink=true --isUplink=true --animFile=${animFile} --interAppInterval=0.001 --interPacketInterval=1 --nPayloadBytes=1036" > $outLogPath 2>&1) &
    if [ $i = 0 ]
    then
	    sleep 20s
    fi
    sleep 1s
done

echo "isSdnEnabled = ${isSdnEnabled}"
echo "outLogDir = ${outLogDir}"

wait
