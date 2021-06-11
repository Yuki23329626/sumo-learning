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

using namespace ns3;
using namespace std;

class UEs_Info{
    public:

        void
        set_imsi(int imsi){
            if(imsi >= 0)
                this->imsi = imsi;
        }

        void
        set_Position(Vector position){
            this->position = position;
        }

        /*double
        get_sinr(){
            return sinr;
        }

        double
        get_distance(){
            return distance;
        }

        double
        get_imsi(){
            return imsi;
        }

        double
        get_x(){
            return position.x;
        }

        double
        get_y(){
            return position.y;
        }*/

        /////TraceFunction
        void
        GetUeSinr(uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId){
          double now = Simulator::Now().GetSeconds();
          //bool print = false ;
          this->sinr = 10*log10(sinr);
          //if ( counter %1000 == 0 ){

          if (now >= 223 && now < 224 )
            *os4 <<now <<","<< imsi << "," << this->sinr  <<"," << position.x <<"," <<position.y<<","<< connectenb << endl;
          else if ( now >= 266 && now < 267) 
            *os1 <<now <<","<< imsi << "," << this->sinr  <<"," << position.x <<"," <<position.y<<","<< connectenb << endl;
          else if ( now >= 267 && now < 268)
            *os2 <<now <<","<< imsi << "," << this->sinr  <<"," << position.x <<"," <<position.y<<","<< connectenb << endl;
          else if ( now >= 268 && now < 269)
            *os3 <<now <<","<< imsi << "," << this->sinr  <<"," << position.x <<"," <<position.y<<","<< connectenb << endl;
          /*else if ( now >= 406 && now < 406)
            *os5 <<now <<","<< imsi << "," << this->sinr  <<"," << position.x <<"," <<position.y<<","<< connectenb << endl;*/


        }

        //When ue move change the ue position
        void
        CourseChange1 ( Ptr<const MobilityModel> mobility)
        {
           Vector pos = mobility->GetPosition (); // Get position
           position = pos ;
        }

        /*void setCounter(int c){
          counter = c;
        }*/
        void setConnectenb(int c){
          connectenb = c;
        }

        Vector get_Position(){
            return position;
        }

        void set_output(std::ofstream *os1, std::ofstream *os2, std::ofstream *os3 ,std::ofstream *os4){
          this->os1 = os1;
          this->os2 = os2;
          this->os3 = os3;
          this->os4 = os4;
        }
       
    private:
        int imsi = -1;
        //int counter;
        int connectenb;
        //double distance = 0.0;
        double sinr = 0.0;
        double last_gt = 0.0 ;
        Vector position ,enb_position;
        std::ofstream *os1;
        std::ofstream *os2;
        std::ofstream *os3;
        std::ofstream *os4;
};


