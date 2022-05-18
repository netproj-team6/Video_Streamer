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
#include "ns3/trace-source-accessor.h"
#include "client.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StreamingClientApplication");

NS_OBJECT_ENSURE_REGISTERED (StreamingClient);

TypeId
StreamingClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::StreamingClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<StreamingClient> ()
	.AddAttribute ("Port", "port",
        UintegerValue (9),
        MakeUintegerAccessor (&StreamingClient::m_port),
        MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MaxPackets",
        "The maximum number of packets the application will send",
        UintegerValue (100),
        MakeUintegerAccessor (&StreamingClient::m_count),
        MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("GeneratorInterval",
        "The time to wait between packets",
        TimeValue (Seconds (1.0)),
        MakeTimeAccessor (&StreamingClient::m_intervalGenerator),
        MakeTimeChecker ())
	.AddAttribute ("ConsumerInterval", "time",
        TimeValue (Seconds (1.0)),
        MakeTimeAccessor (&StreamingClient::m_intervalConsumer),
        MakeTimeChecker ())
    .AddAttribute ("RemoteAddress",
        "The destination Address of the outbound packets",
        AddressValue (),
        MakeAddressAccessor (&StreamingClient::m_peerAddress),
        MakeAddressChecker ())
    .AddAttribute ("RemotePort",
        "The destination port of the outbound packets",
        UintegerValue (0),
        MakeUintegerAccessor (&StreamingClient::m_peerPort),
        MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
        UintegerValue (100),
        MakeUintegerAccessor (&StreamingClient::SetDataSize, &StreamingClient::GetDataSize),
        MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("LossRate", "loss rate",
        DoubleValue(0),
        MakeDoubleAccessor (&StreamingClient::lossRate),
        MakeDoubleChecker<double> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
        MakeTraceSourceAccessor (&StreamingClient::m_txTrace),
        "ns3::Packet::TracedCallback")
    .AddTraceSource ("Rx", "A packet has been received",
        MakeTraceSourceAccessor (&StreamingClient::m_rxTrace),
        "ns3::Packet::TracedCallback")
    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
        MakeTraceSourceAccessor (&StreamingClient::m_txTraceWithAddresses),
        "ns3::Packet::TwoAddressTracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
        MakeTraceSourceAccessor (&StreamingClient::m_rxTraceWithAddresses),
        "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

StreamingClient::StreamingClient ()
{
    NS_LOG_FUNCTION (this);
    m_sent = 0;
    m_socket = 0;
	m_generatorEvent = EventId ();
	m_consumerEvent = EventId ();
    m_data = 0;
    m_dataSize = 0;
	frameIdx = 0;
	lossRate = 0.0;
    packetsPerFrame = 100;
}

StreamingClient::~StreamingClient()
{
    NS_LOG_FUNCTION (this);
    m_socket = 0;

    delete [] m_data;
    m_data = 0;
    m_dataSize = 0;
}

void 
StreamingClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }

	if (m_socketRecv == 0)
		{
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			m_socketRecv = Socket::CreateSocket (GetNode (), tid);
			InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
			if (m_socketRecv->Bind (local) == -1)
				NS_FATAL_ERROR ("Failed to bind socket");
			if (addressUtils::IsMulticast (m_local))
			{
				Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socketRecv);
				if (udpSocket) udpSocket->MulticastJoinGroup (0, m_local);
				else NS_FATAL_ERROR ("Error: Failed to join multicast group");
			}
		}


  m_socketRecv->SetRecvCallback (MakeCallback (&StreamingClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  ScheduleRequest(Seconds(0.));
  ScheduleGenerator (Seconds (0.));
  ScheduleConsumer (Seconds (0.));
}

void 
StreamingClient::StopApplication ()
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

  Simulator::Cancel (m_generatorEvent);
	Simulator::Cancel (m_consumerEvent);
}

void 
StreamingClient::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t 
StreamingClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void
StreamingClient::ScheduleRequest(Time dt)
{
    NS_LOG_FUNCTION(this << dt);
    m_generatorEvent = Simulator::Schedule(dt, &StreamingClient::RequestStreaming, this);
}

void
StreamingClient::ScheduleGenerator (Time dt)
{
	NS_LOG_FUNCTION (this << dt);
	m_generatorEvent = Simulator::Schedule (dt, &StreamingClient::GenerateFrame, this);
}

void
StreamingClient::ScheduleConsumer (Time dt)
{
	NS_LOG_FUNCTION (this << dt);
	m_consumerEvent = Simulator::Schedule (dt, &StreamingClient::ConsumeFrame, this);
}

void StreamingClient::RequestStreaming(void)
{
    NS_LOG_FUNCTION(this);

    Ptr<Packet> p = Create<Packet>(m_size);

    Address localAddress;
    m_socket->GetSockName(localAddress);

    SeqTsHeader seqTs;
    seqTs.SetSeq(0);
    p->AddHeader(seqTs);

    m_socket->Send(p);
}

void
StreamingClient::GenerateFrame (void)
{
	NS_LOG_FUNCTION (this);

	NS_ASSERT (m_generatorEvent.IsExpired ());

    packetsSize = packets.size();

    for (uint32_t i = frameIdx; i <= packetsSize / packetsPerFrame; i++)
    {
        if (frames.count(i))
        {
            continue;
        }
        for (uint32_t j = 0; j < packetsPerFrame; j++)
        {
            if (!packets.count(i * packetsPerFrame + j))
            {
                break;
            }
            if (j == packetsPerFrame - 1)
            {
                if (frameBuffer.size() < 40)
                {
                    frames.insert(i);
                    frameBuffer.push(i);
                }
            }
        }
    }

	ScheduleGenerator (m_intervalGenerator);
}

void
StreamingClient::ConsumeFrame (void)
{
	NS_LOG_FUNCTION (this);

	NS_ASSERT (m_consumerEvent.IsExpired ());

	while (!frameBuffer.empty() && frameBuffer.top() < frameIdx) {
		frameBuffer.pop();
	}

	if (!frameBuffer.empty()) {
		if (frameIdx == frameBuffer.top()) {
			frameBuffer.pop();
		} else {
		}
		NS_LOG_INFO(frameBuffer.size());
	} else {
		NS_LOG_INFO(0);
	}

	if (frameBuffer.size() > 30) {
		Ptr<Packet> p = Create<Packet> (m_size);
		SeqTsHeader pauseTs;
		pauseTs.SetSeq(1);
		p->AddHeader(pauseTs);
		m_socket->Send(p);
	} else if (frameBuffer.size() < 25) {
		Ptr<Packet> p = Create<Packet> (m_size);
		SeqTsHeader pauseTs;
		pauseTs.SetSeq(2);
		p->AddHeader(pauseTs);
		m_socket->Send(p);
	}

	frameIdx++;

	ScheduleConsumer (m_intervalConsumer);
}
	
void StreamingClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
			SeqTsHeader seqTs;
			packet->RemoveHeader(seqTs);
			uint32_t seq = seqTs.GetSeq();

			if ((std::rand() % 100) >= (lossRate * 100)) {
				packets.insert(seq);

			}

      socket->GetSockName (localAddress);
      m_rxTrace (packet);
      m_rxTraceWithAddresses (packet, from, localAddress);
    }
}
}
