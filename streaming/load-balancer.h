#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <map>
#include <vector>

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Address;
class Socket;
class Packet;

class LoadBalancer : public Application
{
public:
    static TypeId GetTypeId(void);
    LoadBalancer();
    virtual ~LoadBalancer();

protected:
    virtual void DoDispose(void);

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void HandleRead(Ptr<Socket> socket);
    Address AssignTargetAddress(uint32_t from);

    uint16_t m_port;
    Ptr<Socket> m_socket;
    Address m_local;

    Address m_peerAddress0;
    Address m_peerAddress1;
    Address m_peerAddress2;

    uint16_t m_peerWeight0;
    uint16_t m_peerWeight1;
    uint16_t m_peerWeight2;

    uint16_t m_peerCnt;
    uint16_t m_maxWeight;
    uint16_t m_curRound;
    uint16_t m_curIndex;

    std::map<uint32_t, uint16_t> m_sessions;
    std::vector<std::pair<Address, uint16_t>> m_states;

    TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;
    TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;
};

} // ns3

#endif //LOAD_BALANCER_H
