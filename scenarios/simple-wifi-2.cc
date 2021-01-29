/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

#include "ns3/ndnSIM-module.h"
#include "ns3/animation-interface.h"

#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

using namespace std;
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ndn.WifiExample");

//
// DISCLAIMER:  Note that this is an extremely simple example, containing just 2 wifi nodes
// communicating directly over AdHoc channel.
//

// Ptr<ndn::NetDeviceFace>
// MyNetDeviceFaceCallback (Ptr<Node> node, Ptr<ndn::L3Protocol> ndn, Ptr<NetDevice> device)
// {
//   // NS_LOG_DEBUG ("Create custom network device " << node->GetId ());
//   Ptr<ndn::NetDeviceFace> face = CreateObject<ndn::MyNetDeviceFace> (node, device);
//   ndn->AddFace (face);
//   return face;
// }

int
main(int argc, char* argv[])
{
  // disable fragmentation
  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue("OfdmRate6MbpsBW10MHz"));

  CommandLine cmd;
  cmd.Parse(argc, argv);

  //////////////////////
  //////////////////////
  //////////////////////
  // WiFi channel
  YansWifiChannelHelper wifiChannelHelper = YansWifiChannelHelper::Default();
  wifiChannelHelper.AddPropagationLoss("ns3::RangePropagationLossModel");
  Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (11));
  Ptr<YansWifiChannel> channel = wifiChannelHelper.Create();

  // Wifi Physical layer and set channel
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  wifiPhy.SetChannel(channel);
  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11);

  // Wifi MAC layer
  NqosWaveMacHelper wifi80211pMac =  NqosWaveMacHelper::Default();

  // Wifi
  std::string phyMode("OfdmRate6MbpsBW10MHz");
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default();
  wifi80211p.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue(phyMode),
                               "ControlMode", StringValue(phyMode));

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector(0.0, 5.0, 0.0));
  positionAlloc->Add (Vector(10.0, 5.0, 0.0));
  positionAlloc->Add (Vector(20.0, 5.0, 0.0));

  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  NodeContainer nodes;
  nodes.Create(3);

  ////////////////
  // 1. Install Wifi
  NetDeviceContainer wifiNetDevices = wifi80211p.Install(wifiPhy, wifi80211pMac, nodes);

  // Enable pcap
  //wifiPhy.EnablePcap("dump", wifiNetDevices);

  // 2. Install Mobility model
  mobility.Install(nodes);

  // 3. Install NDN stack
  NS_LOG_INFO("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  // ndnHelper.AddNetDeviceFaceCreateCallback (WifiNetDevice::GetTypeId (), MakeCallback
  // (MyNetDeviceFaceCallback));
  ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.setCsSize(1000);
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.Install(nodes);

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::Install(nodes, "/test/prefix", "/localhost/nfd/strategy/best-route");

  // 4. Set up applications
  NS_LOG_INFO("Installing Applications");

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbrFifa");
  consumerHelper.SetPrefix("/test/prefix");
  //consumerHelper.SetAttribute("MaxSeq", UintegerValue(200));
  //consumerHelper.SetAttribute("StopTime", TimeValue (Seconds(100)));
  consumerHelper.Install(nodes.Get(0));
  //consumerHelper.Install(nodes.Get(2));

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/test/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1200"));
  producerHelper.Install(nodes.Get(2));

  

  ////////////////
  std::string output_path("/home/parth/Desktop/simulation_data/");
  std::string animFile = output_path+"v2v-test-1.xml";
  AnimationInterface anim(animFile);

  //Global routing helper
  ndn::GlobalRoutingHelper ndnGRH;
  ndnGRH.InstallAll();
  ndnGRH.AddOrigin("/test/prefix", nodes.Get(2));  
  ndn::GlobalRoutingHelper::CalculateRoutes();

  //Simulator::Stop(Seconds(100.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}