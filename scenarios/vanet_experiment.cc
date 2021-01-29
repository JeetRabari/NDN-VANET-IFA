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

#include <fstream>
#include <iostream>
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/itu-r-1411-los-propagation-loss-model.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/integer.h"
#include "ns3/wave-bsm-helper.h"
#include "ns3/wave-helper.h"
#include "ns3/yans-wifi-helper.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

#include "ns3/ndnSIM-module.h"

#include "my-ndn-strategy-choice-helper.hpp"
#include "fifa-forwarding-strategy.hpp"

using namespace std;
using namespace ns3;


class Experiment{

  public:
    Experiment();
    ~Experiment();
    void RunSimulation ();
    void PrintCurrentTime ();
    void PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower);
    void PhyRxOk (std::string context, Ptr< const Packet > packet, double snr, WifiMode mode, WifiPreamble preamble);
    void DataTraceCallback (std::string context);
    void InterestTraceCallback ( shared_ptr< const ns3::ndn::Interest > interest, Ptr< ns3::ndn::App > app, shared_ptr< ns3::ndn::Face > face);
  private:
    void ConfigureNodes ();
    void ConfigureChannels ();
    void ConfigureDevices ();
    void ConfigureMobility ();
    void ConfigureApplications ();
    void SetUpNDN ();
    void SetUpProducers ();
    void SetUpConsumers ();
    void SetUpAttackers ();
    

    //member variables
    double m_TotalSimTime; ///< total sim time
    NodeContainer m_adhocTxNodes; ///< adhoc transmit nodes
    NetDeviceContainer m_adhocTxDevices; ///< adhoc transmit devices
    uint32_t m_nNodes; ///< number of nodes

    uint32_t m_nConsumers; ///< number of consumers
    uint32_t m_nProducers; ///< number of producers
    uint32_t m_nAttackers; ///< number of attackers
    DoubleValue m_normal_rate; ///< rate of consumers
    DoubleValue m_attack_rate; ///< rate of attackers
    std::string m_valid_prefix; ///< valid prefix
    std::string m_fake_prefix; ///< fake prefix

    uint32_t m_lossModel; ///< loss model (1=Friis;2=ItuR1411Los;3=TwoRayGround;4=LogDistance)
    uint32_t m_fading; ///< fading
    std::string m_lossModelName; ///< loss model name

    std::string m_phyMode; ///< phy mode 
    uint32_t m_80211mode; ///< 80211 mode (1=802.11p, 2=802.11b , 3=WAVE) 

    std::string m_phyModeB; ///< phy mode
    double m_txp; ///< distance

    uint32_t m_phyTxPkts; ///< phy transmit packets
    uint32_t m_phyTxBytes; ///< phy transmit bytes
    uint32_t m_phyRxPkts; ///< phy recevied packets
    uint32_t m_phyRxBytes; ///< phy recevied bytes

    int m_verbose; ///< verbose (0=active, 1=inactive)
    int m_asciiTrace; ///< ascii trace (0=active, 1=inactive)
    int m_pcap; ///< PCAP (0=active, 1=inactive) 
    std::string m_trName; ///< trace file name

    uint32_t m_mobility; ///< mobility (1=sumo, 2=synthetic highway)
    std::string m_traceFile; ///< trace file 
    int m_nodeSpeed; ///< in m/s
    int m_nodePause; ///< in s

    /// used to get consistent random numbers across scenarios
    int64_t m_streamIndex;
    int m_forwardingStrategy; //(1= best-route-2, 2=fifa)
    bool m_defaultRoutes; 
    ns3::ndn::GlobalRoutingHelper m_ndnGRH;

    uint32_t m_payloadSize;
    uint32_t m_ndnTxInterest;
    uint32_t m_ndnRxData;
};

