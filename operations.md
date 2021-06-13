# 舊版在 windows 上產生路徑的方式

# Installation
請直接從 [nNS3官方的GitLab](https://gitlab.com/nsnam/ns-3-dev) 複製 respsitory 
```bash
git clone git@gitlab.com:nsnam/ns-3-dev.git
```

1. generate a bundle file from created scenario
sumo -c test09.sumocfg --fcd-output test09.xml --step-length 1

會產生 xml 檔

2. create mobility tract to import for NS3
python traceExporter.py --fcd-input test10.xml --ns2mobility-output test10.tcl
python D:\SUMO\tools\traceExporter.py --fcd-input D:\github\sumo-learning\test09\test09.xml --ns2mobility-output D:\github\sumo-learning\test09\test09.tcl
python D:\SUMO\tools\traceExporter.py --fcd-input D:\github\sumo-learning\test10\test10.xml --ns2mobility-output D:\github\sumo-learning\test10\test10.tcl --begin 270 --end 1240

3. 產生 excel 黨
python D:\SUMO\tools\traceExporter.py traceExporter.py --fcd-input D:\github\sumo-learning\test09\test09.xml --gpsdat-output D:\github\sumo-learning\test09\test09.csv
python D:\SUMO\tools\traceExporter.py --fcd-input D:\github\sumo-learning\test10\test10.xml --gpsdat-output D:\github\sumo-learning\test10\test10.csv --begin 240 --end 300

D:
cd git

會產生 tcl 檔

# 新版在 windows 上產生路徑的方式

``` CMD

:: 設定檔案名稱
set SUMO_PREFIX=2020-12-12-22-44-15
set SUMO_MAXSPEED=_max-speed-40
:: 模擬開始跟結束時間
set SIM_BEGIN=1300
set SIM_END=1421

:: 生成檔案路徑名稱
set SUMO_CONFIG=D:\github\sumo-learning\%SUMO_PREFIX%\osm.sumocfg
set SUMO_XML=D:\github\sumo-learning\%SUMO_PREFIX%\%SUMO_PREFIX%%SUMO_MAXSPEED%.xml
set SUMO_CSV=D:\github\sumo-learning\%SUMO_PREFIX%\%SUMO_PREFIX%%SUMO_MAXSPEED%.csv
set SUMO_TCL=D:\github\sumo-learning\%SUMO_PREFIX%\%SUMO_PREFIX%%SUMO_MAXSPEED%.tcl
echo %SUMO_PREFIX%
echo %SUMO_CONFIG%
echo %SUMO_XML%
echo %SUMO_TCL%

:: 產生 NS3 可以 input 的 mobility trace file
sumo -c %SUMO_CONFIG% --fcd-output %SUMO_XML% --step-length 1
python D:\SUMO\tools\traceExporter.py --fcd-input %SUMO_XML% --ns2mobility-output %SUMO_TCL% --begin %SIM_BEGIN% --end %SIM_END%
python D:\SUMO\tools\traceExporter.py --fcd-input %SUMO_XML% --gpsdat-output %SUMO_CSV% --begin %SIM_BEGIN% --end %SIM_END%

```

# 在 server 上使用 NS3 的相關動作

5. ./waf --run "test09 --traceFile=scratch/test09.tcl --nodeNum=100 --duration=60 --selectedEnb=0 --outputDir=test09-4"
在 server 上產生結果
./waf --run "scratch/lteudp_multi_udp --numberOfUes=5 --startUe=0 --endUe=4 --isSdnEnabled=true --isDownlink=true --isUplink=true --interAppInterval=1 --interPacketInterval=1000" > test.log

cd sumo-learning
git pull
cp test09/test09.cc ../scratch/
cp test09/test09.tcl ../scratch/
cd ..
./waf --run "test09 --traceFile=scratch/test09.tcl --nodeNum=100 --duration=1000 --selectedEnb=0 --outputDir=test09-4"

./waf --run "test09-delay --traceFile=scratch/test09.tcl --nodeNum=100 --duration=1000 --selectedEnb=0 --outputDir=test09-4"

```bash

cd sumo-learning
git pull
cp test08/test08.cc ../scratch/
cp test08/test08.tcl ../scratch/
cd ..
# ./waf
# export NS_LOG=FirstScriptExample=info
./waf --run "scratch/test08"


```

```bash

cd sumo-learning
git pull
cp test09/test09-course.cc ../scratch/
cp test09/test09.tcl ../scratch/
cd ..
# ./waf
# export NS_LOG=FirstScriptExample=info
./waf --run "scratch/test09-course"

```


cd sumo-learning
git pull
cp test07/test07.cc ../scratch/
cp test07/test07-delay.cc ../scratch/
cp test07/test07.tcl ../scratch/
cd ..
./waf --run test07-delay


./waf clean
./waf configure --build-profile=optimized --enable-examples --enable-tests

./waf clean
./waf configure --build-profile=debug --enable-examples --enable-tests

./waf --run "test05 --traceFile=scratch/test05.tcl --nodeNum=100 --duration=60 --selectedEnb=3"
CXXFLAGS="-std=c++0x" ./waf configure
./waf clean

top -n 1 | grep nxshen | cut -c 1-15

列出 NS_LOG_COMPONENT_DEFINE
find . -name '*.cc' | xargs grep NS_LOG_COMPONENT_DEFINE

搜尋特定 component 
find . -name '*.cc' | xargs grep NS_LOG_COMPONENT_DEFINE | grep -i sink

grep -i 為 忽略大小寫

測試用的變數設定
export NS_LOG=UdpL4Protocol
    ./waf --run first

