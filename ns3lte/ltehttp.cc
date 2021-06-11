/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
 // - Based on Mohamned.A.FERRAG(2015) code
 // - Modified unneccessary code
 
// Default Network Topology

//                 +UE              +---+
//                 ++  +---------> |EnodeB--------------------------+
//                                   +---+                            |
//                                                                    |
// +UE       +-UE+                                                +----+---+                         +-------+
// +-+       +---------------------^  +----+                      |        |                         |       |
//              +----------------->   +EnodeB--------------------+        |                        ++   Remote server
// UE+       +UE++                                                |   PGW  +-------------------------+       |
// +-+       +--+                                                 |        |                         +-------+
//                                      +------------------------+        |
// +UE+      +UE++                   +---+                        +--------+
// +--+      +-------------------->  |EnodeB
//                     +--------^     +--+
//                     |
//                     |
//                 +-UE +
//                +--+


//Contents
//========
// - 15 nodes: 4 UE, 3 EnodeB, 1 PGW, 1 remote host
// - Create sample LTE Network
// - Developer: Van Linh Nguyen
// - Release date: 2016/11/03
// - Documentation
// - Release notes

#include <iostream>
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include <ns3/buildings-module.h>
#include "ns3/netanim-module.h"

#include "ns3/flow-monitor-module.h"
#include "ns3/log.h"
#include <sys/timeb.h>
#include <ns3/internet-trace-helper.h>
#include <ns3/spectrum-module.h>
#include <ns3/log.h>
#include <ns3/string.h>
#include <fstream>
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/gnuplot.h"
#include <math.h>
#include <vector>

#include "ns3/lteddos.h"
#include "ns3/ltenormalapplication-helper.h"
#include "ns3/lteattackhighworkload-helper.h"


using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("LenaFrequencyReuse");

AnimationInterface * pAnim = 0;

// Create different type of nodes : EnodeB, UE, Remote host...
uint32_t resourceId1;
uint32_t resourceId2;
uint32_t resourceId3;
uint32_t resourceId4;
uint32_t resourceId5;
uint32_t resourceId6;


// Update value for scenario via Popup 
// We can set default value so no need this step if need.

void modify ()
{
  std::ostringstream oss;
  oss << "Update:" << Simulator::Now ().GetSeconds ();
  pAnim->UpdateLinkDescription (0, 1, oss.str ());
  pAnim->UpdateLinkDescription (0, 2, oss.str ());
  pAnim->UpdateLinkDescription (0, 3, oss.str ());
  pAnim->UpdateLinkDescription (0, 4, oss.str ());
  pAnim->UpdateLinkDescription (0, 5, oss.str ());
  pAnim->UpdateLinkDescription (0, 6, oss.str ());
  pAnim->UpdateLinkDescription (1, 7, oss.str ());
  pAnim->UpdateLinkDescription (1, 8, oss.str ());
  pAnim->UpdateLinkDescription (1, 9, oss.str ());
  pAnim->UpdateLinkDescription (1, 10, oss.str ());
  pAnim->UpdateLinkDescription (1, 11, oss.str ());
  
  // Every update change the node description for node 2
  std::ostringstream node0Oss;
  node0Oss << "-----Node:" << Simulator::Now ().GetSeconds ();
  pAnim->UpdateNodeDescription (2, node0Oss.str ());
  static uint32_t currentResourceId = resourceId1;
  static uint32_t currentResourceId2 = resourceId2;
  static uint32_t currentResourceId3 = resourceId3;
  static uint32_t currentResourceId4 = resourceId4;
  static uint32_t currentResourceId5 = resourceId5;
  static uint32_t currentResourceId6 = resourceId6;

  pAnim->UpdateNodeSize (1, 150, 70);
  pAnim->UpdateNodeImage (1, currentResourceId4);
  pAnim->UpdateNodeSize (0, 150, 250);
  pAnim->UpdateNodeImage (0, currentResourceId3);

  for (uint16_t i = 2; i < 50; i++)
  {
    pAnim->UpdateNodeSize (i, 70, 70);
    pAnim->UpdateNodeImage (i, currentResourceId);
  }

  for (uint16_t i =50; i < 100; i++)
  { 
    pAnim->UpdateNodeSize (i, 50, 30);
    pAnim->UpdateNodeImage (i, currentResourceId2);
  }

  for (uint16_t i =100; i < 150; i++)
  {
    pAnim->UpdateNodeSize (i, 70, 30);
    pAnim->UpdateNodeImage (i, currentResourceId6);
  }

  for (uint16_t i =150; i < 200; i++)
  {
    pAnim->UpdateNodeSize (i, 50, 50);
    pAnim->UpdateNodeImage (i, currentResourceId5);
  }

 if (Simulator::Now ().GetSeconds () < 10) 
  // Modify schedule again
     Simulator::Schedule (Seconds (0.05), modify);

}

