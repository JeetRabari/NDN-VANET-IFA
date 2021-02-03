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

#include "ns3/aodv-module.h"

#include "ndn-consumer-fifa.hpp"
#include "my-ndn-strategy-choice-helper.hpp"
#include "GlobalVariable.hpp"
#include "fifa-forwarding-strategy.hpp"

#include "ns3/ndnSIM/ndn-cxx/encoding/nfd-constants.hpp"
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
std::string filename = "/home/parth/Desktop/simulation_data/SimpleVanet/simple_vanet_pit_atck_fifa.csv";

void PrintCurrentTime ()
{
  std::cout << "Simulation at: " << Simulator::Now().As(Time::S) << std::endl;
  Simulator::Schedule(Seconds(5.0), &PrintCurrentTime);
}

void SetUpPitTrace()
{
  std::ofstream out (filename.c_str (), std::ios::app);

  NodeContainer c = NodeContainer::GetGlobal();
  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
    auto node_id = (*i)->GetId();
    auto pit_size = (*i)->GetObject<ns3::ndn::L3Protocol>()->getForwarder()->getPit().size();

     out << Simulator::Now().GetSeconds() <<","
      << node_id << ","
      << pit_size << ""
      << std::endl;
  }

  out.close();

  Simulator::Schedule (Seconds (5.0), &SetUpPitTrace);
}

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
  WifiHelper wifi;
  //wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifi.SetStandard(WIFI_PHY_STANDARD_80211n_2_4GHZ);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("OfdmRate6MbpsBW10MHz"));

  YansWifiChannelHelper wifiChannel; // = YansWifiChannelHelper::Default ();
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::ThreeLogDistancePropagationLossModel");
  wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel");

  Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (27));

  // YansWifiPhy wifiPhy = YansWifiPhy::Default();
  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
  wifiPhyHelper.SetChannel(wifiChannel.Create());
  wifiPhyHelper.Set("TxPowerStart", DoubleValue(5));
  wifiPhyHelper.Set("TxPowerEnd", DoubleValue(5));

  WifiMacHelper wifiMacHelper;
  wifiMacHelper.SetType("ns3::AdhocWifiMac");

  
  /*MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector(0.0, 30.0, 0.0));
  positionAlloc->Add (Vector(15.0, 30.0, 0.0));
  positionAlloc->Add (Vector(30.0, 30.0, 0.0));
  positionAlloc->Add (Vector(45.0, 30.0, 0.0));

  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  */

  int numNodes = 5;

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::WaypointMobilityModel");


  NodeContainer nodes;
  nodes.Create(numNodes);

  ////////////////
  // 1. Install Wifi
  NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, nodes);


  // 2. Install Mobility model
  mobility.Install(nodes);

  Ptr<WaypointMobilityModel> wayMobility[numNodes];
    
  for (uint32_t i = 0; i < numNodes; i++) {
    wayMobility[i] = nodes.Get(i)->GetObject<WaypointMobilityModel>();
    Waypoint waypointStart(Seconds(0), Vector3D(i*10, 0, 0));
    Waypoint waypointEnd(Seconds(100), Vector3D(i*10 + 100, 0, 0));

    wayMobility[i]->AddWaypoint(waypointStart);
    wayMobility[i]->AddWaypoint(waypointEnd);
    NS_LOG_INFO("Node " << i << " positions " << waypointStart << " " << waypointEnd);
  }

  // 3. Install NDN stack
  NS_LOG_INFO("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.setCsSize(1000);
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.Install(nodes);

  // Set BestRoute strategy
  //ndn::StrategyChoiceHelper::Install(nodes, "/", "/localhost/nfd/strategy/best-route-2");
  ns3::GlobalVariable::setSimulationEnd(100);
  ns3::GlobalVariable::setInterestCntTh(0.85);
  ns3::GlobalVariable::setSatisfactionRatioTh(0.15);
  ns3::GlobalVariable::setPrimaryTimer("5s");
  ns3::GlobalVariable::setSecondaryTimer("10s");
  ndn::MyStretegyChoiceHelper::InstallAll<nfd::fw::FifaStrategy>("/","/localhost/nfd/strategy/FifaStrategy/%FD%05");

  // 4. Set up applications
  NS_LOG_INFO("Installing Applications");

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbrFifa");
  consumerHelper.SetPrefix("/test/prefix");
  consumerHelper.SetAttribute("Frequency", DoubleValue(10));
  consumerHelper.SetAttribute("StopTime", TimeValue (Seconds(100)));
  consumerHelper.Install(nodes.Get(1));

  ndn::AppHelper producerHelper("ns3::ndn::ProducerFifa");
  producerHelper.SetPrefix("/test/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("512"));
  producerHelper.Install(nodes.Get(4));

  ndn::AppHelper attackerHelper("ns3::ndn::ConsumerCbrFifa");
  attackerHelper.SetPrefix("/test/fake");
  attackerHelper.SetAttribute("Frequency", DoubleValue(100));
  attackerHelper.SetAttribute ("StartTime", TimeValue (Seconds (25)));
  attackerHelper.SetAttribute("StopTime", TimeValue (Seconds(75)));
  attackerHelper.Install(nodes.Get(0));


  //Global routing helper
  ndn::GlobalRoutingHelper ndnGRH;
  ndnGRH.InstallAll();
  ndnGRH.AddOrigin("/test/prefix", nodes.Get(4));  
  ndnGRH.AddOrigin("/test/fake", nodes.Get(4));  
  ndn::GlobalRoutingHelper::CalculateRoutes();
  
  SetUpPitTrace();

  PrintCurrentTime ();

  Simulator::Stop(Seconds(100.0));

  ndn::L3RateTracer::InstallAll("/home/parth/Desktop/simulation_data/SimpleVanet/simple_vanet_trace_atck_fifa.txt", Seconds(5.0));

  std::ofstream out (filename.c_str ());

  out << "Time" << ","
      << "NodeId" << ","
      << "PIT_Size" << ""
      << std::endl;

  out.close();

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