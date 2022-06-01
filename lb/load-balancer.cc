#include "load-balancer.h"
#include "load-balancer-header.h"

#include <iostream>

#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LoadBalancer");

NS_OBJECT_ENSURE_REGISTERED(LoadBalancer);

TypeId
LoadBalancer::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::LoadBalancer")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<LoadBalancer>()
            .AddAttribute("Port", "Port on which we listen for incoming packets",
                          UintegerValue(80),
                          MakeUintegerAccessor(&LoadBalancer::m_port),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("FirstAddress", "Server address for load balancing",
                          AddressValue(),
                          MakeAddressAccessor(&LoadBalancer::m_peerAddress0),
                          MakeAddressChecker())
            .AddAttribute("FirstWeight", "Server weight for load balancing",
                          UintegerValue(1),
                          MakeUintegerAccessor(&LoadBalancer::m_peerWeight0),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("SecondAddress", "Server address for load balancing",
                          AddressValue(),
                          MakeAddressAccessor(&LoadBalancer::m_peerAddress1),
                          MakeAddressChecker())
            .AddAttribute("SecondWeight", "Server weight for load balancing",
                          UintegerValue(1),
                          MakeUintegerAccessor(&LoadBalancer::m_peerWeight1),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("ThirdAddress", "Server address for load balancing",
                          AddressValue(),
                          MakeAddressAccessor(&LoadBalancer::m_peerAddress2),
                          MakeAddressChecker())
            .AddAttribute("ThirdWeight", "Server weight for load balancing",
                          UintegerValue(1),
                          MakeUintegerAccessor(&LoadBalancer::m_peerWeight2),
                          MakeUintegerChecker<uint16_t>())
            .AddTraceSource("Rx", "A packet has been received",
                            MakeTraceSourceAccessor(&LoadBalancer::m_rxTrace),
                            "ns3::Packet::AddressTracedCallback")
            .AddTraceSource("RxWithAddresses", "A packet has been received",
                            MakeTraceSourceAccessor(&LoadBalancer::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

LoadBalancer::LoadBalancer()
    : m_socket(0),
    m_peerCnt(0),
    m_maxWeight(0),
    m_curRound(0),
    m_curIndex(0)
{
    NS_LOG_FUNCTION(this);
}

LoadBalancer::~LoadBalancer()
{
    NS_LOG_FUNCTION(this);
    m_socket = 0;
}

void
LoadBalancer::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
LoadBalancer::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    if (m_socket == 0)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        if (addressUtils::IsMulticast(m_local))
        {
            Ptr <UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socket);
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

    m_socket->SetRecvCallback(MakeCallback(&LoadBalancer::HandleRead, this));

    if (m_peerAddress0 != Address())
    {
        m_states.push_back(std::make_pair(m_peerAddress0, m_peerWeight0 - 1));
        ++m_peerCnt;
        if (m_peerWeight0 > m_maxWeight)
        {
            m_maxWeight = m_peerWeight0;
        }
    }
    if (m_peerAddress1 != Address())
    {
        m_states.push_back(std::make_pair(m_peerAddress1, m_peerWeight1 - 1));
        ++m_peerCnt;
        if (m_peerWeight1 > m_maxWeight)
        {
            m_maxWeight = m_peerWeight1;
        }
    }
    if (m_peerAddress2 != Address())
    {
        m_states.push_back(std::make_pair(m_peerAddress2, m_peerWeight2 - 1));
        ++m_peerCnt;
        if (m_peerWeight2 > m_maxWeight)
        {
            m_maxWeight = m_peerWeight2;
        }
    }

    NS_LOG_DEBUG("### LB Server Attribute\t(peerCnt, maxWeight): " << "(" << m_peerCnt << ", " << m_maxWeight << ")");
}

void
LoadBalancer::StopApplication(void)
{
    NS_LOG_FUNCTION(this);

    if (m_socket != 0)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
LoadBalancer::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);

    Ptr<Packet> packet;
    Address from;
    Address localAddress;

    while ((packet = socket->RecvFrom(from)))
    {
        socket->GetSockName(localAddress);
        m_rxTrace(packet, from);
        m_rxTraceWithAddresses(packet, from, localAddress);

        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server received " << packet->GetSize() << " bytes from " <<
                            InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
        }

        packet->RemoveAllPacketTags();
        packet->RemoveAllByteTags();

        InetSocketAddress transport = InetSocketAddress::ConvertFrom(from);
        uint32_t fromIpv4Address = transport.GetIpv4().Get();
        uint16_t fromPort = transport.GetPort();

        LoadBalancerHeader header;
        header.SetIpv4Address(fromIpv4Address);
        header.SetPort(fromPort);
        packet->AddHeader(header);

        Address target = AssignTargetAddress(fromIpv4Address);
        socket->SendTo(packet, 0, target);

        if (InetSocketAddress::IsMatchingType(m_peerAddress0))
        {
            NS_LOG_INFO ("At time " << Simulator::Now().As(Time::S) << " server sent " << packet->GetSize() << " bytes to " <<
                            InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
        }
    }
}

Address
LoadBalancer::AssignTargetAddress(uint32_t from)
{
    uint16_t round;
    uint16_t index;
    Address target;

    auto session = m_sessions.find(from);
    if (session != m_sessions.end())
    {
        index = session->second;
        target = m_states.at(index).first;
        NS_LOG_DEBUG("### FIND\t" << from << " <=> " << target);
        return target;
    }

    for (uint16_t i = 0; i < m_maxWeight; i++)
    {
        round = (m_curRound + i) % m_maxWeight;
        for (uint16_t j = 0; j < m_peerCnt; j++)
        {
            index = (m_curIndex + i) % m_peerCnt;
            if (m_states.at(index).second >= round)
            {
                m_sessions.insert(std::make_pair(from, index));
                m_curRound = round + 1;
                m_curIndex = index + 1;
                target = m_states.at(index).first;
                NS_LOG_DEBUG("### Assign\t" << from << " <=> " << target);
                return target;
            }
        }
    }

    m_curRound = round + 1;
    m_curIndex = index + 1;
    return target;
}

} // ns3
