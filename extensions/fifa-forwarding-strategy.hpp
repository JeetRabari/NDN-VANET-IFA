/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFD_DAEMON_FW_BEST_ROUTE_STRATEGY2_HPP
#define NFD_DAEMON_FW_BEST_ROUTE_STRATEGY2_HPP

#include "ns3/ndnSIM/NFD/daemon/fw/strategy.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/process-nack-traits.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/retx-suppression-exponential.hpp"

// Addition
#include "RecordTable.hpp"
#include "MaliciousVehicleTable.hpp"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/err.h"

namespace nfd {
namespace fw {

/** \brief Best Route strategy version 4
 *
 *  This strategy forwards a new Interest to the lowest-cost nexthop (except downstream).
 *  After that, if consumer retransmits the Interest (and is not suppressed according to
 *  exponential backoff algorithm), the strategy forwards the Interest again to
 *  the lowest-cost nexthop (except downstream) that is not previously used.
 *  If all nexthops have been used, the strategy starts over with the first nexthop.
 *
 *  This strategy returns Nack to all downstreams with reason NoRoute
 *  if there is no usable nexthop, which may be caused by:
 *  (a) the FIB entry contains no nexthop;
 *  (b) the FIB nexthop happens to be the sole downstream;
 *  (c) the FIB nexthops violate scope.
 *
 *  This strategy returns Nack to all downstreams if all upstreams have returned Nacks.
 *  The reason of the sent Nack equals the least severe reason among received Nacks.
 *
 *  \note This strategy is not EndpointId-aware.
 */

//  +++++++++++++++++++++++++++++++++++++++
//  + Modification of BestRouteStrategy2  +
//  +++++++++++++++++++++++++++++++++++++++
class FifaStrategy : public Strategy
                         , public ProcessNackTraits<FifaStrategy>
{
public:
  explicit
  FifaStrategy(Forwarder& forwarder, const Name& name = getStrategyName());

  static const Name&
  getStrategyName();

  void
  afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
                       const shared_ptr<pit::Entry>& pitEntry) override;

  void
  afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
                   const shared_ptr<pit::Entry>& pitEntry) override;

  void 
  beforeSatisfyInterest(const shared_ptr<pit::Entry>& pitEntry,const FaceEndpoint& ingress, 
                        const Data& data) override;

  void primaryMethod(void);

  void secondaryMethod(void);

private:
PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  static const time::milliseconds RETX_SUPPRESSION_INITIAL;
  static const time::milliseconds RETX_SUPPRESSION_MAX;
  RetxSuppressionExponential m_retxSuppression;

  friend ProcessNackTraits<FifaStrategy>;

private:
  RecordTable m_recordTable;
  MaliciousVehicleTable m_maliciousTable;
  unordered_map<std::string, int> map;
  std::string m_VID;

  std::string decrypt (std::string cipherText, std::string publicKey);
};

} // namespace fw
} // namespace nfd

#endif // NFD_DAEMON_FW_BEST_ROUTE_STRATEGY2_HPP
