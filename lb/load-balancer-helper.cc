#include "load-balancer-helper.h"
#include "ns3/load-balancer.h"

#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/names.h"
#include "ns3/data-rate.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"

namespace ns3 {

LoadBalancerHelper::LoadBalancerHelper()
{
    m_factory.SetTypeId(LoadBalancer::GetTypeId());
}

void
LoadBalancerHelper::SetAttribute(std::string name, const AttributeValue &value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
LoadBalancerHelper::Install(Ptr <Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
LoadBalancerHelper::Install(std::string nodeName) const
{
    Ptr <Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
LoadBalancerHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }
    return apps;
}

Ptr<Application>
LoadBalancerHelper::InstallPriv(Ptr <Node> node) const
{
//    Ptr <Application> app = m_factory.Create<LoadBalancer>();
    Ptr <Application> app = m_factory.Create<Application>();
    node->AddApplication(app);

    return app;
}

} // ns3