Experiment::Experiment():
  m_TotalSimTime (500.0),
  m_adhocTxNodes (),
  m_nNodes (20),
  m_nConsumers (15),
  m_nProducers (5),
  m_nAttackers (0),
  m_normal_rate (10.0),
  m_attack_rate (200.0),
  m_valid_prefix ("/vanet"),
  m_fake_prefix ("/fake"),
  // Two-Ray ground
  m_lossModel (3),
  m_fading (0),
  m_lossModelName (""),
  m_phyMode ("OfdmRate6MbpsBW10MHz"),
  // 1=802.11p
  m_80211mode (1),
  m_phyModeB ("DsssRate11Mbps"),
  m_txp (20),
  m_verbose (0),
  m_asciiTrace (0),
  m_pcap (0),
  m_trName ("vanet-experiment"),
  m_phyTxPkts (0),
  m_phyTxBytes (0),
  m_phyRxPkts (0),
  m_phyRxBytes (0),
  m_mobility (2),
  m_traceFile (""),
  m_streamIndex (0),
  m_nodeSpeed (20),
  m_nodePause (0),
  m_forwardingStrategy (1),
  m_defaultRoutes (true),
  m_payloadSize (512),
  m_ndnGRH (),
  m_ndnTxInterest (0),
  m_ndnRxData (0)
{
}

Experiment:: ~Experiment()
{
}

void
Experiment::PrintCurrentTime ()
{
  std::cout << "Simulation at: " << Simulator::Now().As(Time::S) << std::endl;
  Simulator::Schedule(Seconds(10.0), &Experiment::PrintCurrentTime, this);
}

void
Experiment::ConfigureNodes()
{
  m_adhocTxNodes.Create (m_nNodes);
}

void
Experiment::ConfigureChannels()
{
  if (m_lossModel == 1)
    {
      m_lossModelName = "ns3::FriisPropagationLossModel";
    }
  else if (m_lossModel == 2)
    {
      m_lossModelName = "ns3::ItuR1411LosPropagationLossModel";
    }
  else if (m_lossModel == 3)
    {
      m_lossModelName = "ns3::TwoRayGroundPropagationLossModel";
    }
  else if (m_lossModel == 4)
    {
      m_lossModelName = "ns3::LogDistancePropagationLossModel";
    }
  else
    {
      // Unsupported propagation loss model.
      // Treating as ERROR
      //NS_LOG_ERROR ("Invalid propagation loss model specified.  Values must be [1-4], where 1=Friis;2=ItuR1411Los;3=TwoRayGround;4=LogDistance");
    }

  // frequency
  double freq = 0.0;
  if ((m_80211mode == 1)
      || (m_80211mode == 3))
    {
      // 802.11p 5.9 GHz
      freq = 5.9e9;
    }
  else
    {
      // 802.11b 2.4 GHz
      freq = 2.4e9;
    }

  // Setup propagation models
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  if (m_lossModel == 3)
    {
      // two-ray requires antenna height (else defaults to Friss)
      wifiChannel.AddPropagationLoss (m_lossModelName, "Frequency", DoubleValue (freq), "HeightAboveZ", DoubleValue (1.5));
    }
  else
    {
      wifiChannel.AddPropagationLoss (m_lossModelName, "Frequency", DoubleValue (freq));
    }

  // Propagation loss models are additive.
  if (m_fading != 0)
    {
      // if no obstacle model, then use Nakagami fading if requested
      wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
    }

  // the channel
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();

  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (channel);
  // ns-3 supports generate a pcap trace
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);

  YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
  wavePhy.SetChannel (channel);
  wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);

  // Setup WAVE PHY and MAC
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  WaveHelper waveHelper = WaveHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  if (m_verbose)
    {
      wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
      // likewise, turn on WAVE PHY logging
      waveHelper.EnableLogComponents ();
    }

  WifiHelper wifi;

  // Setup 802.11b stuff
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (m_phyModeB),
                                "ControlMode",StringValue (m_phyModeB));

  // Setup 802.11p stuff
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (m_phyMode),
                                      "ControlMode",StringValue (m_phyMode));

  // Setup WAVE-PHY stuff
  waveHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (m_phyMode),
                                      "ControlMode",StringValue (m_phyMode));

  // Set Tx Power
  wifiPhy.Set ("TxPowerStart",DoubleValue (m_txp));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txp));
  wavePhy.Set ("TxPowerStart",DoubleValue (m_txp));
  wavePhy.Set ("TxPowerEnd", DoubleValue (m_txp));

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();

  // Setup net devices

  if (m_80211mode == 3)
    {
      m_adhocTxDevices = waveHelper.Install (wavePhy, waveMac, m_adhocTxNodes);
    }
  else if (m_80211mode == 1)
    {
      m_adhocTxDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, m_adhocTxNodes);
    }
  else
    {
      m_adhocTxDevices = wifi.Install (wifiPhy, wifiMac, m_adhocTxNodes);
    }

  if (m_asciiTrace != 0)
    {
      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (m_trName + ".tr").c_str ());
      wifiPhy.EnableAsciiAll (osw);
      wavePhy.EnableAsciiAll (osw);
    }
  if (m_pcap != 0)
    {
      wifiPhy.EnablePcapAll ("vanet-routing-compare-pcap");
      wavePhy.EnablePcapAll ("vanet-routing-compare-pcap");
    }
}

