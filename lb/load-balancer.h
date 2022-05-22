#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

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

    uint16_t m_port;
    Ptr<Socket> m_socket;
    Address m_local;

    Address m_peerAddress0;
    uint16_t m_peerPort0;
    uint16_t m_peerWeight0;

    TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;
    TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;
};

} // ns3

#endif //LOAD_BALANCER_H
