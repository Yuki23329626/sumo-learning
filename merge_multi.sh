#/bin/bash

all_num=19
prefix="log.lteudp_multi___"
targetFile="total.log"
rm $targetFile

a=$(date +%H%M%S)

for index in `seq 0 ${all_num}`
do
{
    cat ${prefix}${index} >> $targetFile
    echo ${index}
}
done

wait

b=$(date +%H%M%S)

echo -e "startTime:\t$a"
echo -e "endTime:\t$b"
