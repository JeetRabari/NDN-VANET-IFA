// ndn-grid.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"
#include "fifa-forwarding-strategy.hpp"
#include "GlobalVariable.hpp"
#include "my-ndn-strategy-choice-helper.hpp"

namespace ns3 {

int
main(int argc, char* argv[])
{
  // Setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("10p"));

  //Global Variable
  ns3::GlobalVariable::setSimulationEnd(20);
  ns3::GlobalVariable::setInterestCntTh(0.75);
  ns3::GlobalVariable::setSatisfactionRatioTh(0.35);
  ns3::GlobalVariable::setPrimaryTimer("5s");
  ns3::GlobalVariable::setSecondaryTimer("10s");

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating 3x3 topology
  PointToPointHelper p2p;
  PointToPointGridHelper grid(3, 3, p2p);
  grid.BoundingBox(100, 100, 200, 200);

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.setCsSize(1000);
  ndnHelper.InstallAll();

  // Set BestRoute strategy
  ndn::MyStretegyChoiceHelper::InstallAll<nfd::fw::FifaStrategy>("/","/localhost/nfd/strategy/FifaStrategy/%FD%05");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> producer = grid.GetNode(2, 2);
  NodeContainer consumerNodes;
  consumerNodes.Add(grid.GetNode(0, 0));

  Ptr<Node> attacker = grid.GetNode(2,0);

  // Install NDN applications
  std::string prefix = "/prefix";

  TimeValue stop = Seconds(20);
  TimeValue start = Seconds(5);
  TimeValue stopatck = Seconds(15);

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbrFifa");
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Frequency", StringValue("100")); // 100 interests a second
  consumerHelper.SetAttribute("StopTime", stop);
  consumerHelper.Install(consumerNodes);

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix(prefix);
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(producer);

  ndn::AppHelper atckHelper("ns3::ndn::ConsumerCbrFifa");
  atckHelper.SetPrefix("/prefix/fake");
  atckHelper.SetAttribute("Frequency", StringValue("2000")); // 2000 interests a second
  atckHelper.SetAttribute("StartTime", start);
  atckHelper.SetAttribute("StopTime", stopatck);
  atckHelper.Install(attacker);


  // Add /prefix origins to ndn::GlobalRouter
  ndnGlobalRoutingHelper.AddOrigins(prefix, producer);

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(20.0));

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