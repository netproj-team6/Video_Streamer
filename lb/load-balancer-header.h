#ifndef LOAD_BALANCER_HEADER_H
#define LOAD_BALANCER_HEADER_H

#include "ns3/header.h"

namespace ns3 {

class LoadBalancerHeader : public Header
{
public:
    static TypeId GetTypeId(void);
    LoadBalancerHeader();

    void SetIpv4Address(uint32_t address);
    uint32_t GetIpv4Address(void) const;
    void SetPort(uint16_t port);
    uint16_t GetPort(void) const;

    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:
    uint32_t m_address;
    uint16_t m_port;
};

} // ns3

#endif //LOAD_BALANCER_HEADER_H
