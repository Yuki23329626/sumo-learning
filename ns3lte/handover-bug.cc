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
#include "ns3/flow-monitor-module.h"
#include "ns3/log.h"
#include <sys/timeb.h>
#include <iostream>
#include <ns3/internet-trace-helper.h>

using namespace ns3;
using namespace std;

double simulationTime = 100; // seconds

// Return system time in milliseconds
int getMilliCount(){
	timeb tb;
	ftime(&tb);
	int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
	return nCount;
}

// Compute difference between current system time and the input value
int getMilliSpan(int nTimeStart){
	int nSpan = getMilliCount() - nTimeStart;
	if(nSpan < 0)
		nSpan += 0x100000 * 1000;
	return nSpan;
}

// Print the simulation time with a progress status
void DisplayCurrentTime(void) {
	int nsec = Simulator::Now().GetMilliSeconds();
	printf("%c[2K\r", 27); //clear line
	printf("t = %.02f s (%g %%)", nsec/1000.0, floor((double)nsec/(10*(double)simulationTime)));
	cout.flush();
	Simulator::Schedule(MilliSeconds(2.5*simulationTime), &DisplayCurrentTime);
}

void NotifyHandoverStartUe (std::string context, 
                       uint64_t imsi, 
                       uint16_t cellid, 
                       uint16_t rnti, 
                       uint16_t targetCellId)
{
  std::cout << " *** Handover from " << cellid << " to " << targetCellId << std::endl;
}

