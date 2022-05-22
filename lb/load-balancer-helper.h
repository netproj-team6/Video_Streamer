#ifndef LOAD_BALANCER_HELPER_H
#define LOAD_BALANCER_HELPER_H

#include "ns3/load-balancer.h"

#include <stdint.h>
#include <string>

#include "ns3/object-factory.h"
#include "ns3/net-device.h"
#include "ns3/attribute.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

namespace ns3 {

class LoadBalancerHelper
{
public:
    LoadBalancerHelper();
    void SetAttribute(std::string name, const AttributeValue &value);
    ApplicationContainer Install(Ptr <Node> node) const;
    ApplicationContainer Install(std::string nodeName) const;
    ApplicationContainer Install(NodeContainer c) const;

private:
    Ptr <Application> InstallPriv(Ptr <Node> node) const;

    ObjectFactory m_factory;
};

} // ns3

#endif //LOAD_BALANCER_HELPER_H
