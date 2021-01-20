#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ndnSIM/apps/ndn-producer.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/helper/ndn-app-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include <ns3/ndnSIM/helper/ndn-global-routing-helper.hpp>
#include "ns3/animation-interface.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/ndnSIM-module.h"

// SUMO mobility
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"

#include <bits/stdc++.h>

#include "fifa-forwarding-strategy.hpp"
#include "ndn-consumer-fifa.hpp"
#include "GlobalVariable.hpp"

namespace ns3{

/**
 * This scenario simulates a scenario with 6 cars movind and communicating
 * in an ad-hoc way.
 *
 * 5 consumers request data from producer with frequency 1 interest per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-v2v-simple
 *
 * To modify the mobility model, see function installMobility.
 * To modify the wifi model, see function installWifi.
 * To modify the NDN settings, see function installNDN and for consumer and
 * producer settings, see functions installConsumer and installProducer
 * respectively.
 */

NS_LOG_COMPONENT_DEFINE ("V2VSimple");


static const uint32_t numNodes = 5;
static const std::string valid_prefix("v2v");
static const std::string fake_prefix("v2v/fake");
static const size_t cs_size = 1000;
static const std::string output_path("/home/parth/Desktop/simulation_data/");
static const DoubleValue normal_rate(10.0);
static const DoubleValue attacker_rate(200.0);
static const TimeValue attack_start_time = Seconds(25.0);
static const TimeValue attack_stop_time = Seconds(75.0);
static const TimeValue apps_stop_time = Seconds(100.0);
static const int simulationEnd = 100;
static const std::string traceFile("/home/parth/Desktop/sumo_files/mobility.tcl");
static const std::string producerIdFile("/home/parth/Desktop/simulation_data/prodID.txt");


void printPosition(Ptr<const MobilityModel> mobility) //DEBUG purpose
{
  Simulator::Schedule(Seconds(1), &printPosition, mobility);
  NS_LOG_INFO("Car "<<  mobility->GetObject<Node>()->GetId() << " is at: " <<mobility->GetPosition());
}


void installMobility(NodeContainer &c, int simulationEnd)
{
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::WaypointMobilityModel");
  mobility.Install(c);

  bool test = true;
  if(test){
    Ptr<WaypointMobilityModel> wayMobility[numNodes];
    for (uint32_t i = 0; i < numNodes; i++) {
      wayMobility[i] = c.Get(i)->GetObject<WaypointMobilityModel>();
      Waypoint waypointStart(Seconds(0), Vector3D(i*10, 0, 0));
      Waypoint waypointEnd(Seconds(simulationEnd), Vector3D(i*10+100, 0, 0));

      wayMobility[i]->AddWaypoint(waypointStart);
      wayMobility[i]->AddWaypoint(waypointEnd);
      NS_LOG_INFO("Node " << i << " positions " << waypointStart << " " << waypointEnd);
    }


    return;
  }
  Ptr<WaypointMobilityModel> wayMobility[numNodes];
  for (uint32_t i = 0; i < numNodes; i++) {
    wayMobility[i] = c.Get(i)->GetObject<WaypointMobilityModel>();
    Waypoint waypointStart(Seconds(0), Vector3D(i*10, 0, 0));
    Waypoint waypointMiddle(Seconds(simulationEnd/2), Vector3D(i*20+1000, 0, 0));
    Waypoint waypointEnd(Seconds(simulationEnd+1), Vector3D(i*20+1000, 0, 0));

    wayMobility[i]->AddWaypoint(waypointStart);
    wayMobility[i]->AddWaypoint(waypointMiddle);
    wayMobility[i]->AddWaypoint(waypointEnd);
    NS_LOG_INFO("Node " << i << " positions " << waypointStart << " " << waypointMiddle << " " << waypointEnd);
  }



}

void installMobilitySumo()
{
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
    ns2.Install();
}

void installWifi(NodeContainer &c, NetDeviceContainer &devices)
{
  // Modulation and wifi channel bit rate
  std::string phyMode("OfdmRate6MbpsBW10MHz");

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

  // Use 802.11p wifi helper
  Wifi80211pHelper wifi = Wifi80211pHelper::Default();

  // Disable rate control
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue(phyMode),
                               "ControlMode", StringValue(phyMode));

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  wifiPhy.Set("TxPowerStart", DoubleValue(20));
  wifiPhy.Set("TxPowerEnd", DoubleValue(50));
  wifiPhy.Set("TxPowerLevels", UintegerValue(5));

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                 "MaxRange", DoubleValue(19.0));
  wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
                                 "m0", DoubleValue(1.0),
                                 "m1", DoubleValue(1.0),
                                 "m2", DoubleValue(1.0));
  wifiPhy.SetChannel(wifiChannel.Create());

  // Add a non-QoS wave upper mac
  NqosWaveMacHelper wifiMac = NqosWaveMacHelper::Default();

  devices = wifi.Install(wifiPhy, wifiMac, c);

  //wifiPhy.EnablePcap(output_path+"packet_trace_data", devices , true);
}

void installNDN(NodeContainer &c)
{
  ndn::StackHelper ndnHelper;
  ndnHelper.setCsSize(cs_size);
  ndnHelper.Install(c);  
}

void installConsumer(NodeContainer &c)
{
  ndn::AppHelper helper("ns3::ndn::ConsumerCbrFifa");
  helper.SetAttribute("Frequency", normal_rate);
  helper.SetAttribute("Randomize", StringValue("uniform"));
  helper.SetPrefix(valid_prefix+"/file");
  helper.SetAttribute("StopTime", apps_stop_time);
  helper.Install(c);
}

void installProducer(NodeContainer &c)
{
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix(valid_prefix+"/file");

  producerHelper.Install(c);
}

