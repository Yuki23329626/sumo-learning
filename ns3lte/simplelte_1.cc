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

using namespace ns3;
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

  for (uint16_t i = 2; i < 5; i++)
  {
	  pAnim->UpdateNodeSize (i, 70, 70);
	  pAnim->UpdateNodeImage (i, currentResourceId);
  }

  for (uint16_t i =5; i < 7; i++)
  { 
	  pAnim->UpdateNodeSize (i, 50, 30);
	  pAnim->UpdateNodeImage (i, currentResourceId2);
  }

  for (uint16_t i =7; i < 10; i++)
  {
	  pAnim->UpdateNodeSize (i, 70, 30);
	  pAnim->UpdateNodeImage (i, currentResourceId6);
  }

  for (uint16_t i =10; i < 15; i++)
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
	// Set default value for LTE Network
	Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (true));
	Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (true));
	Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));
	Config::SetDefault ("ns3::LteHelper::UsePdschForCqiGeneration", BooleanValue (true));
	Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (true));
	Config::SetDefault ("ns3::LteUePowerControl::ClosedLoop", BooleanValue (true));
	Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (false));


	Time::SetResolution (Time::NS);
	
       

	// EnodeB = 3, UE = 20
	uint16_t numberOfNodesENB = 3;
	uint16_t numberOfNodesEU = 4;
	double simTime = 0.05;
	double distance = 250.0;
	double interPacketInterval = 150.0;

	std::string animFile = "lte-4G.xml";

	// Allow input via command
	CommandLine cmd;
	cmd.AddValue("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodesENB);
	cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
	cmd.AddValue("distance", "Distance between eNBs [m]", distance);
	cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
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
	lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
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
	positionAlloc1->Add(Vector(500.0, -100.0, 20.0));
	MobilityHelper mobility1;
	mobility1.SetPositionAllocator (positionAlloc1);
	mobility1.Install (remoteHostContainer);
    
	// Create Internet connection: PPP connection between PGW and Remote host
	PointToPointHelper p2ph;
	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("150Mb/s")));
	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
	p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
	NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
    
    p2ph.EnablePcapAll("pgw-host");
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

	double x_min = 0.0;
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
	gridBuildingAllocator->Create (6);

	
	MobilityHelper mobility;
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.SetPositionAllocator("ns3::GridPositionAllocator",
								   "MinX", DoubleValue (-350.0),
								   "MinY", DoubleValue (-200.0),
								   "DeltaX", DoubleValue (0.0),
								   "DeltaY", DoubleValue (150.0),
								   "GridWidth", UintegerValue (1),
								   "LayoutType", StringValue ("RowFirst"));

	mobility.Install (enbNodes);
	BuildingsHelper::Install (enbNodes);

	// Set movement attributes for all EU nodes
	for (uint16_t i = 0; i < numberOfNodesEU; i++){
	   mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
								   "MinX", DoubleValue (-900.0),
								   "MinY", DoubleValue (-250.0),
								   "DeltaX", DoubleValue (120.0),
								   "DeltaY", DoubleValue (150.0),
								   "GridWidth", UintegerValue (4),
								   "LayoutType", StringValue ("RowFirst"));

	   mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
							   "Mode", StringValue ("Time"),
							   "Time", StringValue ("0.5s"),
							   "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=50.0]"),
							   "Bounds", RectangleValue (Rectangle (-12000.0, 12000.0, -12000.0, 12000.0)));
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

	//Attach UE to EnodeB
     
    // UE 1 to eNB1
	lteHelper->Attach (ueDevs.Get(0), enbDevs.Get(0));

	// UE 2 to eNB2

	lteHelper->Attach (ueDevs.Get(1), enbDevs.Get(1));

	// UE 3 to eNB3

	lteHelper->Attach (ueDevs.Get(2), enbDevs.Get(2));

	// UE 4 to eNB3
    
    lteHelper->Attach (ueDevs.Get(3), enbDevs.Get(2));

	/* this code is auto attach UE to EnodeB. Default attach to eNB1 
	uint16_t j = 0;
	for (uint16_t i = 0; i < numberOfNodesEU; i++){  
	   if (j < numberOfNodesENB){
			  lteHelper->Attach (ueDevs.Get(i), enbDevs.Get(j));
			  j++;
		}
		else{
			  j = 0;
			  lteHelper->Attach (ueDevs.Get(i), enbDevs.Get(j));
		}   
	}*/



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
	uint16_t ulPort = 2000; // UE port can change
    uint16_t otherPort = 3000;
    uint32_t maxBytes = 200;
    ApplicationContainer clientApps;
    ApplicationContainer serverApps;

	// generate traffic request to remote server
    for (uint32_t u = 0; u < ueNodes.GetN (); ++u){

	        ++ulPort;  // Uplink port is different
	        ++otherPort;
	  PacketSinkHelper dlPacketSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
	  PacketSinkHelper ulPacketSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
	  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
	  serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u))); // serve all clients with port 1234 
	  serverApps.Add (ulPacketSinkHelper.Install (remoteHost)); // install app for sever
	  serverApps.Add (packetSinkHelper.Install (ueNodes.Get(u))); // server other

	  BulkSendHelper dlClient ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
	  dlClient.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
	  //dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
	 // dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

	  BulkSendHelper ulClient ("ns3::TcpSocketFactory", InetSocketAddress (remoteHostAddr, ulPort));
	  ulClient.SetAttribute ("MaxBytes", UintegerValue (maxBytes));

	  //ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
	  //ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

	  PacketSinkHelper client ("ns3::TcpSocketFactory", InetSocketAddress (ueIpIface.GetAddress (u), otherPort));
	  //client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
	  //client.SetAttribute ("MaxPackets", UintegerValue(1000000));

	  clientApps.Add (dlClient.Install (remoteHost));
	  clientApps.Add (ulClient.Install (ueNodes.Get(u)));
	  if (u+1 < ueNodes.GetN ()){
		  clientApps.Add (client.Install (ueNodes.Get(u+1)));
	  }
	  else
		{
		  clientApps.Add (client.Install (ueNodes.Get(0)));
		}
	 }


       // LogComponentEnable ("TcpSocketBase", LOG_ALL);
        LogComponentEnable ("EpcMme", LOG_ALL);
        LogComponentEnable("EpcTft",LOG_ALL);
        LogComponentEnable ("GtpuHeader", LOG_ALL);
       //LogComponentEnable ("A2A4RsrqHandoverAlgorithm", LOG_ALL);
       // LogComponentEnable ("A3RsrpHandoverAlgorithm", LOG_ALL);
     //   LogComponentEnable ("CqaFfMacScheduler", LOG_ALL);
        LogComponentEnable ("EpcEnbApplication", LOG_ALL);
        LogComponentEnable ("EpcSgwPgwApplication", LOG_ALL);
      //  LogComponentEnable ("EpcUeNas", LOG_ALL);
        LogComponentEnable ("EpcX2Header", LOG_ALL);
      //  LogComponentEnable ("EpcX2", LOG_ALL);
      //  LogComponentEnable ("FdBetFfMacScheduler", LOG_ALL);
      //  LogComponentEnable ("FdMtFfMacScheduler", LOG_ALL);
      //  LogComponentEnable ("FdTbfqFfMacScheduler", LOG_ALL);
        LogComponentEnable ("LteAmc", LOG_ALL);
        LogComponentEnable ("LteAnr", LOG_ALL);
      //  LogComponentEnable ("Asn1Header", LOG_ALL);
      //  LogComponentEnable ("LteChunkProcessor", LOG_ALL);
     //   LogComponentEnable ("LteCommon", LOG_ALL);
        LogComponentEnable ("LteEnbMac", LOG_ALL);
        LogComponentEnable ("LteEnbNetDevice", LOG_ALL);
        LogComponentEnable ("LteEnbPhy", LOG_ALL);
        LogComponentEnable ("LteEnbRrc", LOG_ALL);
      //  LogComponentEnable ("LteFfrAlgorithm", LOG_ALL);
      //  LogComponentEnable ("LteFfrDistributedAlgorithm", LOG_ALL);
      //  LogComponentEnable ("LteFfrEnhancedAlgorithm", LOG_ALL);
      //  LogComponentEnable ("LteFfrSoftAlgorithm", LOG_ALL);
     //   LogComponentEnable ("LteFrHardAlgorithm", LOG_ALL);
        LogComponentEnable ("LteHarqPhy", LOG_ALL);
      //  LogComponentEnable ("LteInterference", LOG_ALL);
        LogComponentEnable ("LteNetDevice", LOG_ALL);
        LogComponentEnable ("LtePdcpHeader", LOG_ALL);
        LogComponentEnable ("LtePdcp", LOG_ALL);
        LogComponentEnable ("LtePhy", LOG_ALL);
        LogComponentEnable ("LteRlcAmHeader", LOG_ALL);
       // LogComponentEnable ("LteRlcAm", LOG_ALL);
        LogComponentEnable ("LteRlcHeader", LOG_ALL);
       // LogComponentEnable ("LteRlcTm", LOG_ALL);
      //  LogComponentEnable ("LteRlcUm", LOG_ALL);
        LogComponentEnable ("LteRlc", LOG_ALL);
        LogComponentEnable ("LteRlcHeader", LOG_ALL);
        LogComponentEnable ("RrcHeader", LOG_ALL);
      //  LogComponentEnable ("LteRrcProtocolIdeal", LOG_ALL);
      //  LogComponentEnable ("LteRrcProtocolReal", LOG_ALL);
      //  LogComponentEnable ("LteSpectrumPhy", LOG_ALL);
      //  LogComponentEnable ("LteSpectrumSignalParameters", LOG_ALL);
        LogComponentEnable ("LteUeMac", LOG_ALL);
        LogComponentEnable ("LteUeNetDevice", LOG_ALL);
        LogComponentEnable ("LteUePhy", LOG_ALL);
        LogComponentEnable ("LteUeRrc", LOG_ALL);
      //  LogComponentEnable ("PfFfMacScheduler", LOG_ALL);
      //  LogComponentEnable ("PssFfMacScheduler", LOG_ALL);
     //   LogComponentEnable ("RemSpectrumPhy", LOG_ALL);
     //   LogComponentEnable ("RrFfMacScheduler", LOG_ALL);
     //   LogComponentEnable ("TdBetFfMacScheduler", LOG_ALL);
     //   LogComponentEnable ("TdMtFfMacScheduler", LOG_ALL);
     //   LogComponentEnable ("TdTbfqFfMacScheduler", LOG_ALL);
      //  LogComponentEnable ("TtaFfMacScheduler", LOG_ALL);

	// Install and start applications on UEs and remote host

    //serverApps.EnablePcapAll ("datatransfer");
	serverApps.Start (Seconds (0.01));
	clientApps.Start (Seconds (0.01));

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
	resourceId2 = pAnim->AddResource ("/home/nvlinh/ns-allinone-3.26/ns-3.26/scratch/policecar.png");
	resourceId1 = pAnim->AddResource ("/home/nvlinh/ns-allinone-3.26/ns-3.26/scratch/enodeb.png");
	resourceId3 = pAnim->AddResource ("/home/nvlinh/ns-allinone-3.26/ns-3.26/scratch/server.png");
	resourceId4 = pAnim->AddResource ("/home/nvlinh/ns-allinone-3.26/ns-3.26/scratch/remotehost.png");
	resourceId5 = pAnim->AddResource ("/home/nvlinh/ns-allinone-3.26/ns-3.26/scratch/phone.png");
	resourceId6 = pAnim->AddResource ("/home/nvlinh/ns-allinone-3.26/ns-3.26/scratch/car.png");
	pAnim->SetBackgroundImage ("/home/nvlinh/ns-allinone-3.26/ns-3.26/scratch/4g-lte.jpg", -1, 1, 0.5, 0.5, 1);
	  
	Simulator::Schedule (Seconds (simTime), modify);
	Simulator::Run ();
	Simulator::Destroy ();
	delete pAnim;

	return 0;

}
