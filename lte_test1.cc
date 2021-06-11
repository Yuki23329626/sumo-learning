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

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0
 
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int main (int argc, char *argv[])
{
    bool verbose = true;
    uint32_t nEnb = 1;
    uint32_t nUe = 1;

    CommandLine cmd;
    cmd.AddValue ("nEnb", "Number of \"extra\" ENB nodes/devices", nEnb);
    cmd.AddValue ("nUe", "Number of wifi UE devices", nUe);
    cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse (argc, argv);

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    // LTE MAC Schesular
    // lteHelper->SetSchedulerType ("ns3::FdMtFfMacScheduler");    // FD-MT scheduler
    // lteHelper->SetSchedulerType ("ns3::TdMtFfMacScheduler");    // TD-MT scheduler
    // lteHelper->SetSchedulerType ("ns3::TtaFfMacScheduler");     // TTA scheduler
    // lteHelper->SetSchedulerType ("ns3::FdBetFfMacScheduler");   // FD-BET scheduler
    // lteHelper->SetSchedulerType ("ns3::TdBetFfMacScheduler");   // TD-BET scheduler
    // lteHelper->SetSchedulerType ("ns3::FdTbfqFfMacScheduler");  // FD-TBFQ scheduler
    // lteHelper->SetSchedulerType ("ns3::TdTbfqFfMacScheduler");  // TD-TBFQ scheduler
    // lteHelper->SetSchedulerType ("ns3::PssFfMacScheduler");     //PSS scheduler
    // lteHelper->SetSchedulerAttribute("DebtLimit", IntegerValue(yourvalue)); // default value -625000 bytes (-5Mb)
    // lteHelper->SetSchedulerAttribute("CreditLimit", UintegerValue(yourvalue)); // default value 625000 bytes (5Mb)
    // lteHelper->SetSchedulerAttribute("TokenPoolSize", UintegerValue(yourvalue)); // default value 1 byte
    // lteHelper->SetSchedulerAttribute("CreditableThreshold", UintegerValue(yourvalue)); // default value 0

    // lteHelper->EnablePhyTraces ();
    // lteHelper->EnableMacTraces ();
    // lteHelper->EnableRlcTraces ();
    // lteHelper->EnablePdcpTraces ();

    NodeContainer enbNodes;
    enbNodes.Create (1);
    NodeContainer ueNodes;
    ueNodes.Create (1);

    LteHelper lte;

    NetDeviceContainer ueDevs, enbDevs;
    ueDevs = lte.InstallUeDevice (ueNodes);
    enbDevs = lte.InstallEnbDevice (enbNodes);

    InternetStackHelper stack;
    stack.Install (ueNodes);
    stack.Install (enbNodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer UEinterfaces = address.Assign (ueDevs);
    Ipv4InterfaceContainer ENBinterface = address.Assign (enbDevs);

    // register UE to a given eNB:
    Ptr<EnbNetDevice> enb = enbDevs.Get (0)->GetObject<EnbNetDevice> ();
    Ptr<UeNetDevice> ue = ueDevs.Get (i)->GetObject<UeNetDevice> ();
    lte.RegisterUeToTheEnb (ue, enb);

    // define a set of sub channels to use for dl and ul transmission:
    std::vector<int> dlSubChannels;
    for (int i = 0; i < 25; i++)
    {
        dlSubChannels.push_back (i);
    }
    std::vector<int> ulSubChannels;
    for (int i = 50; i < 100; i++)
    {
        ulSubChannels.push_back (i);
    }

    enb->GetPhy ()->SetDownlinkSubChannels (dlSubChannels);
    enb->GetPhy ()->SetUplinkSubChannels (ulSubChannels);
    ue->GetPhy ()->SetDownlinkSubChannels (dlSubChannels);
    ue->GetPhy ()->SetUplinkSubChannels (ulSubChannels);

    lte.AddDownlinkChannelRealization (enbMobility, ueMobility, ue->GetPhy ());

    // udp echo server
    // server
    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (enbNodes.Get (0));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    // client
    UdpEchoClientHelper echoClient (ENBinterface.GetAddress (0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps =
    echoClient.Install (ueNodes.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));

    Simulator::Stop (Seconds (10.0));

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
