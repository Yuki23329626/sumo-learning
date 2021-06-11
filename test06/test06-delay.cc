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

class UEs_Info
{
public:

  // 就設定 UEs_Info object 的 IMSI
  void set_imsi(int imsi)
  {
    if (imsi >= 0)
      this->imsi = imsi;
  }

  // 設定位置 二維坐標系
  void set_Position(Vector position)
  {
    this->position = position;
  }

  // 拿到 UE 的 SINR
  void GetUeSinr(uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
  {
    // 模擬的時間
    double now = Simulator::Now().GetSeconds();

    // SINR，感覺單位是分貝
    this->sinr = 10 * log10(sinr);
    if (now > 880 && now <= 1000 )
      *os1 << now << "," << imsi << "," << this->sinr  << "," << position.x << "," << position.y << "," << connected_eNB << endl;
  }

  // When UE move change the UE position // 沒有人看得懂的英文
  // 當 UE 更換位置時，請確認 course change 是哪種 change
  void CourseChange1(Ptr<const MobilityModel> mobility)
  {
    Vector pos = mobility->GetPosition(); // Get position
    position = pos;
    double now = Simulator::Now().GetSeconds();
    this->sinr = 10 * log10(sinr);
    if (now > 880 && now <= 1000 )
      *os2 << now << "," << imsi << "," << this->sinr  << "," << position.x << "," << position.y << "," << connected_eNB << endl;
  }

  // 要連哪個 ENB
  void setConnectedENB(int c)
  {
    connected_eNB = c;
  }

  // 拿到 UE 當前座標 position.x, position.y
  Vector get_Position()
  {
    return position;
  }

  // 使用 object 的 output stream
  void set_output(std::ofstream *os1, std::ofstream *os2)
  {
    this->os1 = os1;
    this->os2 = os2;
  }

private:
  int imsi = -1;
  int connected_eNB=-1;
  double sinr = 0.0;
  double last_gt = 0.0; // 不知道是甚麼
  Vector position, enb_position;
  std::ofstream *os1;
  std::ofstream *os2;
};

int main(int argc, char *argv[])
{
  string TRACE_FILE = "scratch/test06.tcl";

  // 以下部分變數不能加上 const，因為 cmd 那邊要進行設定
  // int NODE_NUM = 100; // UE 數量
  // int BANDWIDTH = 100; // number of RB ,10MHz
  // int ENB_NUM = 35; // 設置的 eNB 數量

  // int SELECTED_ENB = 0; // 要測試哪一個 enb

  double DURATION = 10;   // 貌似是秒數
  // double ENB_TX_POWER = 20; // Transimission power in doubleBm, doubleBm 不知道是甚麼

  // Ptr<LteUePhy> uephy;
  // Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

  // Ptr<MobilityModel> ueMobilityModel;

  // AsciiTraceHelper asciiTraceHelper;
  // std::ofstream outputfile1;
  // std::ofstream outputfile2;

  // string OUTPUT_FILE = "test06_enb0.csv";
  // string OUTPUT_FILE2 = "test06_enb1.csv";
  // string OUTPUT_DIR = "output_csv";

  // // Enable logging from the ns2 helper
  // LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_DEBUG);
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // CommandLine cmd; // 不知道是甚麼
  // cmd.AddValue("traceFile", "Ns2 movement trace file", TRACE_FILE);
  // cmd.AddValue("nodeNum", "Number of nodes", NODE_NUM);
  // cmd.AddValue("duration", "Duration of Simulation", DURATION);
  // cmd.AddValue("selectedEnb", "Select eNB ID", SELECTED_ENB);
  // cmd.AddValue("outputDir", "output directory", OUTPUT_DIR);
  // cmd.Parse(argc, argv);

  // char CHAR_OUTPUT_DIR[OUTPUT_DIR.length()+1];
  // strcpy(CHAR_OUTPUT_DIR, OUTPUT_DIR.c_str());
  // mkdir(CHAR_OUTPUT_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  // OUTPUT_FILE = OUTPUT_DIR + "/" + to_string(SELECTED_ENB) + "_" + OUTPUT_FILE;
  // OUTPUT_FILE2 = OUTPUT_DIR + "/" + to_string(SELECTED_ENB) + "_" + OUTPUT_FILE2;

  // outputfile1.open(OUTPUT_FILE);
  // outputfile2.open(OUTPUT_FILE2);
  // outputfile1 << "Time_sec,IMSI,SINR,X,Y,Selected_eNB" << endl;
  // outputfile2 << "Time_sec,IMSI,SINR,X,Y,Selected_eNB" << endl;


  // // Set the default Configure
  // Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(ENB_TX_POWER));
  // Config::SetDefault("ns3::LteUePhy::TxPower", DoubleValue(20.0)); // Transimt power in dBm
  // Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue(true));
  // Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue(true));

  // Config::SetDefault("ns3::LteEnbMac::NumberOfRaPreambles", UintegerValue(10));    // when too much ue will 公尛
  // Config::SetDefault("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue(2)); // MIMO Tx diversity

  // // path loss model
  // Config::SetDefault("ns3::LteHelper::PathlossModel", StringValue("ns3::OkumuraHataPropagationLossModel"));
  // Config::SetDefault("ns3::OkumuraHataPropagationLossModel::Frequency", DoubleValue(2000000000));

  // Config::SetDefault("ns3::LteUePowerControl::ClosedLoop", BooleanValue(true));
  // Config::SetDefault("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue(true));
  // Config::SetDefault("ns3::LteUePowerControl::Alpha", DoubleValue(1.0));
  // Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320)); // he SRS periodicity in num TTIs

  // 建立
  NodeContainer p2pNodes[7];
  for(int i=0;i<7;i++){
    p2pNodes[i].Create(2);
  }

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices[7];
  for(int i=0;i<7;i++){
    p2pDevices[i] = pointToPoint.Install (p2pNodes[i]);
  }

  // 建立 csma sdn controller 跟 switch，IPFS，ORACLE 相關的節點 container
  NodeContainer csmaCoreNodes;
  for(int i=0;i<7;i++){
    csmaCoreNodes.Add(p2pNodes[i].Get(0));
  }
  csmaCoreNodes.Create (3);

  // eNB 相關的節點 container
  NodeContainer csmaNodes[7];
  for(int i=0;i<7;i++){
    csmaNodes[i].Add(p2pNodes[i].Get(1));
    csmaNodes[i].Create(4);
  }

  // 乙太網路常用的 CSMA/CA 方法，應該是 mac layer 層的 protocol
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaCoreDevices;
  csmaCoreDevices = csma.Install (csmaCoreNodes);

  NetDeviceContainer csmaDevices[7];
  for(int i=0;i<7;i++){
    csmaDevices[i] = csma.Install (csmaNodes[i]);
  }

  // LteHelper lte;
  // lte.SetEnbDeviceAttribute("DlBandwidth", UintegerValue(BANDWIDTH));
  // lte.SetEnbDeviceAttribute("UlBandwidth", UintegerValue(BANDWIDTH));
  // lte.SetEnbAntennaModelType("ns3::IsotropicAntennaModel");
  // lte.SetEnbAntennaModelAttribute("Gain", DoubleValue(1.0));

  // NodeContainer lteEnbNodes[7];
  // for(int i=0;i<5;i++){
  //   lteEnbNodes[0].Add(csmaNodes[0].Get(i));
  //   lteEnbNodes[1].Add(csmaNodes[1].Get(i));
  //   lteEnbNodes[2].Add(csmaNodes[2].Get(i));
  //   lteEnbNodes[3].Add(csmaNodes[3].Get(i));
  //   lteEnbNodes[4].Add(csmaNodes[4].Get(i));
  //   lteEnbNodes[5].Add(csmaNodes[5].Get(i));
  //   lteEnbNodes[6].Add(csmaNodes[6].Get(i));
  // }
  // for(int i=0;i<7;i++){
  //   lteEnbNodes[i].Create(5);
  // }

  // NetDeviceContainer lteEnbDevices[7];
  // for(int i=0;i<7;i++){
  //   lteEnbDevices[i] = lte.InstallEnbDevice(lteEnbNodes[i]);
  // }

  InternetStackHelper stack;
  stack.Install (csmaCoreNodes);
  for(int i=0;i<7;i++){
    stack.Install (csmaNodes[i]);
  }
  // for(int i=0;i<7;i++){
  //   stack.Install (p2pNodes[i]);
  // }
  // for(int i=0;i<7;i++){
  //   stack.Install (lteEnbNodes[i]);
  // }

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterface[7];
  p2pInterface[0] = address.Assign (p2pDevices[0]);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  p2pInterface[1] = address.Assign (p2pDevices[1]);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  p2pInterface[2] = address.Assign (p2pDevices[2]);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  p2pInterface[3] = address.Assign (p2pDevices[3]);

  address.SetBase ("10.1.5.0", "255.255.255.0");
  p2pInterface[4] = address.Assign (p2pDevices[4]);

  address.SetBase ("10.1.6.0", "255.255.255.0");
  p2pInterface[5] = address.Assign (p2pDevices[5]);

  address.SetBase ("10.1.7.0", "255.255.255.0");
  p2pInterface[6] = address.Assign (p2pDevices[6]);

  address.SetBase ("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaCoreInterfaces;
  csmaCoreInterfaces = address.Assign (csmaCoreDevices);

  address.SetBase ("10.1.9.0", "255.255.255.0");
  address.Assign (csmaDevices[0]);

  address.SetBase ("10.1.10.0", "255.255.255.0");
  address.Assign (csmaDevices[1]);

  address.SetBase ("10.1.11.0", "255.255.255.0");
  address.Assign (csmaDevices[2]);

  address.SetBase ("10.1.12.0", "255.255.255.0");
  address.Assign (csmaDevices[3]);

  address.SetBase ("10.1.13.0", "255.255.255.0");
  address.Assign (csmaDevices[4]);

  address.SetBase ("10.1.14.0", "255.255.255.0");
  address.Assign (csmaDevices[5]);

  address.SetBase ("10.1.15.0", "255.255.255.0");
  address.Assign (csmaDevices[6]);

  UdpEchoServerHelper echoServerIPFS (9);

  ApplicationContainer serverApps = echoServerIPFS.Install (csmaCoreNodes.Get(8));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaCoreInterfaces.GetAddress(8), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install(csmaNodes[0].Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  // lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(BANDWIDTH));
  // lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(BANDWIDTH));
  // // Set antenna type
  // lteHelper->SetEnbAntennaModelType("ns3::IsotropicAntennaModel");
  // lteHelper->SetEnbAntennaModelAttribute("Gain", DoubleValue(1.0));

  // UEs_Info *ues_info = (UEs_Info *)malloc(sizeof(UEs_Info) * NODE_NUM);
  // Ns2MobilityHelper ns2 = Ns2MobilityHelper(TRACE_FILE);

  // // Create UE nodes.
  // NodeContainer ueNode;
  // ueNode.Create(NODE_NUM);

  // // configure movements for each node, while reading trace file
  // ns2.Install();

  // // Create enbNode
  // NodeContainer enbNode;
  // enbNode.Create(ENB_NUM);

  // // Set eNB position
  // Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
  // // 不知道這個 allocation 的用法，待查證
  // enbPositionAlloc->Add(Vector(500.0, 8.00, 0));
  
  // for (int j = 1; j < 6; j++)
  // {
  //   for (int i = 1; i < 8; i++)
  //   {
  //     Vector enbPosition(1300 + 200*i, 250 + 200*j, 0);
  //     enbPositionAlloc->Add(enbPosition);
  //   }
  // }

  // // Install mobility
  // MobilityHelper enbMobility;
  // enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  // enbMobility.SetPositionAllocator(enbPositionAlloc);
  // enbMobility.Install(enbNode);

  // // Install netdevice
  // NetDeviceContainer enbDevs;
  // NetDeviceContainer ueDevs;
  // enbDevs = lteHelper->InstallEnbDevice(enbNode);
  // ueDevs = lteHelper->InstallUeDevice(ueNode);

  // Ptr<LtePhy> enbPhy = enbDevs.Get(SELECTED_ENB)->GetObject<LteEnbNetDevice>()->GetPhy()->GetObject<LtePhy>();

  // // we install the IP stack on the UEs
  // InternetStackHelper internet1;
  // internet1.Install(ueNode);

  // // Set the power density
  // Ptr<SpectrumValue> psd = enbPhy->CreateTxPowerSpectralDensity();
  // (*psd) = 4.14e-21;
  // Ptr<LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get(SELECTED_ENB)->GetObject<LteEnbNetDevice>()->GetPhy()->GetDownlinkSpectrumPhy()->GetObject<LteSpectrumPhy>();
  // enbDlSpectrumPhy->SetNoisePowerSpectralDensity(psd);

  // // 有必要的話更換連接的 ENB ID(SELECTED_ENB)
  // for (int i = 0; i < NODE_NUM; i++)
  // {
  //   lteHelper->Attach(ueDevs.Get(i), enbDevs.Get(SELECTED_ENB));
  // }

  // for (int i = 0; i < NODE_NUM; i++)
  // {
  //   // 從 SUMO 讀進來的路線，設置進去 ues_info[i] 裡
  //   ueMobilityModel = ueNode.Get(i)->GetObject<MobilityModel>();
  //   ues_info[i].set_Position(ueMobilityModel->GetPosition());

  //   // 不知道為什麼要 +1，應該是 eNB 的編號，車輛在 SUMO 裡應該也是 1 開始編號 // 是從 0 開始編號
  //   ues_info[i].setConnectedENB(SELECTED_ENB);
  //   ues_info[i].set_output(&outputfile1, &outputfile2);

  //   // 車輛 IMSI，車輛編號
  //   uephy = ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetPhy();
  //   ues_info[i].set_imsi(ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi());

  //   // 設置 callback function，當 SINR 或是 position 有變動的時候，會分別呼叫這兩個 member function
  //   // 請確認，SINR 變動是怎麼偵測的，是偵測 eNB 的 SINR 嗎，如果是的話，有針對哪個 eNB 嗎?
  //   uephy->TraceConnectWithoutContext("ReportCurrentCellRsrpSinr", MakeCallback(&UEs_Info::GetUeSinr, &ues_info[i]));
  //   ueMobilityModel->TraceConnectWithoutContext("CourseChange", MakeCallback(&UEs_Info::CourseChange1, &ues_info[i]));
  // }

  // // 不知道這是啥
  // enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  // EpsBearer bearer(q);
  // lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

  // AnimationInterface anim("animation.xml"); // jhon 教學投影片裡的東西，也許用不到，動畫模擬之類的東西，不過 NS3 找不到
  Simulator::Stop(Seconds(DURATION));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}