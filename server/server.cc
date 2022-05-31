#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"
#include "server.h"
#include "load-balancer-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StreamingServerApplication");

NS_OBJECT_ENSURE_REGISTERED (StreamingStreamer);

// setting packet
TypeId
StreamingStreamer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::StreamingStreamer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<StreamingStreamer> ()
		.AddAttribute ("Interval", "The time to wait between packets",
									TimeValue (Seconds (1.0)),
									MakeTimeAccessor (&StreamingStreamer::m_interval),
									MakeTimeChecker ())
		.AddAttribute ("PacketSize", "Size of data in outbound packets",
									UintegerValue(100),
									MakeUintegerAccessor (&StreamingStreamer::SetDataSize,
																				&StreamingStreamer::GetDataSize),
									MakeUintegerChecker<uint32_t> ())
		.AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&StreamingStreamer::m_port),
                   MakeUintegerChecker<uint16_t> ())
	  .AddAttribute("PacketsPerFrame", "Number of packets per frame",
		  UintegerValue(100),
		  MakeUintegerAccessor(&StreamingStreamer::m_packetsPerFrame),
		  MakeUintegerChecker<uint32_t>())
	  .AddTraceSource("Tx", "A new packet is created and is sent",
		  MakeTraceSourceAccessor(&StreamingStreamer::m_txTrace),
		  "ns3::Packet::AddressTracedCallback")
	  .AddTraceSource("Rx", "A packet has been received",
		  MakeTraceSourceAccessor(&StreamingStreamer::m_rxTrace),
		  "ns3::Packet::AddressTracedCallback")
	  .AddTraceSource("Rtx", "A retransmission packet is created and is sent",
		  MakeTraceSourceAccessor(&StreamingStreamer::m_rtxTrace),
		  "ns3::Packet::AddressTracedCallback")
	  .AddTraceSource("TxWithAddresses", "A new packet is created and is sent",
		  MakeTraceSourceAccessor(&StreamingStreamer::m_txTraceWithAddresses),
		  "ns3::Packet::TwoAddressTracedCallback")
	  .AddTraceSource("RxWithAddresses", "A packet has been received",
		  MakeTraceSourceAccessor(&StreamingStreamer::m_rxTraceWithAddresses),
		  "ns3::Packet::TwoAddressTracedCallback")
	  .AddTraceSource("RtxWithAddresses", "A retransmission packet is created and is sent",
		  MakeTraceSourceAccessor(&StreamingStreamer::m_rtxTraceWithAddresses),
		  "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}
// constructor
StreamingStreamer::StreamingStreamer ()
{
  NS_LOG_FUNCTION (this);

	m_socket = 0;
	m_sendEvent = EventId ();
	m_data = 0;
	m_dataSize = 0;
	m_curSeq = 0;
	m_seqNumber = 0;
	m_isPause = false;
}
// destroyer
StreamingStreamer::~StreamingStreamer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;

	delete [] m_data;
	m_data = 0;
	m_dataSize = 0;
}

void
StreamingStreamer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
StreamingStreamer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

	if (m_socketRecv == 0)
		{
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			m_socketRecv = Socket::CreateSocket (GetNode (), tid);

			InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
			if (m_socketRecv->Bind (local) == -1)
			{
			NS_FATAL_ERROR ("Failed to bind socket");
			}
		if (addressUtils::IsMulticast (m_local))
			{
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socketRecv);
			if (udpSocket)
			{
				udpSocket->MulticastJoinGroup (0, m_local);
			}
		else
			{
			NS_FATAL_ERROR ("Error: Failed to join multicast group");
			}
		}
	}
    // packet information check
  	m_socketRecv->SetRecvCallback(MakeCallback (&StreamingStreamer::HandleRead, this)); 
}

void
StreamingStreamer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
			m_socket = 0;
    }
	if (m_socketRecv != 0)
		{
			m_socketRecv->Close ();
			m_socketRecv->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
			m_socketRecv = 0;
		}
  if (m_socket6 != 0)
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }

	Simulator::Cancel (m_sendEvent);
}

void
StreamingStreamer::SetDataSize (uint32_t dataSize)
{
	NS_LOG_FUNCTION (this << dataSize);

	delete [] m_data;
	m_data = 0;
	m_dataSize = 0;
	m_size = dataSize;
}

uint32_t
StreamingStreamer::GetDataSize (void) const
{
	NS_LOG_FUNCTION (this);
	return m_size;
}

void
StreamingStreamer::ScheduleTransmit (Time dt)
{
	NS_LOG_FUNCTION (this << dt);
	m_sendEvent = Simulator::Schedule (dt, &StreamingStreamer::Send, this);
}

