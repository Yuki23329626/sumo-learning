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
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/csma-module.h"
#include "ns3/ssid.h"

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

         ENB                                   Core network
  *    *    *    *                    SW   SW   SW  SDNC IPFS ORACLE
  |    |    |    |   Point-To-Point    |    |    |    |    |    |
 n3   n2   n1   n0 -----------------  n12  n13  n14  n15  n16  n17
                                       |    |    |    |    |    | 
                                      ============================
                                              LAN 10.1.1.0
*/

class UE_Info
{
public:
  void set_imsi(int imsi)
  {
    if (imsi >= 0)
      this->imsi = imsi;
  }

  void set_Position(Vector position)
  {
    this->position = position;
  }

  void CourseChange(Ptr<const MobilityModel> mobility)
  {
    Vector pos = mobility->GetPosition(); // Get position
    position = pos;
    double now = Simulator::Now().GetSeconds();
    *ofstream1 << now << "," << imsi  << "," << position.x << "," << position.y << endl;
  }

  // 拿到 UE 當前座標 position.x, position.y
  Vector get_Position()
  {
    return position;
  }

  // 使用 object 的 output stream
  void set_output(std::ofstream *ofstream1)
  {
      this->ofstream1 = ofstream1;
  }

private:
    int imsi = -1;
    Vector position;
    std::ofstream *ofstream1;
};

