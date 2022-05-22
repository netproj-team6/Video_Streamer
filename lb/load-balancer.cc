#include "load-balancer.h"

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
            .AddAttribute("FirstAddress", "First server address for load balancing",
                          AddressValue(),
                          MakeAddressAccessor(&LoadBalancer::m_peerAddress0),
                          MakeAddressChecker())
            .AddAttribute("FirstPort", "First server port for load balancing",
                          UintegerValue(80),
                          MakeUintegerAccessor(&LoadBalancer::m_peerPort0),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("FirstWeight", "First server weight for load balancing",
                          UintegerValue(1),
                          MakeUintegerAccessor(&LoadBalancer::m_peerWeight0),
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
    : m_socket(0)
{
    NS_LOG_FUNCTION(this);
}

LoadBalancer::~LoadBalancer()
{
    NS_LOG_FUNCTION(this);
    m_socket = 0;   // todo
}

// todo void AddRemote(address, port, weight)

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

    // todo set status map
}

void
LoadBalancer::StopApplication(void)
{
    NS_LOG_FUNCTION(this);

    if (m_socket != 0)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback <void, Ptr <Socket>> ());
    }
}

void
LoadBalancer::HandleRead(Ptr <Socket> socket)
{
    NS_LOG_FUNCTION(this);

    Ptr <Packet> packet;
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

        NS_LOG_LOGIC("Load balancing packet");
        socket->SendTo(packet, 0, m_peerAddress0);

        if (InetSocketAddress::IsMatchingType(m_peerAddress0))
        {
            NS_LOG_INFO ("At time " << Simulator::Now().As(Time::S) << " server sent " << packet->GetSize() << " bytes to " <<
                            InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
        }
    }

    // todo handle when client-streaming_server connection terminated
    // todo update session state
    // todo update status map
}

} // ns3
