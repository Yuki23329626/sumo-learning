#/bin/bash

all_num=34

a=$(date +%H%M%S)

for num in `seq 0 ${all_num}`
do
{
    ./waf --run "test06 --traceFile=scratch/test06.tcl --nodeNum=100 --duration=120 --selectedEnb=${num} --outputDir=test06-1"
    echo ${num}
} &
done

wait

b=$(date +%H%M%S)

echo -e "startTime:\t$a"
echo -e "endTime:\t$b"