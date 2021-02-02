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
  //ndn::MyStretegyChoiceHelper::InstallAll<nfd::fw::FifaStrategy>("/","/localhost/nfd/strategy/FifaStrategy/%FD%05");
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route-2");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> producer = grid.GetNode(2, 2);
  NodeContainer consumerNodes;
  consumerNodes.Add(grid.GetNode(1, 1));

  Ptr<Node> attacker = grid.GetNode(2,0);

  // Install NDN applications
  std::string prefix = "/prefix";

  TimeValue stop = Seconds(20);
  TimeValue start = Seconds(5);
  TimeValue stopatck = Seconds(15);

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbrFifa");
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Frequency", StringValue("10")); // 100 interests a second
  consumerHelper.SetAttribute("StopTime", stop);
  consumerHelper.Install(consumerNodes);

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix(prefix);
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(producer);

  ndn::AppHelper atckHelper("ns3::ndn::ConsumerCbrFifa");
  atckHelper.SetPrefix("/fake");
  atckHelper.SetAttribute("Frequency", StringValue("100")); // 2000 interests a second
  atckHelper.SetAttribute("StartTime", start);
  atckHelper.SetAttribute("StopTime", stopatck);
  atckHelper.Install(attacker);


  // Add /prefix origins to ndn::GlobalRouter
  ndnGlobalRoutingHelper.AddOrigins(prefix, producer);
  ndnGlobalRoutingHelper.AddOrigins("/fake", producer);

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();


  

  Simulator::Stop(Seconds(20.0));

  Simulator::Run();

   for(int i = 0 ; i < 3 ; i++)
  {
    for (int j = 0 ; j < 3 ; j++)
    {
      printFIB(grid.GetNode(i,j));
    }
  }
  
  Simulator::Destroy();

 

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}