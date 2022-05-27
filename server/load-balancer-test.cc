#include "ns3/load-balancer-helper.h"
#include "ns3/load-balancer.h"
#include "ns3/load-balancer-header.h"

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/uinteger.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/applications-module.h"
#include "server-helper.h"
#include "server.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("test-lb");

static void
RxTime(std::string context, Ptr<const Packet> packet, const Address &address)
{
    if (context == "LB")
    {
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << " LB   receives " << packet->GetSize());
    }
    else if (context == "DST0")
    {
        LoadBalancerHeader header;
        packet->PeekHeader(header);
        uint32_t clientAddress = header.GetIpv4Address();
        uint16_t clientPort = header.GetPort();
        InetSocketAddress transport(Ipv4Address(clientAddress), clientPort);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST 0 receives " << packet->GetSize() << " from (" <<
                    clientAddress << " " << clientPort << " " << transport << ")");
    }
    else if (context == "DST1")
    {
        LoadBalancerHeader header;
        packet->PeekHeader(header);
        uint32_t clientAddress = header.GetIpv4Address();
        uint16_t clientPort = header.GetPort();
        InetSocketAddress transport(Ipv4Address(clientAddress), clientPort);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST 1 receives " << packet->GetSize() << " from (" <<
                    clientAddress << " " << clientPort << " " << transport << ")");
    }
    else if (context == "DST2")
    {
        LoadBalancerHeader header;
        packet->PeekHeader(header);
        uint32_t clientAddress = header.GetIpv4Address();
        uint16_t clientPort = header.GetPort();
        InetSocketAddress transport(Ipv4Address(clientAddress), clientPort);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST 2 receives " << packet->GetSize() << " from (" <<
                    clientAddress << " " << clientPort << " " << transport << ")");
    }
}

