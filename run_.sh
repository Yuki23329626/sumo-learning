#!/bin/bash

cd sumo-learning
git pull
echo "cp ./ns3lte/lteudp_test3.cc ../scratch/"
cp ./ns3lte/lteudp_test3.cc ../scratch/
echo "cp ./test10/test10.tcl ../scratch/"
cp ./test10/test10.tcl ../scratch/
echo "cp ./test10/oneUE.tcl ../scratch/"
cp ./test10/oneUE.tcl ../scratch/
# echo "cp ./test10/background.png ../scratch/"
# cp ./test10/background.png ../scratch/
cd ..
echo "./waf --run \"scratch/lteudp_test3\" > log.lteudp_test3 2>&1"
./waf --run "scratch/lteudp_test3" > log.lteudp_test3 2>&1
