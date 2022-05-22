#include "ns3/load-balancer-helper.h"
#include "ns3/load-balancer.h"

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/uinteger.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("test-lb");

static void
RxTime(std::string context, Ptr<const Packet> packet, const Address &address)
{
    if (context == "LB")
    {
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << " LB receives " << packet->GetSize());
    }
    else if (context == "DST")
    {
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST receives " << packet->GetSize());
    }
}

int
main(int argc, char *argv[])
{
    LogComponentEnable("test-lb", LOG_LEVEL_INFO);

    NS_LOG_INFO("### SCRATCH START ###");

    Ptr<Node> nSRC = CreateObject<Node>();
    Ptr<Node> nLB = CreateObject<Node>();
    Ptr<Node> nDST = CreateObject<Node>();

    NodeContainer allNodes = NodeContainer(nSRC, nLB, nDST);
    NodeContainer frontNodes = NodeContainer(nSRC, nLB);
    NodeContainer backNodes = NodeContainer(nLB, nDST);

    InternetStackHelper stack;
    stack.Install(allNodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    NetDeviceContainer frontNICs = p2p.Install(frontNodes);
    NetDeviceContainer backNICs = p2p.Install(backNodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer frontInets = ipv4.Assign(frontNICs);
    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer backInets = ipv4.Assign(backNICs);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t lbPort = 1010;
    uint16_t dstPort = 2020;

    Address frontLBAddress(InetSocketAddress(frontInets.GetAddress(1), lbPort));
    Address backDSTAddress(InetSocketAddress(backInets.GetAddress(1), dstPort));

    OnOffHelper src("ns3::UdpSocketFactory", frontLBAddress);
    src.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    src.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    src.SetAttribute("DataRate", DataRateValue(500000));
    ApplicationContainer srcApp = src.Install(nSRC);
    srcApp.Start(Seconds(1.0));
    srcApp.Stop(Seconds(5.0));

    LoadBalancerHelper lb;
    lb.SetAttribute("Port", UintegerValue(lbPort));
    lb.SetAttribute("FirstAddress", AddressValue(backDSTAddress));
//    lb.SetAttribute("FirstPort", dstPort);
//    lb.SetAttribute("FirstWeight", UintegerValue(1));
    ApplicationContainer lbApp = lb.Install(nLB);
    lbApp.Start(Seconds(1.0));
    lbApp.Stop(Seconds(5.0));
    lbApp.Get(0)->TraceConnect("Rx", "LB", MakeCallback(&RxTime));

    PacketSinkHelper dst("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), dstPort));
    ApplicationContainer dstApp = dst.Install(nDST);
    dstApp.Start(Seconds(1.0));
    dstApp.Stop(Seconds(5.0));
    dstApp.Get(0)->TraceConnect("Rx", "DST", MakeCallback(&RxTime));

    Simulator::Stop(Seconds(5.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
