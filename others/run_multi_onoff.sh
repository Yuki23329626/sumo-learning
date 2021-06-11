#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin

trap "kill 0" INT

cd sumo-learning
git pull
cp ./ns3lte/lteudp_multi_onoff.cc ../scratch/
cp ./test10/test10.tcl ../scratch/
cd ..

isSdnEnabled=true
prefixName="lteudp_multi_onoff_sdn"
outLogDir="outLog_${prefixName}"
echo "outLogDir = ${outLogDir}"
mkdir $outLogDir
cp ./sumo-learning/merge_multi.sh $outLogDir
cp ././sumo-learning/log_to_csv3.py $outLogDir

for ((i=0;i<20;i++))
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
    (./waf --run "scratch/lteudpMulti --numberOfUes=100 --startUe=${startUe} --endUe=${endUe} --isSdnEnabled=${isSdnEnabled} --isDownlink=true --isUplink=true --animFile=${animFile} --interAppInterval=2" > $outLogPath 2>&1) &
    if [ $i == 1 ]
    then
	    sleep 20s
    fi
    sleep 1s
done

wait
