#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/address-utils.h"
#include "ns3/udp-socket.h"
#include "ns3/seq-ts-header.h"
#include "client.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("StreamingClientApplication");

    NS_OBJECT_ENSURE_REGISTERED(StreamingClient);

    TypeId StreamingClient::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::StreamingClient")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<StreamingClient>()
            .AddAttribute("RemoteAddress", "The destination Address of the outbound packets",
                AddressValue(),
                MakeAddressAccessor(&StreamingClient::m_peerAddress),
                MakeAddressChecker())
            .AddAttribute("RemotePort", "The destination port of the outbound packets",
                UintegerValue(0),
                MakeUintegerAccessor(&StreamingClient::m_peerPort),
                MakeUintegerChecker<uint16_t>())
            .AddAttribute("Port", "Port to receive packets",
                UintegerValue(0),
                MakeUintegerAccessor(&StreamingClient::m_port),
                MakeUintegerChecker<uint16_t>())
            .AddAttribute("LossRate", "Loss rate of packets received from the server",
                DoubleValue(0.),
                MakeDoubleAccessor(&StreamingClient::m_lossRate),
                MakeDoubleChecker<double>())
            .AddAttribute("PacketSize", "Packet size",
                UintegerValue(100),
                MakeUintegerAccessor(&StreamingClient::m_size),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("PacketsPerFrame", "Number of packets per frame",
                UintegerValue(100),
                MakeUintegerAccessor(&StreamingClient::m_packetsPerFrame),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("BufferingSize", "Buffer size for buffering",
                UintegerValue(25),
                MakeUintegerAccessor(&StreamingClient::m_bufferingSize),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("PauseSize", "Buffer size for sending pause request",
                UintegerValue(30),
                MakeUintegerAccessor(&StreamingClient::m_pauseSize),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("ResumeSize", "Buffer size for sending resume request",
                UintegerValue(25),
                MakeUintegerAccessor(&StreamingClient::m_resumeSize),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("RequestInterval", "Interval to request streaming",
                TimeValue(Seconds(1. / 10.)),
                MakeTimeAccessor(&StreamingClient::m_intervalRequest),
                MakeTimeChecker())
            .AddAttribute("GeneratorInterval", "Interval to generate frames",
                TimeValue(Seconds(1. / 20.)),
                MakeTimeAccessor(&StreamingClient::m_intervalGenerator),
                MakeTimeChecker())
            .AddAttribute("ConsumerInterval", "Interval to consume a frame",
                TimeValue(Seconds(1. / 60.)),
                MakeTimeAccessor(&StreamingClient::m_intervalConsumer),
                MakeTimeChecker())
            .AddTraceSource("Rx", "A packet has been received",
                MakeTraceSourceAccessor(&StreamingClient::m_rxTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses", "A packet has been received",
                MakeTraceSourceAccessor(&StreamingClient::m_rxTraceWithAddresses),
                "ns3::Packet::TwoAddressTracedCallback")
            ;
        return tid;
    }

    StreamingClient::StreamingClient()
    {
        NS_LOG_FUNCTION(this);

        m_socket = 0;
        m_socketRecv = 0;
        m_requestEvent = EventId();
        m_generatorEvent = EventId();
        m_consumerEvent = EventId();
        m_frameIdx = 0;
    }

    StreamingClient::~StreamingClient()
    {
        NS_LOG_FUNCTION(this);

        m_socket = 0;
        m_socketRecv = 0;
    }

    void StreamingClient::DoDispose(void)
    {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    void StreamingClient::StartApplication(void)
    {
        NS_LOG_FUNCTION(this);

        if (m_socket == 0)
        {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
            {
                if (m_socket->Bind() == -1)
                {
                    NS_FATAL_ERROR("Failed to bind socket");
                }
                m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
            }
            else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true)
            {
                if (m_socket->Bind() == -1)
                {
                    NS_FATAL_ERROR("Failed to bind socket");
                }
                m_socket->Connect(m_peerAddress);
            }
            else
            {
                NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
            }
        }

        if (m_socketRecv == 0)
        {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socketRecv = Socket::CreateSocket(GetNode(), tid);
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            if (m_socketRecv->Bind(local) == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            if (addressUtils::IsMulticast(m_local))
            {
                Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socketRecv);
                if (udpSocket)
                {
                    udpSocket->MulticastJoinGroup(0, m_local);
                }
                else
                {
                    NS_FATAL_ERROR("Error: Failed to join multicast group");
                }
            }
        }

        m_socket->SetAllowBroadcast(true);
        m_socketRecv->SetRecvCallback(MakeCallback(&StreamingClient::HandleRead, this));
        ScheduleRequest(Seconds(0.));
    }

    void StreamingClient::StopApplication()
    {
        NS_LOG_FUNCTION(this);

        if (m_socket != 0)
        {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
            m_socket = 0;
        }
        if (m_socketRecv != 0)
        {
            m_socketRecv->Close();
            m_socketRecv->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
            m_socketRecv = 0;
        }

        Simulator::Cancel(m_requestEvent);
        Simulator::Cancel(m_generatorEvent);
        Simulator::Cancel(m_consumerEvent);
    }

    void StreamingClient::ScheduleRequest(Time dt)
    {
        NS_LOG_FUNCTION(this << dt);
        m_requestEvent = Simulator::Schedule(dt, &StreamingClient::RequestStreaming, this);
    }

    void StreamingClient::ScheduleGenerator(Time dt)
    {
        NS_LOG_FUNCTION(this << dt);
        m_generatorEvent = Simulator::Schedule(dt, &StreamingClient::GenerateFrame, this);
    }

    void StreamingClient::ScheduleConsumer(Time dt)
    {
        NS_LOG_FUNCTION(this << dt);
        m_consumerEvent = Simulator::Schedule(dt, &StreamingClient::ConsumeFrame, this);
    }

    void StreamingClient::RequestStreaming(void)
    {
        NS_LOG_FUNCTION(this);

        NS_ASSERT(m_requestEvent.IsExpired());

        if (m_packets.empty())
        {
            Ptr<Packet> p = Create<Packet>(m_size);
            SeqTsHeader header;
            header.SetSeq(0);
            p->AddHeader(header);
            header.SetSeq(0);
            p->AddHeader(header);
            header.SetSeq(m_port);
            p->AddHeader(header);
            m_socket->Send(p);

            ScheduleRequest(m_intervalRequest);
        }
        else
        {
            ScheduleGenerator(Seconds(0.));
            ScheduleConsumer(Seconds(0.));
        }
    }

    void StreamingClient::GenerateFrame(void)
    {
        NS_LOG_FUNCTION(this);

        NS_ASSERT(m_generatorEvent.IsExpired());

        uint32_t maxFrame = m_packets.size() / m_packetsPerFrame;

        for (uint32_t i = m_frameIdx; i <= maxFrame; i++)
        {
            if (!m_frames.count(i))
            {
                for (uint32_t j = 0; j < m_packetsPerFrame; j++)
                {
                    if (!m_packets.count(i * m_packetsPerFrame + j))
                    {
                        break;
                    }
                    if (j == m_packetsPerFrame - 1)
                    {
                        if (m_frameBuffer.size() < 40)
                        {
                            m_frames.insert(i);
                            m_frameBuffer.push(i);
                            NS_LOG_INFO(i);
                        }
                    }
                }
            }
        }
        ScheduleGenerator(m_intervalGenerator);
    }

    void StreamingClient::ConsumeFrame(void)
    {
        NS_LOG_FUNCTION(this);

        NS_ASSERT(m_consumerEvent.IsExpired());

        while (!m_frameBuffer.empty() && m_frameBuffer.top() < m_frameIdx)
        {
            m_frameBuffer.pop();
        }
        if (m_frameBuffer.size() < m_bufferingSize)
        {
            //NS_LOG_INFO("Buffering: " << m_frameIdx << "\tRemain: " << m_frameBuffer.size());

            Ptr<Packet> p = Create<Packet>(m_size);
            SeqTsHeader header;
            header.SetSeq(0);
            p->AddHeader(header);
            header.SetSeq(2);
            p->AddHeader(header);
            header.SetSeq(m_port);
            p->AddHeader(header);
            m_socket->Send(p);
        }
        else
        {
            if (m_frameIdx == m_frameBuffer.top())
            {
                m_frameBuffer.pop();
                //NS_LOG_INFO("Consume: " << m_frameIdx << "\tRemain: " << m_frameBuffer.size());
            }
            else
            {
                //NS_LOG_INFO("NoConsume: " << m_frameIdx << "\tRemain: " << m_frameBuffer.size());
            }
            if (m_frameBuffer.size() > m_pauseSize)
            {
                Ptr<Packet> p = Create<Packet>(m_size);
                SeqTsHeader header;
                header.SetSeq(0);
                p->AddHeader(header);
                header.SetSeq(1);
                p->AddHeader(header);
                header.SetSeq(m_port);
                p->AddHeader(header);
                m_socket->Send(p);
            }
            else if (m_frameBuffer.size() < m_resumeSize)
            {
                Ptr<Packet> p = Create<Packet>(m_size);
                SeqTsHeader header;
                header.SetSeq(0);
                p->AddHeader(header);
                header.SetSeq(2);
                p->AddHeader(header);
                header.SetSeq(m_port);
                p->AddHeader(header);
                m_socket->Send(p);
            }
            m_frameIdx++;
        }
        ScheduleConsumer(m_intervalConsumer);
    }

    void StreamingClient::HandleRead(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this << socket);

        if ((std::rand() % 100) >= (m_lossRate * 100))
        {
            Ptr<Packet> packet;
            Address from;
            Address localAddress;

            while ((packet = socket->RecvFrom(from)))
            {
                socket->GetSockName(localAddress);
                m_rxTrace(packet);
                m_rxTraceWithAddresses(packet, from, localAddress);

                SeqTsHeader seqTs;
                packet->RemoveHeader(seqTs);
                m_packets.insert(seqTs.GetSeq());
                //printf("CL : %d\n", seqTs.GetSeq());

                Ptr<Packet> p = Create<Packet>(m_size);
                SeqTsHeader header;
                header.SetSeq(seqTs.GetSeq());
                p->AddHeader(header);
                header.SetSeq(3);
                p->AddHeader(header);
                header.SetSeq(m_port);
                p->AddHeader(header);
                m_socket->Send(p);

                printf("CLIENT SEND\t%d\t%d\n", 3, seqTs.GetSeq());
            }
        }
    }
}