int main (int argc, char *argv[])
{
  

  LteDDoS lteddos;

  // Set default value for LTE Network
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));
  Config::SetDefault ("ns3::LteHelper::UsePdschForCqiGeneration", BooleanValue (true));
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (true));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
  Config::SetDefault ("ns3::LteUePowerControl::ClosedLoop", BooleanValue (true));
  Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (false));


  Time::SetResolution (Time::NS);
  
       

  // EnodeB = 3, UE = 20
  uint16_t numberOfNodesENB = 10;
  uint16_t numberOfNodesEU = 160;
  double simTime = 5;
  double enb_distance_y = 80.0;
  double enb_distance_x = 80.0;
  uint16_t ueLine = 10;
  double ueDistanceX = 40;
  double ueDistanceY = 50;
  uint16_t speed = 50; // m
  uint16_t enbDistance = 1000;

  double observationTime = 250.0;

  std::string animFile = "lte_http.xml";

  // Allow input via command
  CommandLine cmd;
  cmd.AddValue("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodesENB);
  cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue("ueLine", "UE Line", ueLine);
  cmd.AddValue("speed", "Mobility Moving", speed);
  cmd.AddValue("ueDistanceX", "UE Distance X", ueDistanceX);
  cmd.AddValue("ueDistanceY", "UE Distance Y", ueDistanceY);
  cmd.AddValue("EnbDistanceX", "Distance X between eNBs [m]", enb_distance_x);
  cmd.AddValue("EnbDistanceY", "Distance Y between eNBs [m]", enb_distance_y);
  cmd.AddValue("observationTime", "Inter packet interval [ms])", observationTime);
  cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);
  cmd.Parse (argc, argv);
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  cmd.Parse (argc, argv);


  //create LTE Object by lteHelper.
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));
    
  // Load sample distribution [ Check LTE Module at Ns3 to see at the end of page]
  std::ifstream ifTraceFile;
  ifTraceFile.open ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad", std::ifstream::in);
  if (ifTraceFile.good ()){
    lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
  }else{
    lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
  } 
    
  // these parameters have to setted only in case of the trace format 
  // differs from the standard one, that is
  // - 10 seconds length trace
  // - 10,000 samples
  // - 0.5 seconds for window size
  // - 100 RB
  lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
  lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
  lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (1)));
  lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));

  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (100));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (18100));

  // create EPC Object epcHelper.

  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
    
  // Create PGW object
  Ptr<Node> pgw = epcHelper->GetPgwNode (); 
     
    
    /* Create remote host and PGW channel connection */

   // creation  RemoteHost .
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);
    
  // Set square range where all nodes which move 
  Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator>();
  positionAlloc1->Add(Vector(500.0, 200.0, 150.0));
  MobilityHelper mobility1;
  mobility1.SetPositionAllocator (positionAlloc1);
  mobility1.Install (remoteHostContainer);
    
  // Create Internet connection: PPP connection between PGW and Remote host
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
    

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

     // Add routing table of host to PGW network

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
    
    /* End to create PPP between remotehost and PGW */

  // Create nodes for EnodeB and UE
  NodeContainer enbNodes;
  enbNodes.Create (numberOfNodesENB);
  NodeContainer ueNodes;
  ueNodes.Create (numberOfNodesEU);

        

  //set the position and movement of the nodes

  /*double x_min = 0.0;
  double x_max = 10.0;
  double y_min = 0.0;
  double y_max = 20.0;
  double z_min = 0.0;
  double z_max = 10.0;
  Ptr<Building> b = CreateObject <Building> ();
  b->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
  b->SetBuildingType (Building::Residential);
  b->SetExtWallsType (Building::ConcreteWithWindows); // standard type for wall
  b->SetNFloors (3);
  b->SetNRoomsX (3);
  b->SetNRoomsY (2);
  
    // Create building to allocate nodes
  Ptr<GridBuildingAllocator> gridBuildingAllocator;
  gridBuildingAllocator = CreateObject<GridBuildingAllocator> ();
  gridBuildingAllocator->SetAttribute ("GridWidth", UintegerValue (3));
  gridBuildingAllocator->SetAttribute ("LengthX", DoubleValue (7));
  gridBuildingAllocator->SetAttribute ("LengthY", DoubleValue (13));
  gridBuildingAllocator->SetAttribute ("DeltaX", DoubleValue (3));
  gridBuildingAllocator->SetAttribute ("DeltaY", DoubleValue (3));
  gridBuildingAllocator->SetAttribute ("Height", DoubleValue (6));
  gridBuildingAllocator->SetBuildingAttribute ("NRoomsX", UintegerValue (2));
  gridBuildingAllocator->SetBuildingAttribute ("NRoomsY", UintegerValue (4));
  gridBuildingAllocator->SetBuildingAttribute ("NFloors", UintegerValue (2));
  gridBuildingAllocator->SetAttribute ("MinX", DoubleValue (0));
  gridBuildingAllocator->SetAttribute ("MinY", DoubleValue (0));
  gridBuildingAllocator->Create (6);*/

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodesENB; i++)
    {
      positionAlloc->Add (Vector(enbDistance * i, 0, 0));
    }

  
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                   "MinX", DoubleValue (enb_distance_x - 180),
                   "MinY", DoubleValue (enb_distance_y - 180),
                   "DeltaX", DoubleValue (enb_distance_x),
                   "DeltaY", DoubleValue (enb_distance_y),
                   "GridWidth", UintegerValue (1),
                   "LayoutType", StringValue ("RowFirst"));

  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  // Set movement attributes for all EU nodes
  for (uint16_t i = 0; i < numberOfNodesEU; i++){
     mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                   "MinX", DoubleValue (ueDistanceX - 300),
                   "MinY", DoubleValue (ueDistanceY - 200),
                   "DeltaX", DoubleValue (ueDistanceX),
                   "DeltaY", DoubleValue (ueDistanceY),
                   "GridWidth", UintegerValue (ueLine),
                   "LayoutType", StringValue ("RowFirst"));

     mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                 "Mode", StringValue ("Time"),
                 "Time", StringValue ("0.5s"),
                 "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=50.0]"),
                 "Bounds", RectangleValue (Rectangle (-2000.0, 3000.0, -5000.0, 3000.0)));
  }
  mobility.Install (ueNodes);
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("mobility-trace.tr"));

  // Install Network card for EnodeB and UE
  NetDeviceContainer enbDevs;
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueDevs;
  ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;

  // Assign IP from network card of all UE
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));


    // set Routing for UE is PGW IP address

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u){
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

    
    // Atach UE to enB

  //lteddos.createListUE();

  std::ofstream UEListIMSI;
  std::string filename_imsi = "LteUelistIMSI.txt"; 
  UEListIMSI.open(filename_imsi.c_str(), std::ofstream::out);
  UEListIMSI << "IP , IMSI" << std::endl;

  uint16_t j = 0;
  for (uint16_t i = 0; i < numberOfNodesEU; i++){  
     if (j < numberOfNodesENB){
        lteHelper->Attach (ueDevs.Get(i), enbDevs.Get(j));
        j= i%16;
    }
    else{
        j = 0;
        lteHelper->Attach (ueDevs.Get(i), enbDevs.Get(j));
    }   

    // std::cout << ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi() << std::endl;
     //enbDevs.Get(j)->GetObject<LteEnbNetDevice> ();

     //lteddos.storeListUE(ueIpIface.GetAddress(i),ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi());

     UEListIMSI << ueIpIface.GetAddress(i) << "," << ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
     UEListIMSI << std::endl;
 
   

  }

   UEListIMSI.close();


  // Establish EPC Traffic Flow template between UE and EnodeB
  // Up Link Traffic Flow Template (UL TFT) : - Set of uplink packet filters in TFT
    // Downlink Traffic Flow Template (DL TFT) : - Set of downlink packet filters in TFT
    // Choose only traffic of this bearer pass

  Ptr<EpcTft> tft = Create<EpcTft> ();
  EpcTft::PacketFilter pf;
  pf.localPortStart = 8080;
  pf.localPortEnd = 8080;
  tft->Add (pf);
  lteHelper->ActivateDedicatedEpsBearer (ueDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), tft);


     
    // Set Uplink and downlink port
  uint16_t dlPort = 8080; // server port is fixed
  //uint16_t ulPort = 2000; // UE port can change
    //uint16_t otherPort = 3000;
   // uint32_t maxBytes = 500;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  PacketSinkHelper dlPacketSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
  serverApps = dlPacketSinkHelper.Install (remoteHost); // install app for sever
  serverApps.Start (Seconds (0.01));


     vector <uint32_t> highUEs;
     vector <uint32_t> lowUEs;
     vector <uint32_t> normalUEs;
     vector <ns3::Ipv4Address> normalIPs;
     vector <ns3::Ipv4Address> highIPs;
     vector <ns3::Ipv4Address> lowIPs;

     for(uint32_t u = 0; u < ueNodes.GetN(); u++)
     {

            if(u%5==0)
            {
              highUEs.push_back(u); // Node # 5 , 10 , 15 , (3 high attacker per enB) Check IP list for exact ordering
            } else if(u%4==0)
            {
              lowUEs.push_back(u); // Node # 4, 8, 12, 16 (4 low attacker per enB) Check IP list for exact ordering
            }
            else
            {
              normalUEs.push_back(u); // Node # 0,1,2,3,6,7,9,11,13
            }


     }

     // start install appliation for normal UE

     
    std::ofstream normalIPout;
    std::string filename_normal = "LteNormalUeIPList.txt"; 
    normalIPout.open(filename_normal.c_str(), std::ofstream::out);
    normalIPout << "IP" << std::endl;

      // Normal traffic
    
    OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
    clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    AddressValue remoteAddress (InetSocketAddress (remoteHostAddr, dlPort));
    clientHelper.SetAttribute ("Remote", remoteAddress);
    clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate("50kb/s")));
    clientHelper.SetAttribute ("PacketSize", UintegerValue (512));
  
    while (!normalUEs.empty())
   {
      uint32_t temp; 
      temp  = normalUEs.back();

      //std::cout << temp << " " <<  ueIpIface.GetAddress (temp) << ":" << dlPort << endl;

       normalIPs.push_back(ueIpIface.GetAddress (temp));

       normalIPout << "IP:" << ueIpIface.GetAddress(temp) << " --IMSI:" << ueDevs.Get(temp)->GetObject<LteUeNetDevice>()->GetImsi() ;
       normalIPout << std::endl; 

      

       clientApps.Add (clientHelper.Install (ueNodes.Get(temp)));
       clientApps.Start (Seconds (0.01));

       normalUEs.pop_back();
   }

    normalIPout.close();

    
    // High rate attack
    
    std::ofstream HighIPout;
    std::string filename_high = "LteHighUeIPList.txt"; 
    HighIPout.open(filename_high.c_str(), std::ofstream::out);
    HighIPout << "IP" << std::endl;

    uint32_t maxHighBytes = 0;
    BulkSendHelper highattack ("ns3::TcpSocketFactory", InetSocketAddress (remoteHostAddr, dlPort));
    highattack.SetAttribute ("MaxBytes", UintegerValue (maxHighBytes));

    ApplicationContainer sourceApps ;

    while (!highUEs.empty())
   {
      uint32_t temp; 
      temp  = highUEs.back();

      highIPs.push_back(ueIpIface.GetAddress (temp));

      HighIPout << "IP:" << ueIpIface.GetAddress(temp) << " --IMSI:" << ueDevs.Get(temp)->GetObject<LteUeNetDevice>()->GetImsi() ;
      HighIPout << std::endl;
  
      sourceApps = highattack.Install (ueNodes.Get(temp));
      sourceApps.Start (Seconds (0.25));
     // sourceApps.Stop (Seconds (120));

     // std::cout << temp << " " <<  ueIpIface.GetAddress (temp) << ":" << dlPort << endl;

      highUEs.pop_back();
   }

   HighIPout.close();


   // Low rate attack
   
    std::ofstream LowIPout;
    std::string filename_low= "LteLowUeIPList.txt"; 
    LowIPout.open(filename_low.c_str(), std::ofstream::out);
    LowIPout << "IP" << std::endl;

    uint32_t maxLowBytes = 10000;
    BulkSendHelper lowattack ("ns3::TcpSocketFactory", InetSocketAddress (remoteHostAddr, dlPort));
         // Set the amount of data to send in bytes.  Zero is unlimited.
    lowattack.SetAttribute ("MaxBytes", UintegerValue (maxLowBytes));

    while (!lowUEs.empty())
   {
      uint32_t temp; 
      temp  = lowUEs.back();

      lowIPs.push_back(ueIpIface.GetAddress (temp));

      LowIPout << "IP:" << ueIpIface.GetAddress(temp) << " --IMSI:" << ueDevs.Get(temp)->GetObject<LteUeNetDevice>()->GetImsi() ;
      LowIPout << std::endl;
  
   /*  LteNormalApplicationHelper lowattack("ns3::TcpSocketFactory", InetSocketAddress (ueIpIface.GetAddress (temp), dlPort));
     lowattack.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=2]"));
     lowattack.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
     lowattack.SetAttribute ("DataRate", DataRateValue (DataRate("56kbps")));
     lowattack.SetAttribute ("PacketSize", UintegerValue (3000));

       AddressValue remoteAddress (InetSocketAddress (remoteHostAddr, dlPort));
       lowattack.SetAttribute ("Remote", remoteAddress);
       clientApps.Add (lowattack.Install (ueNodes.Get(temp)));*/

      clientApps = lowattack.Install (ueNodes.Get(temp));

      clientApps.Start (Seconds (0.75));

      // clientApps.Start (Seconds (simTime));

       ///std::cout << temp << " " <<  ueIpIface.GetAddress (temp) << ":" << dlPort << endl;

       lowUEs.pop_back();
   }

   LowIPout.close();


   



   /*  while (!normalIPs.empty())
   {
           ns3::Ipv4Address ip = normalIPs.back();

          std::cout << ip << endl;

          normalIPs.pop_back();
   } */


     //p2ph.EnableAsciiAll (ascii.CreateFileStream ("serverpgw_trace_http.tr"));
     p2ph.EnablePcapAll("pgw-host-http");

    
  /* std::string fileNameWithNoExtension = "FlowVSThroughput";
   std::string graphicsFileName        = fileNameWithNoExtension + ".png";
   std::string plotFileName            = fileNameWithNoExtension + ".plt";
   std::string plotTitle               = "Flow vs Throughput";
   std::string dataTitle               = "Throughput";

    // Instantiate the plot and set its title.
    Gnuplot gnuplot (graphicsFileName);
    gnuplot.SetTitle (plotTitle);

    // Make the graphics file, which the plot file will be when it
    // is used with Gnuplot, be a PNG file.
    gnuplot.SetTerminal ("png");

    // Set the labels for each axis.
    gnuplot.SetLegend ("Flow", "Throughput");


    Gnuplot2dDataset dataset;
    dataset.SetTitle (dataTitle);
    dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    gnuplot.AddDataset (dataset);*/



   // monitor->SerializeToXmlFile ("flowResults.xml" , true, true );


  Simulator::Stop (Seconds (simTime));
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::MiErrorModel));
  Config::SetDefault ("ns3::LteAmc::Ber", DoubleValue (0.00005));

        

  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();

  // Create the animation object and configure for specific output
  pAnim = new AnimationInterface (animFile.c_str ());
  //Provide the absolute path to the resource
  resourceId2 = pAnim->AddResource ("/home/john/ns-allinone-3.26/ns-3.26/scratch/policecar.png");
  resourceId1 = pAnim->AddResource ("/home/john/ns-allinone-3.26/ns-3.26/scratch/enodeb.png");
  resourceId3 = pAnim->AddResource ("/home/john/ns-allinone-3.26/ns-3.26/scratch/server.png");
  resourceId4 = pAnim->AddResource ("/home/john/ns-allinone-3.26/ns-3.26/scratch/remotehost.png");
  resourceId5 = pAnim->AddResource ("/home/john/ns-allinone-3.26/ns-3.26/scratch/phone.png");
  resourceId6 = pAnim->AddResource ("/home/john/ns-allinone-3.26/ns-3.26/scratch/car.png");
  pAnim->SetBackgroundImage ("/home/john/ns-allinone-3.26/ns-3.26/scratch/4g-lte.jpg", -1, 1, 0.5, 0.5, 1);
    
  Simulator::Schedule (Seconds (simTime), modify);
  //Simulator::Stop (Seconds (20));
  Simulator::Run ();

  for(int i = 1; i <= numberOfNodesENB; i++)
  {
     lteddos.classifyLocalnode(i);
  }


  
      // Open the plot file.
     // std::ofstream plotFile (plotFileName.c_str());

      // Write the plot file.
    //  gnuplot.GenerateOutput (plotFile);

      // Close the plot file.
     //  plotFile.close ();

  Simulator::Destroy ();
  delete pAnim;

  return 0;

}
