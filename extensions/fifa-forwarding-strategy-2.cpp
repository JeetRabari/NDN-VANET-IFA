#include "fifa-forwarding-strategy-2.hpp"

#include <boost/random/uniform_int_distribution.hpp>

#include <ndn-cxx/util/random.hpp>

#include "daemon/common/logger.hpp"

// new addition
#include "ns3/core-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/network-module.h"

NFD_LOG_INIT("RandomLoadBalancerStrategy");

namespace nfd {
namespace fw {

FifaStrategy2::FifaStrategy2(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
{
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

FifaStrategy2::~FifaStrategy2()
{
}

static bool
canForwardToNextHop(const Face& inFace, shared_ptr<pit::Entry> pitEntry, const fib::NextHop& nexthop)
{
  return !wouldViolateScope(inFace, pitEntry->getInterest(), nexthop.getFace()) &&
    canForwardToLegacy(*pitEntry, nexthop.getFace());
}

static bool
hasFaceForForwarding(const Face& inFace, const fib::NextHopList& nexthops, const shared_ptr<pit::Entry>& pitEntry)
{
  return std::find_if(nexthops.begin(), nexthops.end(), bind(&canForwardToNextHop, cref(inFace), pitEntry, _1))
         != nexthops.end();
}

void
FifaStrategy2::afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
                                                 const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_TRACE("afterReceiveInterest");

  
  if (hasPendingOutRecords(*pitEntry)) {
    // not a new Interest, don't forward
    return;
  }
  
  // Get Application Parameter
  const uint8_t *ptr = interest.getApplicationParameters().value();
  std::string myStr((char *)ptr);
  std::cout << "ATTENTION : " << myStr << std::endl;

  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  // Ensure there is at least 1 Face is available for forwarding
  if (!hasFaceForForwarding(ingress.face, nexthops, pitEntry)) {
    this->rejectPendingInterest(pitEntry);
    return;
  }

  fib::NextHopList::const_iterator selected;
  do {
    boost::random::uniform_int_distribution<> dist(0, nexthops.size() - 1);
    const size_t randomIndex = dist(m_randomGenerator);

    uint64_t currentIndex = 0;

    for (selected = nexthops.begin(); selected != nexthops.end() && currentIndex != randomIndex;
         ++selected, ++currentIndex) {
    }
  } while (!canForwardToNextHop(ingress.face, pitEntry, *selected));

  this->sendInterest(pitEntry, FaceEndpoint(selected->getFace(), 0), interest);
}

const Name&
FifaStrategy2::getStrategyName()
{
  static Name strategyName("ndn:/localhost/nfd/strategy/fifa-forwarding-strategy-2/%FD%01");
  return strategyName;
}

} // namespace fw
} // namespace nfd