void
StreamingStreamer::ScheduleFind(Time dt)
{
	NS_LOG_FUNCTION(this << dt);
	m_findEvent = Simulator::Schedule(dt, &StreamingStreamer::FindLossPackets, this);
}

// sending streaming packet
void
StreamingStreamer::Send (void)
{
	NS_LOG_FUNCTION (this);
	NS_ASSERT (m_sendEvent.IsExpired ());
	uint32_t cnt = 0;

	if (!m_isPause)
	{
		if (!m_lossPackets.empty())
		{
			for (auto i = m_lossPackets.begin(); i != m_lossPackets.end(); i++)
			{
				Ptr<Packet> p = Create<Packet>(m_size);

				Address localAddress;
				m_socket->GetSockName(localAddress);

				SeqTsHeader seqTs;
				seqTs.SetSeq(*i);
				p->AddHeader(seqTs);

				m_rtxTrace(p, localAddress);
				m_rtxTraceWithAddresses(p, localAddress, InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));

				m_socket->Send(p);
				cnt++;
			}
		}
		if (m_packetsPerFrame > cnt)
		{
			for (uint32_t i = 0; i < m_packetsPerFrame - cnt; i++)
			{
				Ptr<Packet> p = Create<Packet>(m_size);

				Address localAddress;
				m_socket->GetSockName(localAddress);

				SeqTsHeader seqTs;
				seqTs.SetSeq(m_seqNumber++);
				p->AddHeader(seqTs);

				m_txTrace(p, localAddress);
				m_txTraceWithAddresses(p, localAddress, InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));

				m_socket->Send(p);
			}
		}
	}
	ScheduleTransmit(m_interval);
}

void
StreamingStreamer::FindLossPackets(void)
{
	NS_LOG_FUNCTION(this);
	NS_ASSERT(m_findEvent.IsExpired());

	if (!m_ackBuffer.empty() && m_ackBuffer.size() > m_packetsPerFrame)
	{
		for (uint32_t i = 0; i < m_packetsPerFrame; i++)
		{
			if (m_ackBuffer.top() < m_curSeq)
			{
				m_ackBuffer.pop();
			}
			else if (m_ackBuffer.top() == m_curSeq)
			{
				m_ackBuffer.pop();
				m_curSeq++;
			}
			else
			{
				m_lossPackets.insert(m_curSeq);
				m_curSeq++;
			}
		}
	}
	ScheduleFind(m_interval);
}

// check packet header
void
StreamingStreamer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
		socket->GetSockName(localAddress);
		m_rxTrace(packet, from);
		m_rxTraceWithAddresses(packet, from, localAddress);

		LoadBalancerHeader header;
		packet->RemoveHeader(header);
		uint32_t clientAddress = header.GetIpv4Address();
		uint16_t clientPort = header.GetPort();

		SeqTsHeader requestType;
		packet->RemoveHeader(requestType);
		uint32_t type = requestType.GetSeq();
		
		if (type == 3)
		{
			SeqTsHeader seqTs;
			packet->RemoveHeader(seqTs);
			m_ackBuffer.push(seqTs.GetSeq());
			if (m_lossPackets.count(seqTs.GetSeq()))
			{
				m_lossPackets.erase(m_lossPackets.find(seqTs.GetSeq()));
			}
		}
		else if (type == 1)
		{
			m_isPause = true;
		}
		else if (type == 0)
		{
			m_isPause = false;
		}
		else if (type == 2)
		{
			if (m_peerAddress != Ipv4Address(clientAddress))
			{
				m_peerAddress = Ipv4Address(clientAddress);
				m_peerPort = clientPort;
				m_isPause = false;
				NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "] SERVER\trecv streaming request");
				if (m_socket == 0)
				{
					TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
					m_socket = Socket::CreateSocket(GetNode(), tid);
					if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
					{
						if (m_socket->Bind() == -1)
							NS_FATAL_ERROR("Failed to bind socket");
						m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
					}
					else if (Ipv6Address::IsMatchingType(m_peerAddress) == true) {
						if (m_socket->Bind6() == -1)
							NS_FATAL_ERROR("Failed to bind socket");
						m_socket->Connect(Inet6SocketAddress(Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
					}
					else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true) {
						if (m_socket->Bind() == -1)
							NS_FATAL_ERROR("Failed to bind socket");
						m_socket->Connect(m_peerAddress);
					}
					else if (Inet6SocketAddress::IsMatchingType(m_peerAddress) == true) {
						if (m_socket->Bind6() == -1)
							NS_FATAL_ERROR("Failed to bind socket");
						m_socket->Connect(m_peerAddress);
					}
					else {
						NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
					}
				}
				m_socket->SetAllowBroadcast(true);
				ScheduleTransmit(Seconds(0.));
				ScheduleFind(Seconds(0.));
			}
		}
    }
}

} // Namespace ns3