int main (int argc, char *argv[]) {
	CommandLine cmd;
	
	int numberENodeB = 6;
	int numberUser = 1;
	
	double cellDiameter = 1000;
	double eNodeBAltitude = 5;
	double UESpeed = 50; // in km/h
	
	cmd.AddValue("duration", "Duration of the simulation in seconds", simulationTime);
	cmd.AddValue("neNodeB", "Number of eNodeB", numberENodeB);
	cmd.AddValue("cellDiameter", "Diameter of a cell. Defines the space between two consecutive eNodeB", cellDiameter);
	
	cmd.Parse (argc, argv);
	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults ();
	cmd.Parse (argc, argv); //parse again to override default values
	
	double txStartTime = 0;
	double txEndTime = simulationTime;
	
	DataRate dlDataRate = DataRate("1Mbps");
	int dlDataSize = 1000;
	
	Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
	Ptr<EpcHelper> epcHelper = CreateObject<EpcHelper> ();
	
	// Path Losses
	lteHelper->SetPathlossModelType("ns3::FriisSpectrumPropagationLossModel");
	
	lteHelper->SetEpcHelper (epcHelper);
    NodeContainer ueNodes;
	NodeContainer enbNodes;
	
	enbNodes.Create(numberENodeB);
	ueNodes.Create(numberUser);

	// Mobility model for the eNodeB
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> enbPosAllocator = CreateObject <ListPositionAllocator>();
	for(int i=0 ; i < numberENodeB ; i++) {
		enbPosAllocator->Add(
			Vector(i*cellDiameter, 0, eNodeBAltitude)
		);
	}
	mobility.SetPositionAllocator(enbPosAllocator);
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(enbNodes);
							 
	// Mobility model for the UE
	Ptr<ListPositionAllocator> uePosAllocator = CreateObject <ListPositionAllocator>();
	for(int i=0 ; i < numberUser ; i++) {
		uePosAllocator->Add(Vector(0,200,0));
	}
	mobility.SetPositionAllocator(uePosAllocator);
	mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
	mobility.Install(ueNodes);
	Vector vSpeed = Vector( UESpeed/3.6, 0, 0 ); // moves along the x axis
	for(uint32_t u=0 ; u < ueNodes.GetN() ; u++) {
		Ptr<ConstantVelocityMobilityModel> p = ueNodes.Get(u)->GetObject<ConstantVelocityMobilityModel> ();
		p->SetVelocity (vSpeed);
	}
	
	Ptr<Node> pgw = epcHelper->GetPgwNode ();
	
	// Create RemoteHosts
	NodeContainer remoteHostsContainer;
	remoteHostsContainer.Create (numberUser); //each UE is connected to a different remoteHost
	InternetStackHelper internet;
	internet.Install (remoteHostsContainer);
	
	// Link RemoteHosts to the PGW
	for (int u=0; u < numberUser ; ++u)  {
		PointToPointHelper p2ph;
		p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
		p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
		p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(10)));

		NetDeviceContainer internetDevices = p2ph.Install (remoteHostsContainer.Get(u), pgw);
		
		Ipv4AddressHelper ipv4h;
		char buf[512];
		sprintf(buf,"50.50.%d.0", u+1);
		ipv4h.SetBase(buf, "255.255.255.0");
		ipv4h.Assign (internetDevices);
	}

	// Add route from the remotes to the UEs
	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	for (uint32_t u=0; u < remoteHostsContainer.GetN() ; ++u)  {
		Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHostsContainer.Get(u)->GetObject<Ipv4> ());
		remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
	}
	
	// Install LTE Devices to the nodes
	NetDeviceContainer enbLteDevs;
	for(uint32_t u=0 ; u < enbNodes.GetN() ; ++u) {
		lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
		
		/*lteHelper->SetEnbDeviceAttribute("DlEarfcn",     UintegerValue ( (u%2)*50 ));
		lteHelper->SetEnbDeviceAttribute("UlEarfcn",     UintegerValue ( (u%2)*50 + 18000 ));
		lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (25));
		lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (25));*/
		
		enbLteDevs.Add( lteHelper->InstallEnbDevice (enbNodes.Get(u)) );
	}
	lteHelper->SetUeAntennaModelType ("ns3::IsotropicAntennaModel");
	
	NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
	
	// Install X2 interface
	lteHelper->AddX2Interface (enbNodes);

	// Install the IP stack on the UEs
	internet.Install (ueNodes);
	Ipv4InterfaceContainer ueIpIface;
	ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
	for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
	{
	  Ptr<Node> ueNode = ueNodes.Get (u);
	  // Set the default gateway for the UE
	  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
	  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
	  lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get(u), EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());
	}
	// Attach all UEs to the closest eNodeB
	lteHelper->AttachToClosestEnb (ueLteDevs, enbLteDevs);

	// Install and start applications on UEs and remote host
	uint16_t dlPort = 6666;
	ApplicationContainer appContainer;
	for (uint32_t u=0; u < ueNodes.GetN() ; ++u) {
		// App
		OnOffHelper dlApp("ns3::UdpSocketFactory", InetSocketAddress(ueIpIface.GetAddress(u), dlPort+u));
		dlApp.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")); 
		dlApp.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
		dlApp.SetAttribute("DataRate", DataRateValue(dlDataRate));
		dlApp.SetAttribute("PacketSize", UintegerValue(dlDataSize));
		appContainer.Add(dlApp.Install(remoteHostsContainer.Get(u)));
	
		// Sink
		ApplicationContainer tmpAppContainer;
		PacketSinkHelper dlSink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort+u));
		tmpAppContainer = dlSink.Install(ueNodes.Get(u));
		tmpAppContainer.Start(Seconds(0));
		tmpAppContainer.Stop(Seconds(simulationTime));
		
	}
    appContainer.Start(Seconds(txStartTime));
    appContainer.Stop(Seconds(txEndTime));
    
	// Set outputs
	Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart", MakeCallback (&NotifyHandoverStartUe));

	// Set stop time
	Simulator::Stop( Seconds(simulationTime) );

	// Start simulation
	int startTime = getMilliCount();
	DisplayCurrentTime();
	Simulator::Run ();
	DisplayCurrentTime();
	int duration = getMilliSpan(startTime);
	cout << endl << "Simulation finished in " << duration/1000.0 << " secs" << endl;
		
	Simulator::Destroy ();
	return 0;
}
