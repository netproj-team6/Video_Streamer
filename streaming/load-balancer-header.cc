#include "load-balancer-header.h"

#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LoadBalancerHeader");

NS_OBJECT_ENSURE_REGISTERED(LoadBalancerHeader);

LoadBalancerHeader::LoadBalancerHeader()
    : m_address(0),
    m_port(0)
{
    NS_LOG_FUNCTION(this);
}

TypeId
LoadBalancerHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::LoadBalancerHeader")
            .SetParent<Header>()
            .SetGroupName("Applications")
            .AddConstructor<LoadBalancerHeader>();
    return tid;
}

void
LoadBalancerHeader::SetIpv4Address(uint32_t address)
{
    NS_LOG_FUNCTION(this);
    m_address = address;
}

uint32_t
LoadBalancerHeader::GetIpv4Address() const
{
    NS_LOG_FUNCTION(this);
    return m_address;
}

void
LoadBalancerHeader::SetPort(uint16_t port)
{
    NS_LOG_FUNCTION(this);
    m_port = port;
}

uint16_t
LoadBalancerHeader::GetPort() const
{
    NS_LOG_FUNCTION(this);
    return m_port;
}

TypeId
LoadBalancerHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
LoadBalancerHeader::Print(std::ostream &os) const
{
    NS_LOG_FUNCTION(this << &os);
    os << "ipv4=" << m_address << " port=" << m_port;
}

void
LoadBalancerHeader::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    i.WriteHtonU32(m_address);
    i.WriteHtonU16(m_port);
}

uint32_t
LoadBalancerHeader::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return 4+2;
}

uint32_t
LoadBalancerHeader::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    m_address = i.ReadNtohU32();
    m_port = i.ReadNtohU16();
    return GetSerializedSize();
}

} // ns3
