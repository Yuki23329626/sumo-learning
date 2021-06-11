#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin

trap "kill 0" INT

cd sumo-learning
git pull
cp ./ns3lte/lteudpMulti.cc ../scratch/
cp ./test10/test10.tcl ../scratch/
cd ..

prefixName="lteudpMultiSdn"
outLogDir="outLog_${prefixName}"
echo "outLogDir = ${outLogDir}"
mkdir $outLogDir
cp ./sumo-learning/mergeMulti.sh $outLogDir
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
    (./waf --run "scratch/lteudpMulti --numberOfUes=100 --startUe=${startUe} --endUe=${endUe} --isSdnEnabled=true --isDownlink=true --isUplink=true --animFile=${animFile}" > $outLogPath 2>&1) &
    sleep 1s
done

wait