int main (int argc, char *argv[])
{
  string traceFile = "scratch/300node.420s.tcl";
 
  int    nodeNum = 300;
  int    bandwidth = 100;       //num of RB ,10MHz
  int    enbNum = 5;
  int    connected = 4;

  double duration = 269;         //50 seconds
  double eNbTxPower = 20 ;      //Transimission power in doubleBm

  Ptr<LteUePhy> uephy;
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  Ptr<MobilityModel> ueMobilityModel;

  AsciiTraceHelper asciiTraceHelper;
  std::ofstream outputfile1;
  std::ofstream outputfile2;
  std::ofstream outputfile3;
  std::ofstream outputfile4;

  //std::ofstream outputfile5;

  outputfile1.open("enb5_266s.csv");
  outputfile2.open("enb5_267s.csv");
  outputfile3.open("enb5_268s.csv");
  outputfile4.open("enb5_223s.csv");
  //outputfile5.open("enb2_306s.csv");

  outputfile1<<"Time_s,Imsi,Sinr,x,y,Cellid"<<endl;
  outputfile2<<"Time_s,Imsi,Sinr,x,y,Cellid"<<endl;
  outputfile3<<"Time_s,Imsi,Sinr,x,y,Cellid"<<endl;
  outputfile4<<"Time_s,Imsi,Sinr,x,y,Cellid"<<endl;
  //outputfile5<<"Time_s,Imsi,Sinr,x,y,Cellid"<<endl;

  // Enable logging from the ns2 helper
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  CommandLine cmd;
  cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue ("nodeNum", "Number of nodes", nodeNum);
  cmd.AddValue ("duration", "Duration of Simulation", duration);
  cmd.Parse (argc,argv);

  // Set the default Configure
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (eNbTxPower));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (20.0));                        //Transimt power in dbm
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (true));
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (true));
 
  Config::SetDefault ("ns3::LteEnbMac::NumberOfRaPreambles", UintegerValue(10));            //when too much ue will 
  Config::SetDefault ("ns3::LteEnbRrc::DefaultTransmissionMode",UintegerValue(2));          //MIMO Tx diversity

  //path loss model
  Config::SetDefault ("ns3::LteHelper::PathlossModel",StringValue("ns3::OkumuraHataPropagationLossModel"));
  Config::SetDefault ("ns3::OkumuraHataPropagationLossModel::Frequency", DoubleValue(2000000000)); 
  
  //path loss logdistancePropagation
   /* Config::SetDefault ("ns3::LteHelper::PathlossModel",StringValue("ns3::LogDistancePropagationLossModel"));
  Config::SetDefault ("ns3::LogDistancePropagationLossModel::Exponent", DoubleValue(3.0));  //Set the model parameter
  Config::SetDefault ("ns3::LogDistancePropagationLossModel::ReferenceDistance", DoubleValue(1.0));*/

  Config::SetDefault ("ns3::LteUePowerControl::ClosedLoop", BooleanValue (true));
  Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteUePowerControl::Alpha", DoubleValue (1.0));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));              //he SRS periodicity in num TTIs

  lteHelper->SetEnbDeviceAttribute("DlBandwidth",UintegerValue(bandwidth));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth",UintegerValue(bandwidth)); 
  //Set antenna type
  lteHelper->SetEnbAntennaModelType("ns3::IsotropicAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute("Gain",DoubleValue(1.0));

  /*Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));*/


  UEs_Info * ues_info = (UEs_Info *)malloc(sizeof(UEs_Info)*nodeNum);
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);

  // Create UE nodes.
  NodeContainer ueNode;
  ueNode.Create (nodeNum);

  //configure movements for each node, while reading trace file
  ns2.Install ();

  //Create enbNode
  NodeContainer enbNode;
  enbNode.Create (5);

  //Set enb position
  Ptr <ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector(500.0,8.00,0));
  
  for (int i = 1 ; i < enbNum ; i++){
    Vector enbPosition ( 1000 * i +500 , 8, 0);
    enbPositionAlloc-> Add (enbPosition);
  }

  /*Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);*/
  /*lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");
  lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold",
                                          UintegerValue (30));
  lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset",
                                          UintegerValue (1));*/
  /*Ptr<Node> pgw = epcHelper->GetPgwNode();

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);*/


  // Install mobility
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator (enbPositionAlloc);
  enbMobility.Install (enbNode);

  //Install netdevice
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  enbDevs = lteHelper->InstallEnbDevice (enbNode);
  ueDevs = lteHelper->InstallUeDevice(ueNode);

  Ptr<LtePhy> enbPhy= enbDevs.Get(connected)->GetObject<LteEnbNetDevice>()->GetPhy()->GetObject<LtePhy>();
  //enbPhy->CreateTxPowerSpectralDensity();

  // we install the IP stack on the UEs
  InternetStackHelper internet1;
  internet1.Install (ueNode);
 /* Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper-> AssignUeIpv4Address(NetDeviceContainer (ueDevs));*/

  //Set the power density
  Ptr <SpectrumValue> psd = enbPhy->CreateTxPowerSpectralDensity();
  (*psd) = 4.14e-21 ;
  Ptr<LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get(connected)->GetObject<LteEnbNetDevice> ()->GetPhy ()->GetDownlinkSpectrumPhy ()->GetObject<LteSpectrumPhy> ();
  enbDlSpectrumPhy -> SetNoisePowerSpectralDensity(psd);


  //*******change !!!!
  for (int i = 0; i < nodeNum ; i++){
    lteHelper->Attach(ueDevs.Get(i),enbDevs.Get(connected));
  }
 
  /*Ptr<EpcTft> tft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpf;
  dlpf.localPortStart = 1234;
  dlpf.localPortEnd = 1234;
  tft->Add (dlpf);
  EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
  lteHelper->ActivateDedicatedEpsBearer (ueDevs, bearer, tft);*/
 
  //lteHelper->AddX2Interface (enbNode);
  //Ptr <LteUeRrc> ueRrc = CreateObject<LteUeRrc>();  //use ini trace handover

  for(int i = 0; i < nodeNum; i++){
      // set position
      ueMobilityModel = ueNode.Get(i)->GetObject<MobilityModel>();
      ues_info[i].set_Position(ueMobilityModel->GetPosition());
      //ues_info[i].setCounter(0);
      ////*******change
      ues_info[i].setConnectenb(connected+1);
      ues_info[i].set_output(&outputfile1,&outputfile2,&outputfile3,&outputfile4);

      // set imsi
      uephy = ueDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetPhy ();
      //ueRrc = ueDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetRrc ();
      ues_info[i].set_imsi(ueDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetImsi ());

      // Connect Sinr Report to class member fucntion.
      uephy -> TraceConnectWithoutContext("ReportCurrentCellRsrpSinr", MakeCallback (&UEs_Info::GetUeSinr, &ues_info[i]));
      ueMobilityModel -> TraceConnectWithoutContext("CourseChange", MakeCallback (&UEs_Info::CourseChange1, &ues_info[i]));
      //ueRrc->TraceConnectWithoutContext("ConnectionEstablished",MakeCallback(&UEs_Info::NotifyConnectionEstablishedUe,&ues_info[i]));
      //ueRrc->TraceConnectWithoutContext("HandoverStart",MakeCallback(&UEs_Info::NotifyHandoverStartUe,&ues_info[i]));
      //Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteEnbNetDevice/LteEnbPhy/ReportUeSinr",MakeCallback(&ReportUeSinr));

    }
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);
  
  Simulator::Stop (Seconds (duration));

  //lteHelper->EnableTraces ();
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}