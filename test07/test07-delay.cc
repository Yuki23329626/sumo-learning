#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/lte-module.h"
#include "ns3/network-module.h"
#include "ns3/config-store.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/animation-interface.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"

using namespace ns3;
using namespace std;

/*
目前要做甚麼
找一個城市的地圖，目前暫定台北市，1km*1km 的大小，兩百公尺設置一個 eNB，約 40 個好了
UE 約 300 台
有辦法找到，在某個特定時間點，哪個 UE 會選擇哪個 eNB
目前有兩個 callback function，針對 SINR 跟 Mobility
目前應該是先考慮 Mobility，不過不確定這是不是位置移動就會 callback ，待確認
網路節點需要：blockchain node，IPFS，ORACLE 之類的節點，互相之間的傳輸延遲
考慮上傳跟下載所需的時間，怎麼算
確認 SDN 換手需要哪些資訊，要跟那些節點溝通，抽象化計算，比較傳統 hndover 跟 SDN handover 的時間
handover 考慮 prepare time 跟 completion time，也許可以比較
*/

/*
          LTE
                    eNB                 0    1    2    3    4    5    6    7    8    9
 *    *    *    *    *                 SW   SW   SW   SW   SW   SW   SW  SDNC IPFS ORACLE
 |    |    |    |    |     10.1.1.0     |    |    |    |    |    |    |    |    |    |
n4 - n3 - n2 - n1 - n0 --------------  n35  n36  n37  n38  n39  n40  n41  n42  n43  n44
                              LAN       |    |    |    |    |    |    |    |    |    |
                                       ================================================
                                                         LAN 10.1.8.0
*/

int main(int argc, char *argv[])
{
  double DURATION = 10;   // 是秒數
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer csmaCoreNodes;
  csmaCoreNodes.Create(2);

  csmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaCoreDevices;
  csma.Install(csmaCoreNodes);

  InternetStackHelper stack;
  stack.Install (csmaCoreNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer csmaCoreInterface;

  Simulator::Stop(Seconds(DURATION));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}