void installAttacker(NodeContainer &c){
    ndn::AppHelper helper("ns3::ndn::ConsumerCbrFifa");
    helper.SetAttribute("Frequency", attacker_rate);
    helper.SetAttribute("Randomize", StringValue("exponential"));
    helper.SetAttribute("StartTime", attack_start_time);
    helper.SetAttribute("StopTime", attack_stop_time);
    helper.SetPrefix(valid_prefix+"/file/fake");

    helper.Install(c);
}

void printCurrentSimulationTime(){
  std::cout << "Simulation at time: " << Simulator::Now().As(Time::S) << "\n";
}

void getRandomNumbersInFile(int min, int max, int cnt)
{
  
  if(max-min < cnt){
    std::cout << "Count out of range" << std::endl;
    return;
  }

  std::unordered_set<int> set;
  std::ofstream file;

  file.open(producerIdFile, ios::in | ios::out | ios::trunc);

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x -> SetAttribute("Min", DoubleValue(min));
  x -> SetAttribute("Max", DoubleValue(max));

  while(cnt > 0){
    int val = (int)(x -> GetValue());

    if(set.find(val) == set.end()){
      set.insert(val);
      file << val << endl;
      cnt--;
    }
  }

  file.close();

  return;
}

std::unordered_set<int> getIdsFromFile(){
  std::ifstream file;
  unordered_set<int> set;

  file.open(producerIdFile);

  string id;

  while(getline(file, id)){
    set.insert(stoi(id));
  }

  return set;
}

// PIT entry size
/*
void pitTracer(Ptr<Node> node){
  Ptr<ndn::nfd::Pit> pit = node->GetObject<ndn::nfd::Pit>();
  std::cout << Simulator::Now().ToDouble(Time::S) << "\t"
            << node->GetId() << "\t"
            << Names::FindName (node) << "\t"
            << pit->size() << endl;
}
*/

int main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("V2VTest Simulator");

  // Setting Global Variables
  ns3::GlobalVariable::setSimulationEnd(simulationEnd);
  ns3::GlobalVariable::setInterestCntTh(0.75);
  ns3::GlobalVariable::setSatisfactionRatioTh(0.35);
  ns3::GlobalVariable::setPrimaryTimer("10s");
  ns3::GlobalVariable::setSecondaryTimer("20s");

  uint32_t numProducer = 1;
  uint32_t numConsumer = numNodes - numProducer;
  uint32_t numAttacker = 0;

  NodeContainer c;
  c.Create(numNodes); 

  installMobility(c, simulationEnd);
  //installMobilitySumo();

  NetDeviceContainer netDevices;
  installWifi(c, netDevices);


  installNDN(c);

  //setting application
  NodeContainer producer;
  NodeContainer consumers;
  NodeContainer attacker;

  /*
  std::unordered_set<int> producer_ids;

  getRandomNumbersInFile(0, numNodes, numProducer);

  producer_ids = getIdsFromFile();

  std::unordered_set<int> :: iterator itr;

  for(itr = producer_ids.begin() ; itr != producer_ids.end() ; itr++){
    producer.Add(c.Get(*itr));
  }


  for(int i = 0 ; i < numNodes ; i++){
    if(producer_ids.find(i) != producer_ids.end()) continue;
    consumers.Add(c.Get(i));
  }
  */

  

  
    //Mobility scenario 1 
  producer.Add(c.Get(1));
  //producer.Add(c.Get(0));
  //producer.Add(c.Get(5));
  //producer.Add(c.Get(10));
  //producer.Add(c.Get(15));
  //producer.Add(c.Get(20));
  
  for(int i=0; i<numNodes; i++){
    if(i!=1 && i != 5 && i != 10 && i != 15 && i != 20 /*&& i!=44*/){ //remove i!=44
      consumers.Add(c.Get(i));
    }
  }
  if(numAttacker > 0) attacker.Add(c.Get(numNodes + numAttacker - 1));//numNodes + numAttacker - 1));

  installConsumer(consumers);
  installProducer(producer);

  // install producer in all consumer also
  //installProducer(consumers);
  

  if(numAttacker > 0) installAttacker(attacker);

  // Routing header
  ndn::GlobalRoutingHelper ndnGRH;
  ndnGRH.InstallAll();
  ndnGRH.AddOrigins(valid_prefix+"/file", producer);
  ndn::GlobalRoutingHelper::CalculateRoutes();

  // forwarding stretagy
  ndn::StrategyChoiceHelper::InstallAll(valid_prefix, "/localhost/nfd/strategy/best-route-2");
  //ndn::StrategyChoiceHelper::InstallAll(fake_prefix, "/localhost/nfd/strategy/ncc");


  //ndn::StrategyChoiceHelper::InstallAll<nfd::fw::FifaStrategy>(fake_prefix);
  //ndn::StrategyChoiceHelper::InstallAll<nfd::fw::FifaStrategy>(valid_prefix);

  Simulator::Stop(Seconds(simulationEnd));
  
  // trace details
  ndn::L3RateTracer::InstallAll(output_path+"rate-trace_100"+ (numAttacker > 0 ? "_atck":"")+".txt", Seconds(5.0));

  // PIT tracer
  /*for(int i = 5 ; i < simulationEnd ; i += 5) {
    for(int j = 0 ; j < consumers.size() ; j++) {
      Simulator::Schedule(Seconds(i), &pitTracer, consumers.Get(j));
    }
  }*/

  for ( int i = 0 ; i < simulationEnd ; i += 10){
    Simulator::Schedule(Seconds(i), &printCurrentSimulationTime);
  }

  std::string animFile = output_path+"v2v-test.xml";
  AnimationInterface anim(animFile);
  Simulator::Run ();
  return 0;
}
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}