int
main(int argc, char *argv[])
{
    LogComponentEnable("test-lb", LOG_LEVEL_INFO);
    LogComponentEnable("LoadBalancer", LOG_LEVEL_DEBUG);

    Ptr<Node> nSRC0 = CreateObject<Node>();
    Ptr<Node> nSRC1 = CreateObject<Node>();
    Ptr<Node> nSRC2 = CreateObject<Node>();
    Ptr<Node> nLB = CreateObject<Node>();
    Ptr<Node> nDST0 = CreateObject<Node>();
    Ptr<Node> nDST1 = CreateObject<Node>();
    Ptr<Node> nDST2 = CreateObject<Node>();

    NodeContainer srcNodes = NodeContainer(nSRC0, nSRC1, nSRC2);
    NodeContainer dstNodes = NodeContainer(nDST0, nDST1, nDST2);
    NodeContainer frontNodes0 = NodeContainer(nLB, nSRC0);
    NodeContainer frontNodes1 = NodeContainer(nLB, nSRC1);
    NodeContainer frontNodes2 = NodeContainer(nLB, nSRC2);
    NodeContainer backNodes0 = NodeContainer(nLB, nDST0);
    NodeContainer backNodes1 = NodeContainer(nLB, nDST1);
    NodeContainer backNodes2 = NodeContainer(nLB, nDST2);

    NodeContainer Nodes0 = NodeContainer(nSRC0, nDST0);
    NodeContainer Nodes1 = NodeContainer(nSRC1, nDST1);
    NodeContainer Nodes2 = NodeContainer(nSRC2, nDST2);


    InternetStackHelper stack;
    stack.Install(srcNodes);
    stack.Install(nLB);
    stack.Install(dstNodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    NetDeviceContainer frontNICs0 = p2p.Install(frontNodes0);
    NetDeviceContainer frontNICs1 = p2p.Install(frontNodes1);
    NetDeviceContainer frontNICs2 = p2p.Install(frontNodes2);
    NetDeviceContainer backNICs0 = p2p.Install(backNodes0);
    NetDeviceContainer backNICs1 = p2p.Install(backNodes1);
    NetDeviceContainer backNICs2 = p2p.Install(backNodes2);

    NetDeviceContainer NICs0 = p2p.Install(Nodes0);
    NetDeviceContainer NICs1 = p2p.Install(Nodes1);
    NetDeviceContainer NICs2 = p2p.Install(Nodes2);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer frontInets0 = ipv4.Assign(frontNICs0);
    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer frontInets1 = ipv4.Assign(frontNICs1);
    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer frontInets2 = ipv4.Assign(frontNICs2);
    ipv4.SetBase("10.2.1.0", "255.255.255.0");
    Ipv4InterfaceContainer backInets0 = ipv4.Assign(backNICs0);
    ipv4.SetBase("10.2.2.0", "255.255.255.0");
    Ipv4InterfaceContainer backInets1 = ipv4.Assign(backNICs1);
    ipv4.SetBase("10.2.3.0", "255.255.255.0");
    Ipv4InterfaceContainer backInets2 = ipv4.Assign(backNICs2);

    ipv4.SetBase("10.3.1.0", "255.255.255.0");
    Ipv4InterfaceContainer Inets0 = ipv4.Assign(NICs0);
    ipv4.SetBase("10.3.2.0", "255.255.255.0");
    Ipv4InterfaceContainer Inets1 = ipv4.Assign(NICs1);
    ipv4.SetBase("10.3.3.0", "255.255.255.0");
    Ipv4InterfaceContainer Inets2 = ipv4.Assign(NICs2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t sPort = 10;
    uint16_t lbPort = 1010;
    uint16_t dstPort = 2020;

    Address frontLBAddress0(InetSocketAddress(frontInets0.GetAddress(0), lbPort));
    Address frontLBAddress1(InetSocketAddress(frontInets1.GetAddress(0), lbPort));
    Address frontLBAddress2(InetSocketAddress(frontInets2.GetAddress(0), lbPort));
    Address backDSTAddress0(InetSocketAddress(backInets0.GetAddress(1), dstPort));
    Address backDSTAddress1(InetSocketAddress(backInets1.GetAddress(1), dstPort));
    Address backDSTAddress2(InetSocketAddress(backInets2.GetAddress(1), dstPort));

    OnOffHelper src0("ns3::UdpSocketFactory", frontLBAddress0);
    src0.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    src0.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    src0.SetAttribute("DataRate", DataRateValue(100000));
    ApplicationContainer src0App = src0.Install(nSRC0);
    src0App.Start(Seconds(1.0));
    src0App.Stop(Seconds(5.0));

    OnOffHelper src1("ns3::UdpSocketFactory", frontLBAddress1);
    src1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    src1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    src1.SetAttribute("DataRate", DataRateValue(100000));
    ApplicationContainer src1App = src1.Install(nSRC1);
    src1App.Start(Seconds(1.0));
    src1App.Stop(Seconds(5.0));

    OnOffHelper src2("ns3::UdpSocketFactory", frontLBAddress2);
    src2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    src2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    src2.SetAttribute("DataRate", DataRateValue(100000));
    ApplicationContainer src2App = src2.Install(nSRC2);
    src2App.Start(Seconds(1.0));
    src2App.Stop(Seconds(5.0));

    LoadBalancerHelper lb;
    lb.SetAttribute("Port", UintegerValue(lbPort));
    lb.SetAttribute("FirstAddress", AddressValue(backDSTAddress0));
    lb.SetAttribute("FirstWeight", UintegerValue(1));
    lb.SetAttribute("SecondAddress", AddressValue(backDSTAddress1));
    lb.SetAttribute("SecondWeight", UintegerValue(1));
    lb.SetAttribute("ThirdAddress", AddressValue(backDSTAddress2));
    lb.SetAttribute("ThirdWeight", UintegerValue(1));
    ApplicationContainer lbApp = lb.Install(nLB);
    lbApp.Start(Seconds(1.0));
    lbApp.Stop(Seconds(5.0));
    lbApp.Get(0)->TraceConnect("Rx", "LB", MakeCallback(&RxTime));

    StreamingServerHelper dst0(dstPort);
    dst0.SetAttribute("Interval", TimeValue(Seconds(4)));
    ApplicationContainer dst0App = dst0.Install(nDST0);
    dst0App.Start(Seconds(1.0));
    dst0App.Stop(Seconds(5.0));
    //dst0App.Get(0)->TraceConnect("Rx", "DST0", MakeCallback(&RxTime));

    StreamingServerHelper dst1(dstPort);
    dst1.SetAttribute("Interval", TimeValue(Seconds(4)));
    ApplicationContainer dst1App = dst1.Install(nDST1);
    dst1App.Start(Seconds(1.0));
    dst1App.Stop(Seconds(5.0));
    //dst1App.Get(0)->TraceConnect("Rx", "DST1", MakeCallback(&RxTime));

    StreamingServerHelper dst2(dstPort);
    dst2.SetAttribute("Interval", TimeValue(Seconds(4)));
    ApplicationContainer dst2App = dst2.Install(nDST2);
    dst2App.Start(Seconds(1.0));
    dst2App.Stop(Seconds(5.0));
    //dst2App.Get(0)->TraceConnect("Rx", "DST2", MakeCallback(&RxTime));

    PacketSinkHelper ddst0("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sPort));
    ApplicationContainer ddst0App = ddst0.Install(nSRC0);
    ddst0App.Start(Seconds(1.0));
    ddst0App.Stop(Seconds(5.0));
    ddst0App.Get(0)->TraceConnect("Rx", "DST0", MakeCallback(&RxTime));

    PacketSinkHelper ddst1("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sPort));
    ApplicationContainer ddst1App = ddst1.Install(nSRC1);
    ddst1App.Start(Seconds(1.0));
    ddst1App.Stop(Seconds(5.0));
    ddst1App.Get(0)->TraceConnect("Rx", "DST1", MakeCallback(&RxTime));

    PacketSinkHelper ddst2("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sPort));
    ApplicationContainer ddst2App = ddst2.Install(nSRC2);
    ddst2App.Start(Seconds(1.0));
    ddst2App.Stop(Seconds(5.0));
    ddst2App.Get(0)->TraceConnect("Rx", "DST2", MakeCallback(&RxTime));

    Simulator::Stop(Seconds(5.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
