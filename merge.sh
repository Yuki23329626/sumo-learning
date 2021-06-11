#/bin/bash

all_num=34
postfix=_test06_enb1.csv

a=$(date +%H%M%S)

echo "Time_sec,IMSI,SINR,X,Y,Selected_eNB" > total.csv

for num in `seq 0 ${all_num}`
do
{
    cat ${num}${postfix} | tail -n +3 >> total.csv
    echo ${num}
}
done

wait

b=$(date +%H%M%S)

echo -e "startTime:\t$a"
echo -e "endTime:\t$b"