int main(int argc, char *argv[])
{
  string traceFile = "scratch/test09.tcl";
  int nCsma = 3;
  int nNode = 100;
  int duration = 1001;
  string outputDir = "test09-course-1";
  string outputFileName = "test09-course.csv";
  
  Ptr<MobilityModel> ueMobilityModel;
  Ptr<LteUePhy> uephy;

  CommandLine cmd;
  cmd.AddValue("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue("nNode", "Number of nodes", nNode);
  cmd.AddValue("duration", "Duration of Simulation", duration);
  cmd.AddValue("outputDir", "Output directory", outputDir);
  cmd.Parse(argc, argv);

  // Enable logging from the ns2 helper
  LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_DEBUG);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  char char_output_dir[outputDir.length()+1];
  strcpy(char_output_dir, outputDir.c_str());
  mkdir(char_output_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  outputFileName = outputDir + "/" + outputFileName;
  
  std::ofstream ofstream1;
  ofstream1.open(outputFileName);
  ofstream1 << "time(s),IMSI,X,Y" << endl;

  UE_Info *ue_info = (UE_Info *)malloc(sizeof(UE_Info) * nNode);
  Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile);

  NodeContainer ueNodes;
  ueNodes.Create(nNode);

  ns2.Install();

  NodeContainer p2pNodes[12];
  for (int i = 0; i < 12; i++)
  {
    p2pNodes[i].Create (2);
  }

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices[12];
  for (int i = 0; i < 12; i++)
  {
    p2pDevices[i] = pointToPoint.Install (p2pNodes[i]);
  }

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes[0].Get (0));
  csmaNodes.Add (p2pNodes[1].Get (0));
  csmaNodes.Add (p2pNodes[2].Get (0));
  csmaNodes.Add (p2pNodes[3].Get (0));
  csmaNodes.Add (p2pNodes[4].Get (0));
  csmaNodes.Add (p2pNodes[5].Get (0));
  csmaNodes.Add (p2pNodes[6].Get (0));
  csmaNodes.Add (p2pNodes[7].Get (0));
  csmaNodes.Add (p2pNodes[8].Get (0));
  csmaNodes.Add (p2pNodes[9].Get (0));
  csmaNodes.Add (p2pNodes[10].Get (0));
  csmaNodes.Add (p2pNodes[11].Get (0));
  csmaNodes.Create(nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NodeContainer enbNodes;
  enbNodes.Add (p2pNodes[0].Get (1));
  enbNodes.Add (p2pNodes[1].Get (1));
  enbNodes.Add (p2pNodes[2].Get (1));
  enbNodes.Add (p2pNodes[3].Get (1));
  enbNodes.Add (p2pNodes[4].Get (1));
  enbNodes.Add (p2pNodes[5].Get (1));
  enbNodes.Add (p2pNodes[6].Get (1));
  enbNodes.Add (p2pNodes[7].Get (1));
  enbNodes.Add (p2pNodes[8].Get (1));
  enbNodes.Add (p2pNodes[9].Get (1));
  enbNodes.Add (p2pNodes[10].Get (1));
  enbNodes.Add (p2pNodes[11].Get (1));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  // Install netdevice
  NetDeviceContainer ueDevices;
  ueDevices = wifi.Install (phy, mac, ueNodes);
  
  NetDeviceContainer enbDevices;
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  enbDevices = wifi.Install (phy, mac, enbNodes);

  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();

  enbPositionAlloc->Add(Vector(583, 365, 0));
  enbPositionAlloc->Add(Vector(885, 338, 0));
  enbPositionAlloc->Add(Vector(1187, 328, 0));
  enbPositionAlloc->Add(Vector(1305, 322, 0));
  enbPositionAlloc->Add(Vector(596, 703, 0));
  enbPositionAlloc->Add(Vector(895, 690, 0));
  enbPositionAlloc->Add(Vector(1100, 682, 0));
  enbPositionAlloc->Add(Vector(1317, 679, 0));
  enbPositionAlloc->Add(Vector(602, 972, 0));
  enbPositionAlloc->Add(Vector(908, 955, 0));
  enbPositionAlloc->Add(Vector(1107, 953, 0));
  enbPositionAlloc->Add(Vector(1317, 950, 0));

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(enbPositionAlloc);
  mobility.Install(enbNodes);

  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (ueNodes);
  stack.Install (enbNodes);

  for (int i = 0; i < nNode; i++)
  {
    ueMobilityModel = ueNodes.Get(i)->GetObject<MobilityModel>();
    ue_info[i].set_Position(ueMobilityModel->GetPosition());
    // ue_info[i].setConnectedENB(SELECTED_ENB);
    ue_info[i].set_output(&ofstream1);

    // uephy = ueDevices.Get(i)->GetObject<LteUeNetDevice>()->GetPhy();
    // ue_info[i].set_imsi(ueDevices.Get(i)->GetObject<LteUeNetDevice>()->GetImsi());

    // uephy->TraceConnectWithoutContext("ReportCurrentCellRsrpSinr", MakeCallback(&UEs_Info::GetUeSinr, &ues_info[i]));
    ueMobilityModel->TraceConnectWithoutContext("CourseChange", MakeCallback(&UE_Info::CourseChange, &ue_info[i]));
  }

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);
  
  address.SetBase ("10.1.2.0", "255.255.255.0");
  address.Assign (ueDevices);
  address.Assign (enbDevices);

  Ipv4InterfaceContainer p2pInterfaces[12];
  address.SetBase ("10.1.3.0", "255.255.255.0");
  p2pInterfaces[0] = address.Assign (p2pDevices[0]);
  address.SetBase ("10.1.4.0", "255.255.255.0");
  p2pInterfaces[1] = address.Assign (p2pDevices[1]);
  address.SetBase ("10.1.5.0", "255.255.255.0");
  p2pInterfaces[2] = address.Assign (p2pDevices[2]);
  address.SetBase ("10.1.6.0", "255.255.255.0");
  p2pInterfaces[3] = address.Assign (p2pDevices[3]);
  address.SetBase ("10.1.7.0", "255.255.255.0");
  p2pInterfaces[4] = address.Assign (p2pDevices[4]);
  address.SetBase ("10.1.8.0", "255.255.255.0");
  p2pInterfaces[5] = address.Assign (p2pDevices[5]);
  address.SetBase ("10.1.9.0", "255.255.255.0");
  p2pInterfaces[6] = address.Assign (p2pDevices[6]);
  address.SetBase ("10.1.10.0", "255.255.255.0");
  p2pInterfaces[7] = address.Assign (p2pDevices[7]);
  address.SetBase ("10.1.11.0", "255.255.255.0");
  p2pInterfaces[8] = address.Assign (p2pDevices[8]);
  address.SetBase ("10.1.12.0", "255.255.255.0");
  p2pInterfaces[9] = address.Assign (p2pDevices[9]);
  address.SetBase ("10.1.13.0", "255.255.255.0");
  p2pInterfaces[10] = address.Assign (p2pDevices[10]);
  address.SetBase ("10.1.14.0", "255.255.255.0");
  p2pInterfaces[11] = address.Assign (p2pDevices[11]);


  // udp server
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (2));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  // udp client
  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (2), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
  echoClient.Install (ueNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  Simulator::Stop(Seconds(duration));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}