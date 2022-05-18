#include "client-helper.h"
#include "client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

StreamingClientHelper::StreamingClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (StreamingClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

StreamingClientHelper::StreamingClientHelper (Address address)
{
  m_factory.SetTypeId (StreamingClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void 
StreamingClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
StreamingClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
StreamingClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
StreamingClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
StreamingClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<StreamingClient> ();
  node->AddApplication (app);

  return app;
}

}
