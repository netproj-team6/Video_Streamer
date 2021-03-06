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
#include "ns3/seq-ts-header.h"

#include "ns3/load-balancer.h"
#include "ns3/load-balancer-helper.h"
#include "ns3/load-balancer-header.h"
#include "client.h"
#include "client-helper.h"
#include "server.h"
#include "server-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("test-lb");

static void
TxTime(std::string context, Ptr<const Packet> packet, const Address& address)
{
    if (context == "DST0")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST 0 sends packet seq " << seqTs.GetSeq());
    }
    else if (context == "DST1")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST 1 sends packet seq " << seqTs.GetSeq());
    }
    else if (context == "DST2")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST 2 sends packet seq " << seqTs.GetSeq());
    }
}

static void
RxTime(std::string context, Ptr<const Packet> packet, const Address &address)
{
    if (context == "SRC0")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "SRC 0 receives packet seq " << seqTs.GetSeq());
    }
    else if (context == "SRC1")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "SRC 1 receives packet seq " << seqTs.GetSeq());
    }
    else if (context == "LB")
    {
        SeqTsHeader requestType;
        packet->PeekHeader(requestType);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "LB receives packet type " << requestType.GetSeq());
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

static void
RtxTime(std::string context, Ptr<const Packet> packet, const Address& address)
{
    if (context == "DST0")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST 0 retransmits packet seq " << seqTs.GetSeq());
    }
    else if (context == "DST1")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST 1 retransmits packet seq " << seqTs.GetSeq());
    }
    else if (context == "DST2")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "DST 2 retransmits packet seq " << seqTs.GetSeq());
    }
}

int
main(int argc, char *argv[])
{
    LogComponentEnable("test-lb", LOG_LEVEL_INFO);
    //LogComponentEnable("LoadBalancer", LOG_LEVEL_DEBUG);
    //LogComponentEnable("StreamingClientApplication", LOG_LEVEL_FUNCTION);
    //LogComponentEnable("StreamingServerApplication", LOG_LEVEL_FUNCTION);
    //LogComponentEnable("StreamingServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("StreamingClientApplication", LOG_LEVEL_INFO);

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

    InternetStackHelper stack;
    stack.Install(srcNodes);
    stack.Install(nLB);
    stack.Install(dstNodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    NetDeviceContainer frontNICs0 = p2p.Install(frontNodes0);
    NetDeviceContainer frontNICs1 = p2p.Install(frontNodes1);
    NetDeviceContainer frontNICs2 = p2p.Install(frontNodes2);
    NetDeviceContainer backNICs0 = p2p.Install(backNodes0);
    NetDeviceContainer backNICs1 = p2p.Install(backNodes1);
    NetDeviceContainer backNICs2 = p2p.Install(backNodes2);

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

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t lbPort = 1010;
    uint16_t dstPort = 2020;

    Address frontLBAddress0(InetSocketAddress(frontInets0.GetAddress(0), lbPort));
    Address frontLBAddress1(InetSocketAddress(frontInets1.GetAddress(0), lbPort));
    Address frontLBAddress2(InetSocketAddress(frontInets2.GetAddress(0), lbPort));
    Address backDSTAddress0(InetSocketAddress(backInets0.GetAddress(1), dstPort));
    Address backDSTAddress1(InetSocketAddress(backInets1.GetAddress(1), dstPort));
    Address backDSTAddress2(InetSocketAddress(backInets2.GetAddress(1), dstPort));

    StreamingClientHelper src0(frontInets0.GetAddress(0), lbPort);
    src0.SetAttribute("LossRate", DoubleValue(0.1));
    src0.SetAttribute("PacketSize", UintegerValue(100));
    src0.SetAttribute("PacketsPerFrame", UintegerValue(100));
    src0.SetAttribute("BufferingSize", UintegerValue(1));
    src0.SetAttribute("PauseSize", UintegerValue(30));
    src0.SetAttribute("ResumeSize", UintegerValue(25));
    src0.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
    src0.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
    src0.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
    ApplicationContainer src0App = src0.Install(nSRC0);
    src0App.Start(Seconds(1.0));
    src0App.Stop(Seconds(5.0));
    src0App.Get(0)->TraceConnect("Rx", "SRC0", MakeCallback(&RxTime));

    StreamingClientHelper src1(frontInets1.GetAddress(0), lbPort);
    src0.SetAttribute("LossRate", DoubleValue(0.1));
    src0.SetAttribute("PacketSize", UintegerValue(100));
    src0.SetAttribute("PacketsPerFrame", UintegerValue(100));
    src0.SetAttribute("BufferingSize", UintegerValue(1));
    src0.SetAttribute("PauseSize", UintegerValue(30));
    src0.SetAttribute("ResumeSize", UintegerValue(25));
    src0.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
    src0.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
    src0.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
    ApplicationContainer src1App = src1.Install(nSRC1);
    src1App.Start(Seconds(1.0));
    src1App.Stop(Seconds(5.0));
    src1App.Get(0)->TraceConnect("Rx", "SRC1", MakeCallback(&RxTime));

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
    dst0.SetAttribute("Interval", TimeValue(Seconds(1. / 90.)));
    dst0.SetAttribute("PacketSize", UintegerValue(100));
    dst0.SetAttribute("PacketsPerFrame", UintegerValue(100));
    ApplicationContainer dst0App = dst0.Install(nDST0);
    dst0App.Start(Seconds(1.0));
    dst0App.Stop(Seconds(5.0));
    dst0App.Get(0)->TraceConnect("Tx", "DST0", MakeCallback(&TxTime));
    dst0App.Get(0)->TraceConnect("Rx", "DST0", MakeCallback(&RxTime));
    dst0App.Get(0)->TraceConnect("Rtx", "DST0", MakeCallback(&RtxTime));

    StreamingServerHelper dst1(dstPort);
    dst0.SetAttribute("Interval", TimeValue(Seconds(1. / 90.)));
    dst0.SetAttribute("PacketSize", UintegerValue(100));
    dst0.SetAttribute("PacketsPerFrame", UintegerValue(100));
    ApplicationContainer dst1App = dst1.Install(nDST1);
    dst1App.Start(Seconds(1.0));
    dst1App.Stop(Seconds(5.0));
    dst1App.Get(0)->TraceConnect("Tx", "DST1", MakeCallback(&TxTime));
    dst1App.Get(0)->TraceConnect("Rx", "DST1", MakeCallback(&RxTime));
    dst1App.Get(0)->TraceConnect("Rtx", "DST1", MakeCallback(&RtxTime));

    Simulator::Stop(Seconds(5.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
