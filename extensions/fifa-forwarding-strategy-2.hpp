#ifndef NDNSIM_EXAMPLES_NDN_LOAD_BALANCER_RANDOM_LOAD_BALANCER_STRATEGY_HPP
#define NDNSIM_EXAMPLES_NDN_LOAD_BALANCER_RANDOM_LOAD_BALANCER_STRATEGY_HPP

#include <boost/random/mersenne_twister.hpp>
#include "face/face.hpp"
#include "fw/strategy.hpp"
#include "fw/algorithm.hpp"

namespace nfd {
namespace fw {

class FifaStrategy2 : public Strategy {
public:
  FifaStrategy2(Forwarder& forwarder, const Name& name = getStrategyName());

  virtual
  ~FifaStrategy2() override;

  void
  afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
                       const shared_ptr<pit::Entry>& pitEntry) override;

  static const Name&
  getStrategyName();

protected:
  boost::random::mt19937 m_randomGenerator;
};

} // namespace fw
} // namespace nfd

#endif // NDNSIM_EXAMPLES_NDN_LOAD_BALANCER_RANDOM_LOAD_BALANCER_STRATEGY_HPP