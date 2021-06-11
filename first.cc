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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{

  NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NS_LOG_INFO ("Creating Topology");

  NodeContainer nodes;
  nodes.Create (2);

  // 拓樸輔助工具 Helper
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // NetDeviceContainer 用來存放 NetDevice 物件
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  // TCP, UDP, IP 等
  InternetStackHelper stack;
  stack.Install (nodes);

  // 為裝置分配 IP
  Ipv4AddressHelper address;
  // 為 10.1.1.0 的網段，subnet mask 為 255.255.255.0
  // 預設從 10.1.1.1 開始，每次+1，下一個為 10.1.1.2
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // 一樣為設定相關參數的 helper 類物件
  // 9 為 server port number
  UdpEchoServerHelper echoServer (9);

  // 將 container index = 1 的 container 的節點上安裝一個 UDP echo server application
  // Install 會 return 一個 container, 包含所有被 helper 物件建立的 application pointer
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  // 開始 跟 結束資料通訊 的時間
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  // echo client 端相關設定
  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  // 模擬期間能傳送的 最大資料包個數
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  // 兩個 packet 間要等待多長時間
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  // packet 應該包含多少資料，應該沒有包含 header
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  // contianer index = 0
  // 開始 與 結束時間 為 2, 10
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
