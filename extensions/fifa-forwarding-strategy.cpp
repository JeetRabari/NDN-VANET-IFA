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

#include "fifa-forwarding-strategy.hpp"
#include "algorithm.hpp"
#include "common/logger.hpp"

// +++++++++ New Addition ++++++++++
#include "GlobalVariable.hpp"



namespace nfd {
namespace fw {

NFD_LOG_INIT(FifaStrategy);
NFD_REGISTER_STRATEGY(FifaStrategy);

const time::milliseconds FifaStrategy::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds FifaStrategy::RETX_SUPPRESSION_MAX(250);

FifaStrategy::FifaStrategy(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , ProcessNackTraits(this)
  , m_retxSuppression(RETX_SUPPRESSION_INITIAL,
                      RetxSuppressionExponential::DEFAULT_MULTIPLIER,
                      RETX_SUPPRESSION_MAX)
{

  /*
  ***********************************
  * Schedule periodic event of FIFA *
  * ********************************* 
  */

  ns3::Time maxSimTime = ns3::Time::FromInteger(ns3::GlobalVariable::getSimulationEnd(), ns3::Time::Unit::S);
  ns3::Time interval(ns3::GlobalVariable::getPrimaryTimer());
  ns3::Time interval2(ns3::GlobalVariable::getSecondaryTimer());

  
  // set primary timer
  for(ns3::Time start = interval ; start < maxSimTime ; start = start + interval){
    ns3::Simulator::Schedule(start, &FifaStrategy::primaryMethod, this);
  }

  // set secondary timer
  for(ns3::Time start = interval2 ; start < maxSimTime ; start += interval2){
    ns3::Simulator::Schedule(start, &FifaStrategy::secondaryMethod, this);
  }


  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    m_VID = parsed.parameters.toUri().substr(1);
    //std::cout << m_VID << std::endl;
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    NDN_THROW(std::invalid_argument(
      "FifaStrategy does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

const Name&
FifaStrategy::getStrategyName()
{
  static Name strategyName("/localhost/nfd/strategy/FifaStrategy/%FD%05/");
  return strategyName;
}

void
FifaStrategy::afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
                                         const shared_ptr<pit::Entry>& pitEntry)
{
  /*
  // Get Application Parameter
  const uint8_t *ptr = interest.getApplicationParameters().value();
  std::string myStr((char *)ptr);
  std::cout << "APPLICATION PARAMETERS : " << myStr << std::endl;
  */

  // Get vehicleID from name
  // 2nd last element in name fromat is vehicleID
  string vehicleID = interest.getName().at(-2).toUri();


  // Check whether decrypted value matches vehicleID with 
  // one in certificate
  
  // vehicleID is in malicious then drop interest
  if (m_maliciousTable.contains(vehicleID)){
    //std::cout << "Interest From Malicious Interest: Dropping it!!" << std::endl;
    this->rejectPendingInterest(pitEntry);
    return;
  }


  RetxSuppressionResult suppression = m_retxSuppression.decidePerPitEntry(*pitEntry);
  if (suppression == RetxSuppressionResult::SUPPRESS) {
    NFD_LOG_DEBUG(interest << " from=" << ingress << " suppressed");
    return;
  }

  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();
  
  /*
    Current implmentation of ndnSIM do not forward
    interest/data to same face receiving face
    but we need Ad-Hoc face so we will skip check
  */
  
  auto it = nexthops.begin(); // auto it = netxhopes.end();

  if (suppression == RetxSuppressionResult::NEW) {

    // DISABLE CHECK TO MAKE Ad-Hoc

    // forward to nexthop with lowest cost except downstream
    //it = std::find_if(nexthops.begin(), nexthops.end(), [&] (const auto& nexthop) {
    //  return isNextHopEligible(ingress.face, interest, nexthop, pitEntry);
    //});

    if (it == nexthops.end()) {
      NFD_LOG_DEBUG(interest << " from=" << ingress << " noNextHop");

      lp::NackHeader nackHeader;
      nackHeader.setReason(lp::NackReason::NO_ROUTE);
      this->sendNack(pitEntry, ingress, nackHeader);
      if(m_VID == "v1") std::cout<<"NO NODE" << std::endl;
      this->rejectPendingInterest(pitEntry);
      return;
    }

    auto egress = FaceEndpoint(it->getFace(), 0);
    NFD_LOG_DEBUG(interest << " from=" << ingress << " newPitEntry-to=" << egress);
    
    /*
    ***********************************
    *       Update RecordTable        *
    *********************************** 
    */
    if(vehicleID != m_VID){
      if(!m_recordTable.hasRecord(vehicleID)) m_recordTable.addRecord(vehicleID);
      m_recordTable.incrementNoOfReceivedInterest(vehicleID);
      m_recordTable.updateSatisfactionRatio(vehicleID);
    }
    //
    this->sendInterest(pitEntry, egress, interest);
    return;
  }

  // find an unused upstream with lowest cost except downstream
  it = std::find_if(nexthops.begin(), nexthops.end(), [&] (const auto& nexthop) {
    return isNextHopEligible(ingress.face, interest, nexthop, pitEntry, true, time::steady_clock::now());
  });

  if (it != nexthops.end()) {
    auto egress = FaceEndpoint(it->getFace(), 0);

    /*
    ***********************************
    *       Update RecordTable        *
    *********************************** 
    */
    if(vehicleID != m_VID){
      if(!m_recordTable.hasRecord(vehicleID)) m_recordTable.addRecord(vehicleID);
      m_recordTable.incrementNoOfReceivedInterest(vehicleID);
      m_recordTable.updateSatisfactionRatio(vehicleID);
    }
    //
    this->sendInterest(pitEntry, egress, interest);
    NFD_LOG_DEBUG(interest << " from=" << ingress << " retransmit-unused-to=" << egress);
    return;
  }

  // find an eligible upstream that is used earliest
  it = findEligibleNextHopWithEarliestOutRecord(ingress.face, interest, nexthops, pitEntry);
  if (it == nexthops.end()) {
    NFD_LOG_DEBUG(interest << " from=" << ingress << " retransmitNoNextHop");
  }
  else {
    auto egress = FaceEndpoint(it->getFace(), 0);

    /*
    ***********************************
    *       Update RecordTable        *
    *********************************** 
    */
    if(vehicleID != m_VID){
      if(!m_recordTable.hasRecord(vehicleID)) m_recordTable.addRecord(vehicleID);
      m_recordTable.incrementNoOfReceivedInterest(vehicleID);
      m_recordTable.updateSatisfactionRatio(vehicleID);
    }
    //
    this->sendInterest(pitEntry, egress, interest);
    NFD_LOG_DEBUG(interest << " from=" << ingress << " retransmit-retry-to=" << egress);
  }
}

void
FifaStrategy::afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
                                     const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(ingress.face, nack, pitEntry);
}

void 
FifaStrategy::beforeSatisfyInterest(const shared_ptr<pit::Entry>& pitEntry, const FaceEndpoint& ingress, const Data& data) {
     
     // find vehicleID from name
     string vehicleID = data.getName().at(-2).toUri();

     if(vehicleID == m_VID) return;

     NFD_LOG_DEBUG(m_VID<< "\t"<<data << "from =" << ingress);

     // update record table info
     m_recordTable.incrementNoOfSatisfiedInterest(vehicleID);
     m_recordTable.updateSatisfactionRatio(vehicleID);
}

void FifaStrategy::primaryMethod(){

  // std::cout << ns3::Simulator::Now().As(ns3::Time::Unit::S) << "\t" << "inside primary method" << endl;

  uint64_t totalInterestCnt = m_recordTable.getTotalInterestCnt();

  uint64_t int_th = (uint64_t)(totalInterestCnt * ns3::GlobalVariable::getInterestCntTh());

  double_t sr_th = ns3::GlobalVariable::getSatisfactionRatioTh();

  vector<string> keys = m_recordTable.keySet();

  if(keys.size() == 0) return;

  //std::cout << "IN Primary of " << m_VID << endl;
  //m_recordTable.printTable();
  //std::cout << "End RecordTable" << std::endl;

  for(string k : keys){

    if(m_maliciousTable.contains(k)) continue;

    uint64_t curIntCnt = m_recordTable.getNoOfReceivedInerest(k);
    double_t curSR = m_recordTable.getSatisfactionRatio(k);

    if(curIntCnt >= int_th && curSR <= sr_th){
      // identify vehicle as malicious
      m_maliciousTable.adddVehicle(k);
      std::cout<< "vid: " << k 
               <<" curSR: " << curSR
               << " curIntCnt: " << curIntCnt
               << " int_th: " << int_th
               << " sr_th: " << sr_th
               << " total_intrest: " << totalInterestCnt <<endl;
      std::cout<<"Identified Malicious Vehicle (Primary Method of "<<m_VID<<" ):" << k << endl;
    } 
  }
}

void FifaStrategy::secondaryMethod(){

  //std::cout << "IN SECONDARY" << std::endl;

  vector<string> keys = m_recordTable.keySet();

  if(keys.size() == 0) return;

  double_t minSR = std::numeric_limits<double>::max();
  string vid;

  for(string k : keys){
    if(m_recordTable.getSatisfactionRatio(k) < minSR){
      minSR = m_recordTable.getSatisfactionRatio(k);
      vid = k;
    }
  }

  if(m_maliciousTable.contains(vid)) return;

  if(map.find(vid) == map.end()){
    map.insert({vid,0});
  }
  std::unordered_map<string, int>::iterator it = map.find(vid);
  if(m_recordTable.getSatisfactionRatio(vid) <= ns3::GlobalVariable::getSatisfactionRatioTh())
    (it->second)++;

  if(it->second == 3){
    // add it into malicious vehicle list
    m_maliciousTable.adddVehicle(vid);
    std::cout<<"Identified Malicious Vehicle (Secondary Method of " << m_VID <<" ):" << vid << endl;
  }
}


} // namespace fw
} // namespace nfd
