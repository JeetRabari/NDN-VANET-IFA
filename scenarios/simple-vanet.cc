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

void
printFIB(Ptr<Node> node)
{

  std::cout << "FIB for " << node->GetId() << std::endl;
  ndn::nfd::Fib::const_iterator it = node->GetObject<ns3::ndn::L3Protocol>()->getForwarder()->getFib().begin();
  ndn::nfd::Fib::const_iterator end = node->GetObject<ns3::ndn::L3Protocol>()->getForwarder()->getFib().end();

  while ( it != end )
  {
    std::cout << it->getPrefix().toUri() << "\t";
  
    nfd::fib::NextHopList nh = it->getNextHops();

    for(auto a = nh.begin() ; a != nh.end() ; a++)
    {
      std::cout <<"L: "<< a->getFace().getLocalUri() <<
                "R: " << a->getFace().getRemoteUri() << "\t";
    }
    
    it++;
    std::cout << endl;
  }
  
  std::cout << endl;
}


void 
interestTB (shared_ptr<const ndn::Interest> in, Ptr<ns3::ndn::App> app, shared_ptr<ns3::ndn::Face> face)
{
  std::cout << "In traceback" << std::endl;
}


int
main(int argc, char* argv[])
{
  // disable fragmentation
  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue("OfdmRate24Mbps"));

  CommandLine cmd;
  cmd.Parse(argc, argv);

  //////////////////////
  //////////////////////
  //////////////////////
  WifiHelper wifi;
  // wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifi.SetStandard(WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("OfdmRate24Mbps"));

  YansWifiChannelHelper wifiChannel; // = YansWifiChannelHelper::Default ();
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::ThreeLogDistancePropagationLossModel");
  wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");

  // YansWifiPhy wifiPhy = YansWifiPhy::Default();
  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
  wifiPhyHelper.SetChannel(wifiChannel.Create());
  wifiPhyHelper.Set("TxPowerStart", DoubleValue(5));
  wifiPhyHelper.Set("TxPowerEnd", DoubleValue(5));

  WifiMacHelper wifiMacHelper;
  wifiMacHelper.SetType("ns3::AdhocWifiMac");

  
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector(10.0, 10.0, 0.0));
  positionAlloc->Add (Vector(20.0, 10.0, 0.0));
  positionAlloc->Add (Vector(30.0, 10.0, 0.0));

  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  NodeContainer nodes;
  nodes.Create(3);

  ////////////////
  // 1. Install Wifi
  NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, nodes);


  // 2. Install Mobility model
  mobility.Install(nodes);

  // 3. Install NDN stack
  NS_LOG_INFO("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  // ndnHelper.AddNetDeviceFaceCreateCallback (WifiNetDevice::GetTypeId (), MakeCallback
  // (MyNetDeviceFaceCallback));
  ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.setCsSize(1000);
  //ndnHelper.SetDefaultRoutes(true);
  ndnHelper.Install(nodes);

  // Set BestRoute strategy
  //ndn::StrategyChoiceHelper::Install(nodes, "/test/prefix", "/localhost/nfd/strategy/best-route");
  ns3::GlobalVariable::setSimulationEnd(100);
  ns3::GlobalVariable::setInterestCntTh(0.75);
  ns3::GlobalVariable::setSatisfactionRatioTh(0.35);
  ns3::GlobalVariable::setPrimaryTimer("100s");
  ns3::GlobalVariable::setSecondaryTimer("100s");
  ndn::MyStretegyChoiceHelper::InstallAll<nfd::fw::FifaStrategy>("/","/localhost/nfd/strategy/FifaStrategy/%FD%05");

  // 4. Set up applications
  NS_LOG_INFO("Installing Applications");

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbrFifa");
  consumerHelper.SetPrefix("/test/prefix");
  //consumerHelper.SetAttribute("MaxSeq", UintegerValue(200));
  consumerHelper.SetAttribute("StopTime", TimeValue (Seconds(100)));
  consumerHelper.Install(nodes.Get(0));

  ndn::AppHelper producerHelper("ns3::ndn::ProducerFifa");
  producerHelper.SetPrefix("/test/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1200"));
  producerHelper.Install(nodes.Get(2));


  //Global routing helper
  ndn::GlobalRoutingHelper ndnGRH;
  ndnGRH.InstallAll();
  ndnGRH.AddOrigin("/test/prefix", nodes.Get(2));  
  ndn::GlobalRoutingHelper::CalculateRoutes();
  


  Simulator::Stop(Seconds(100.0));

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