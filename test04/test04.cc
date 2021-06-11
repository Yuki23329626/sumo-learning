#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <string>
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

    if (now >= 10 && now < 11 )
      *os1 <<now <<","<< imsi << "," << this->sinr  <<"," << position.x <<"," <<position.y<<","<< connected_eNB << endl;
  }

  // When UE move change the UE position // 沒有人看得懂的英文
  // 當 UE 更換位置時，請確認 course change 是哪種 change
  void CourseChange1(Ptr<const MobilityModel> mobility)
  {
    Vector pos = mobility->GetPosition(); // Get position
    position = pos;
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
  void set_output(std::ofstream *os1)
  {
    this->os1 = os1;
  }

private:
  int imsi = -1;
  int connected_eNB;
  double sinr = 0.0;
  double last_gt = 0.0; // 不知道是甚麼
  Vector position, enb_position;
  std::ofstream *os1;
};

int main(int argc, char *argv[])
{
  string TRACE_FILE = "scratch/test04.tcl";

  int NODE_NUM = 300; // UE 數量
  int BANDWIDTH = 100; // number of RB ,10MHz
  int ENB_NUM = 35; // 設置的 eNB 數量

  int SELECTED_ENB = 1; // 要測試哪一個 enb

  double DURATION = 60;   // 貌似是秒數
  double ENB_TX_POWER = 20; // Transimission power in doubleBm, doubleBm 不知道是甚麼

  Ptr<LteUePhy> uephy;
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

  Ptr<MobilityModel> ueMobilityModel;

  AsciiTraceHelper asciiTraceHelper;
  std::ofstream outputfile1;

  outputfile1.open("test04.csv");
  outputfile1 << "Time_sec,IMSI,SINR,X,Y,Selected_eNB" << endl;

  // Enable logging from the ns2 helper
  LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_DEBUG);

  CommandLine cmd; // 不知道是甚麼
  cmd.AddValue("traceFile", "Ns2 movement trace file", TRACE_FILE);
  cmd.AddValue("NODE_NUM", "Number of nodes", NODE_NUM);
  cmd.AddValue("duration", "Duration of Simulation", DURATION);
  cmd.Parse(argc, argv);

  // Set the default Configure
  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(ENB_TX_POWER));
  Config::SetDefault("ns3::LteUePhy::TxPower", DoubleValue(20.0)); // Transimt power in dBm
  Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue(true));
  Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue(true));

  Config::SetDefault("ns3::LteEnbMac::NumberOfRaPreambles", UintegerValue(10));    // when too much ue will 公尛
  Config::SetDefault("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue(2)); // MIMO Tx diversity

  // path loss model
  Config::SetDefault("ns3::LteHelper::PathlossModel", StringValue("ns3::OkumuraHataPropagationLossModel"));
  Config::SetDefault("ns3::OkumuraHataPropagationLossModel::Frequency", DoubleValue(2000000000));

  Config::SetDefault("ns3::LteUePowerControl::ClosedLoop", BooleanValue(true));
  Config::SetDefault("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue(true));
  Config::SetDefault("ns3::LteUePowerControl::Alpha", DoubleValue(1.0));
  Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320)); // he SRS periodicity in num TTIs

  lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(BANDWIDTH));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(BANDWIDTH));
  // Set antenna type
  lteHelper->SetEnbAntennaModelType("ns3::IsotropicAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute("Gain", DoubleValue(1.0));

  UEs_Info *ues_info = (UEs_Info *)malloc(sizeof(UEs_Info) * NODE_NUM);
  Ns2MobilityHelper ns2 = Ns2MobilityHelper(TRACE_FILE);

  // Create UE nodes.
  NodeContainer ueNode;
  ueNode.Create(NODE_NUM);

  // configure movements for each node, while reading trace file
  ns2.Install();

  // Create enbNode
  NodeContainer enbNode;
  enbNode.Create(ENB_NUM);

  // Set eNB position
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
  // 不知道這個 allocation 的用法，待查證
  enbPositionAlloc->Add(Vector(500.0, 8.00, 0));
  
  for (int j = 1; j < 6; j++)
  {
    for (int i = 1; i < 8; i++)
    {
      Vector enbPosition(700 + 500*i, 1900 + 500*j, 0);
      enbPositionAlloc->Add(enbPosition);
    }
  }

  // Install mobility
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator(enbPositionAlloc);
  enbMobility.Install(enbNode);

  // Install netdevice
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  enbDevs = lteHelper->InstallEnbDevice(enbNode);
  ueDevs = lteHelper->InstallUeDevice(ueNode);

  Ptr<LtePhy> enbPhy = enbDevs.Get(SELECTED_ENB)->GetObject<LteEnbNetDevice>()->GetPhy()->GetObject<LtePhy>();

  // we install the IP stack on the UEs
  InternetStackHelper internet1;
  internet1.Install(ueNode);

  // Set the power density
  Ptr<SpectrumValue> psd = enbPhy->CreateTxPowerSpectralDensity();
  (*psd) = 4.14e-21;
  Ptr<LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get(SELECTED_ENB)->GetObject<LteEnbNetDevice>()->GetPhy()->GetDownlinkSpectrumPhy()->GetObject<LteSpectrumPhy>();
  enbDlSpectrumPhy->SetNoisePowerSpectralDensity(psd);

  // 有必要的話更換連接的 ENB ID(SELECTED_ENB)
  for (int i = 0; i < NODE_NUM; i++)
  {
    lteHelper->Attach(ueDevs.Get(i), enbDevs.Get(SELECTED_ENB));
  }

  for (int i = 0; i < NODE_NUM; i++)
  {
    // 從 SUMO 讀進來的路線，設置進去 ues_info[i] 裡
    ueMobilityModel = ueNode.Get(i)->GetObject<MobilityModel>();
    ues_info[i].set_Position(ueMobilityModel->GetPosition());

    // 不知道為什麼要 +1，應該是 eNB 的編號，車輛在 SUMO 裡應該也是 1 開始編號
    ues_info[i].setConnectedENB(SELECTED_ENB + 1);
    ues_info[i].set_output(&outputfile1);

    // 車輛 IMSI，車輛編號
    uephy = ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetPhy();
    ues_info[i].set_imsi(ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi());

    // 設置 callback function，當 SINR 或是 position 有變動的時候，會分別呼叫這兩個 member function
    // 請確認，SINR 變動是怎麼偵測的，是偵測 eNB 的 SINR 嗎，如果是的話，有針對哪個 eNB 嗎?
    uephy->TraceConnectWithoutContext("ReportCurrentCellRsrpSinr", MakeCallback(&UEs_Info::GetUeSinr, &ues_info[i]));
    ueMobilityModel->TraceConnectWithoutContext("CourseChange", MakeCallback(&UEs_Info::CourseChange1, &ues_info[i]));
  }

  // 不知道這是啥
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer(q);
  lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

  // AnimationInterface anim("animation.xml"); // jhon 教學投影片裡的東西，也許用不到，動畫模擬之類的東西，不過 NS3 找不到
  Simulator::Stop(Seconds(DURATION));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}