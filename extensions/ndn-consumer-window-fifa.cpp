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

#include "ndn-consumer-window-fifa.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include <limits>

NS_LOG_COMPONENT_DEFINE("ndn.ConsumerWindowFifa");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerWindowFifa);

TypeId
ConsumerWindowFifa::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerWindowFifa")
      .SetGroupName("Ndn")
      .SetParent<ConsumerFifa>()
      .AddConstructor<ConsumerWindowFifa>()

      .AddAttribute("Window", "Initial size of the window", StringValue("1"),
                    MakeUintegerAccessor(&ConsumerWindowFifa::GetWindow, &ConsumerWindowFifa::SetWindow),
                    MakeUintegerChecker<uint32_t>())

      .AddAttribute("PayloadSize",
                    "Average size of content object size (to calculate interest generation rate)",
                    UintegerValue(1040), MakeUintegerAccessor(&ConsumerWindowFifa::GetPayloadSize,
                                                              &ConsumerWindowFifa::SetPayloadSize),
                    MakeUintegerChecker<uint32_t>())

      .AddAttribute("Size", "Amount of data in megabytes to request, relying on PayloadSize "
                            "parameter (alternative to MaxSeq attribute)",
                    DoubleValue(-1), // don't impose limit by default
                    MakeDoubleAccessor(&ConsumerWindowFifa::GetMaxSize, &ConsumerWindowFifa::SetMaxSize),
                    MakeDoubleChecker<double>())

      .AddAttribute("MaxSeq", "Maximum sequence number to request (alternative to Size attribute, "
                              "would activate only if Size is -1). "
                              "The parameter is activated only if Size negative (not set)",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeUintegerAccessor(&ConsumerWindowFifa::GetSeqMax, &ConsumerWindowFifa::SetSeqMax),
                    MakeUintegerChecker<uint32_t>())

      .AddAttribute("InitialWindowOnTimeout", "Set window to initial value when timeout occurs",
                    BooleanValue(true),
                    MakeBooleanAccessor(&ConsumerWindowFifa::m_setInitialWindowOnTimeout),
                    MakeBooleanChecker())

      .AddTraceSource("WindowTrace",
                      "Window that controls how many outstanding interests are allowed",
                      MakeTraceSourceAccessor(&ConsumerWindowFifa::m_window),
                      "ns3::ndn::ConsumerWindowFifa::WindowTraceCallback")
      .AddTraceSource("InFlight", "Current number of outstanding interests",
                      MakeTraceSourceAccessor(&ConsumerWindowFifa::m_inFlight),
                      "ns3::ndn::ConsumerWindowFifa::WindowTraceCallback");

  return tid;
}

ConsumerWindowFifa::ConsumerWindowFifa()
  : m_payloadSize(1040)
  , m_inFlight(0)
{
}

void
ConsumerWindowFifa::SetWindow(uint32_t window)
{
  m_initialWindow = window;
  m_window = m_initialWindow;
}

uint32_t
ConsumerWindowFifa::GetWindow() const
{
  return m_initialWindow;
}

uint32_t
ConsumerWindowFifa::GetPayloadSize() const
{
  return m_payloadSize;
}

void
ConsumerWindowFifa::SetPayloadSize(uint32_t payload)
{
  m_payloadSize = payload;
}

double
ConsumerWindowFifa::GetMaxSize() const
{
  if (m_seqMax == 0)
    return -1.0;

  return m_maxSize;
}

void
ConsumerWindowFifa::SetMaxSize(double size)
{
  m_maxSize = size;
  if (m_maxSize < 0) {
    m_seqMax = std::numeric_limits<uint32_t>::max();
    return;
  }

  m_seqMax = floor(1.0 + m_maxSize * 1024.0 * 1024.0 / m_payloadSize);
  NS_LOG_DEBUG("MaxSeqNo: " << m_seqMax);
  // std::cout << "MaxSeqNo: " << m_seqMax << "\n";
}

uint32_t
ConsumerWindowFifa::GetSeqMax() const
{
  return m_seqMax;
}

void
ConsumerWindowFifa::SetSeqMax(uint32_t seqMax)
{
  if (m_maxSize < 0)
    m_seqMax = seqMax;

  // ignore otherwise
}

void
ConsumerWindowFifa::ScheduleNextPacket()
{
  if (m_window == static_cast<uint32_t>(0)) {
    Simulator::Remove(m_sendEvent);

    NS_LOG_DEBUG(
      "Next event in " << (std::min<double>(0.5, m_rtt->RetransmitTimeout().ToDouble(Time::S)))
                       << " sec");
    m_sendEvent =
      Simulator::Schedule(Seconds(
                            std::min<double>(0.5, m_rtt->RetransmitTimeout().ToDouble(Time::S))),
                          &ConsumerFifa::SendPacket, this);
  }
  else if (m_inFlight >= m_window) {
    // simply do nothing
  }
  else {
    if (m_sendEvent.IsRunning()) {
      Simulator::Remove(m_sendEvent);
    }

    m_sendEvent = Simulator::ScheduleNow(&ConsumerFifa::SendPacket, this);
  }
}

///////////////////////////////////////////////////
//          Process incoming packets             //
///////////////////////////////////////////////////

void
ConsumerWindowFifa::OnData(shared_ptr<const Data> contentObject)
{
  ConsumerFifa::OnData(contentObject);

  m_window = m_window + 1;

  if (m_inFlight > static_cast<uint32_t>(0))
    m_inFlight--;
  NS_LOG_DEBUG("Window: " << m_window << ", InFlight: " << m_inFlight);

  ScheduleNextPacket();
}

void
ConsumerWindowFifa::OnTimeout(uint32_t sequenceNumber)
{
  if (m_inFlight > static_cast<uint32_t>(0))
    m_inFlight--;

  if (m_setInitialWindowOnTimeout) {
    // m_window = std::max<uint32_t> (0, m_window - 1);
    m_window = m_initialWindow;
  }

  NS_LOG_DEBUG("Window: " << m_window << ", InFlight: " << m_inFlight);
  ConsumerFifa::OnTimeout(sequenceNumber);
}

void
ConsumerWindowFifa::WillSendOutInterest(uint32_t sequenceNumber)
{
  m_inFlight++;
  ConsumerFifa::WillSendOutInterest(sequenceNumber);
}

} // namespace ndn
} // namespace ns3