void
Experiment::SetUpNDN ()
{
  ns3::ndn::StackHelper stackHelper;
  stackHelper.setCsSize (1000);
  stackHelper.setPolicy ("nfd::cs::lru");

  stackHelper.SetDefaultRoutes(m_defaultRoutes);

  stackHelper.InstallAll();

  if (m_forwardingStrategy == 1){
    ns3::ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route-2");
  }else if (m_forwardingStrategy == 2){
    ns3::ndn::MyStretegyChoiceHelper::InstallAll<nfd::fw::FifaStrategy>("/","/localhost/nfd/strategy/FifaStrategy/%FD%05");
  }

  m_ndnGRH.InstallAll();
}

void
Experiment::ConfigureDevices () 
{
  SetUpNDN();

  Config::Connect ("/NodeList/*/DeviceList/*/Phy/State/Tx", MakeCallback (&Experiment::PhyTxTrace, this));
  Config::Connect ("/NodeList/*/DeviceList/*/Phy/State/RxOk", MakeCallback (&Experiment::PhyRxOk, this));
  //Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::ndn::ConsumerFifa/ReceivedData", MakeCallback (&Experiment::DataTraceCallback, this));
  //Config::ConnectWithoutContext ("/NodeList/*/ApplicationList/*/$ns3::ndn::App/TransmittedInterests", MakeCallback (&Experiment::InterestTraceCallback, this));

}

void
Experiment::ConfigureMobility ()
{
  if (m_mobility == 1)
    {
      // Create Ns2MobilityHelper with the specified trace log file as parameter
      Ns2MobilityHelper ns2 = Ns2MobilityHelper (m_traceFile);
      ns2.Install (); // configure movements for each node, while reading trace file
      // initially assume all nodes are not moving
      WaveBsmHelper::GetNodesMoving ().resize (m_nNodes, 0);
    }
  else if (m_mobility == 2)
    {
      MobilityHelper mobilityAdhoc;

      ObjectFactory pos;
      pos.SetTypeId ("ns3::RandomBoxPositionAllocator");
      pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
      pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
      // we need antenna height uniform [1.0 .. 2.0] for loss model
      pos.Set ("Z", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=2.0]"));

      Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
      m_streamIndex += taPositionAlloc->AssignStreams (m_streamIndex);

      std::stringstream ssSpeed;
      ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_nodeSpeed << "]";
      std::stringstream ssPause;
      ssPause << "ns3::ConstantRandomVariable[Constant=" << m_nodePause << "]";
      mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                      "Speed", StringValue (ssSpeed.str ()),
                                      "Pause", StringValue (ssPause.str ()),
                                      "PositionAllocator", PointerValue (taPositionAlloc));
      mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
      mobilityAdhoc.Install (m_adhocTxNodes);
      m_streamIndex += mobilityAdhoc.AssignStreams (m_adhocTxNodes, m_streamIndex);

      // initially assume all nodes are moving
      WaveBsmHelper::GetNodesMoving ().resize (m_nNodes, 1);
    }

  // Configure callback for logging
  //Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
  //                 MakeBoundCallback (&VanetRoutingExperiment::CourseChange, &m_os));
}

