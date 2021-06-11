#!/bin/bash

cd sumo-learning
git pull

echo "cp ./test10/lteudp_lte_uplink.cc ../scratch/"
cp ./test10/lteudp_lte_uplink.cc ../scratch/

echo "cp ./test10/test10.tcl ../scratch/"
cp ./test10/test10.tcl ../scratch/

echo "cp ./test10/oneUE.tcl ../scratch/"
cp ./test10/oneUE.tcl ../scratch/

cd ..
echo "./waf --run \"scratch/lteudp_lte_uplink\" > log.lteudp_lte_uplink 2>&1"
./waf --run "scratch/lteudp_lte_uplink" > log.lteudp_lte_uplink 2>&1