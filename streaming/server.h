/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SERVER_H
#define SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"

#include <vector>
#include <queue>
#include <set>

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup udpecho UdpEcho
 */

/**
 * \ingroup udpecho
 * \brief A Udp Echo server
 *
 * Every packet received is sent back.
 */
class StreamingStreamer : public Application 
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  StreamingStreamer ();
  virtual ~StreamingStreamer ();


	void SetDataSize (uint32_t dataSize);
	uint32_t GetDataSize (void) const;

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

	void ScheduleTransmit (Time dt);
	void Send (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

	uint32_t m_count;
	Time m_interval;
	uint32_t m_size;

	uint32_t seqNumber;
	bool isPause;

	uint32_t m_dataSize;
	uint8_t *m_data;

  uint16_t m_port; //!< Port on which we listen for incoming packets.
  Ptr<Socket> m_socket; //!< IPv4 Socket
	Ptr<Socket> m_socketRecv;
  Ptr<Socket> m_socket6; //!< IPv6 Socket
  Address m_local; //!< local multicast address

	Address m_peerAddress;
	uint16_t m_peerPort;
	EventId m_sendEvent;

    std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t> > ackBuffer;
    std::set<uint32_t> lossPackets;
    void FindLossPackets(void);
    void ScheduleFind(Time dt);
    EventId m_findEvent;
    uint32_t curSeq;
    uint32_t packetsPerFrame;

	TracedCallback<Ptr<const Packet> > m_txTrace;
  /// Callbacks for tracing the packet Rx events
  TracedCallback<Ptr<const Packet> > m_rxTrace;

	TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_txTraceWithAddresses;
  /// Callbacks for tracing the packet Rx events, includes source and destination addresses
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* SERVER_H */