void
Experiment::SetUpProducers () {

  ns3::ndn::AppHelper producerApp ("ns3::ndn::Producer");
  producerApp.SetPrefix (m_valid_prefix);
  producerApp.SetAttribute ("PayloadSize", UintegerValue(m_payloadSize));
  producerApp.SetAttribute ("StopTime", TimeValue(Seconds(m_TotalSimTime)));

  for(int i = 0 ; i < m_nProducers ; i++) {
    producerApp.Install (m_adhocTxNodes.Get(i));
    m_ndnGRH.AddOrigins (m_valid_prefix, m_adhocTxNodes.Get(i));
    m_ndnGRH.AddOrigins (m_fake_prefix, m_adhocTxNodes.Get(i));
  }

  m_ndnGRH.CalculateRoutes();
}

void
Experiment::SetUpConsumers ()
{
  ns3::ndn::AppHelper consumerApp("ns3::ndn::ConsumerCbrFifa");
  consumerApp.SetPrefix (m_valid_prefix);
  consumerApp.SetAttribute ("Frequency", m_normal_rate);
  consumerApp.SetAttribute ("StopTime", TimeValue (Seconds (m_TotalSimTime)));

  for(int i = m_nProducers ; i < m_nNodes ; i++){
    consumerApp.Install (m_adhocTxNodes.Get(i));
  }
}

void
Experiment::SetUpAttackers ()
{
  if(m_nAttackers > 0)
  {

  }
}

void
Experiment::ConfigureApplications ()
{
  SetUpProducers();
  SetUpConsumers();
  SetUpAttackers();
}

void
Experiment::DataTraceCallback (std::string context)
{
  std::cout << "IN TRACE BACK" << std::endl;
  ++m_ndnRxData;
}

void
Experiment::InterestTraceCallback ( shared_ptr< const ns3::ndn::Interest > interest, Ptr< ns3::ndn::App > app, shared_ptr< ns3::ndn::Face > face)
{
  ++m_ndnTxInterest;
}


void
Experiment::PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower)
{
  ++m_phyTxPkts;
  uint32_t pktSize = packet->GetSize ();
  m_phyTxBytes += pktSize;
}

void
Experiment::PhyRxOk (std::string context, Ptr< const Packet > packet, double snr, WifiMode mode, WifiPreamble preamble)
{
  ++m_phyRxPkts;
  uint32_t pktSize = packet->GetSize ();
  m_phyRxBytes += pktSize;
}

void
Experiment::RunSimulation()
{

  ConfigureNodes (); //done
  ConfigureChannels (); //done
  ConfigureDevices (); //done
  ConfigureMobility (); //done
  ConfigureApplications (); 

  Simulator::Schedule(Seconds(0.0), &Experiment::PrintCurrentTime, this);
  Simulator::Stop (Seconds (m_TotalSimTime));
  Simulator::Run ();
  Simulator::Destroy ();

  std::cout << m_phyTxBytes << "\t" << m_phyRxBytes << std::endl;
  std::cout << m_ndnTxInterest <<"\t" <<m_ndnRxData << std::endl;
}

int
main(int argc, char* argv[])
{
  Experiment myExp;
  myExp.RunSimulation();
  return 0;
}