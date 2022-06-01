#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-grid.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"

#include "ns3/load-balancer-helper.h"
#include "server/server-helper.h"
#include "server/server.h"
#include "client/client-helper.h"
#include "client/client.h"
#include "ns3/bridge-module.h"

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("video_streamer");



namespace vs {

static void
RxTime_0(std::string context, Ptr<const Packet> packet, const Address &address)
{   
    if (context == "LB")
    {
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << " LB   receives " << packet->GetSize());
    }
    else if (context == "Stream Server 0")
    {
        LoadBalancerHeader header;
        packet->PeekHeader(header);
        uint32_t clientAddress = header.GetIpv4Address();
        uint16_t clientPort = header.GetPort();
        InetSocketAddress transport(Ipv4Address(clientAddress), clientPort);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "Stream Server 0 receives " << packet->GetSize() << " from (" <<
                    clientAddress << " " << clientPort << " " << transport << ")");
    }
    else if (context == "Stream Server 1")
    {
        LoadBalancerHeader header;
        packet->PeekHeader(header);
        uint32_t clientAddress = header.GetIpv4Address();
        uint16_t clientPort = header.GetPort();
        InetSocketAddress transport(Ipv4Address(clientAddress), clientPort);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "Stream Server 1 receives " << packet->GetSize() << " from (" <<
                    clientAddress << " " << clientPort << " " << transport << ")");
    }
    else if (context == "Stream Server 2")
    {
        LoadBalancerHeader header;
        packet->PeekHeader(header);
        uint32_t clientAddress = header.GetIpv4Address();
        uint16_t clientPort = header.GetPort();
        InetSocketAddress transport(Ipv4Address(clientAddress), clientPort);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "Stream Server 2 receives " << packet->GetSize() << " from (" <<
                    clientAddress << " " << clientPort << " " << transport << ")");
    }
    
}

static void
TxTime(std::string context, Ptr<const Packet> packet, const Address& address)
{
    
    if (context == "Stream Server 0")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "Stream Server 0 sends packet seq " << seqTs.GetSeq());
    }
    else if (context == "Stream Server 1")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "Stream Server 1 sends packet seq " << seqTs.GetSeq());
    }
    else if (context == "Stream Server 2")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "Stream Server 2 sends packet seq " << seqTs.GetSeq());
    }
    
}

static void
RxTime(std::string context, Ptr<const Packet> packet, const Address &address)
{
    
    if (context.substr(0, 6) == "Global")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << context << " receives packet seq " << seqTs.GetSeq());
    }

    
	else if (context.substr(0, 4) == "CSMA")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << context << " receives packet seq " << seqTs.GetSeq());
    }

	else if (context.substr(0, 4) == "Wifi")
    {
        SeqTsHeader seqTs;
        packet->PeekHeader(seqTs);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << context << " receives packet seq " << seqTs.GetSeq());
    }
	
    else if (context == "LB")
    {
        SeqTsHeader requestType;
        packet->PeekHeader(requestType);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << "LB receives packet type " << requestType.GetSeq());
    }
    else if (context.substr(0, 13) == "Stream Server")
    {
        LoadBalancerHeader header;
        packet->PeekHeader(header);
        uint32_t clientAddress = header.GetIpv4Address();
        uint16_t clientPort = header.GetPort();
        InetSocketAddress transport(Ipv4Address(clientAddress), clientPort);
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "]   \t" << context << " receives " << packet->GetSize() << " from (" <<
                    Ipv4Address(clientAddress) << " " << clientPort << " " << InetSocketAddress::ConvertFrom(transport).GetIpv4()  << ")");
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
};



int main(int argc, char *argv[])
{

	double LossRate = 0.;
	double DownRate = 0.0;
	double ErrorRate = 0.005;
	uint32_t payloadSize = 1472; // bytes
	uint64_t simulationTime = 1;
    uint32_t nMpdu = 10;

	uint32_t numOfSwitches = 4;
	uint32_t numOfStream= 3;
	uint32_t numOfGlobalUsers= 3;
	uint32_t numOfWifiUsers= 3;
	uint32_t numOfCsma= 3;

	// LogComponentEnable("LoadBalancer", LOG_LEVEL_ALL);
	// LogComponentEnable("StreamingClientApplication", LOG_LEVEL_INFO);
	// LogComponentEnable("StreamingServerApplication", LOG_LEVEL_ALL);
	// LogComponentEnable("StreamingClientApplication", LOG_LEVEL_ALL);
	LogComponentEnable("video_streamer", LOG_LEVEL_INFO);

	// LogComponentEnable("PointToPointNetDevice", LOG_LEVEL_LOGIC);

	// LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_ALL);

	// Parsing 
	CommandLine cmd;
	cmd.AddValue("LossRate", "How do networks loss their packet? It should be between [0, 1]", LossRate);
	cmd.AddValue("DownRate", "It is the probability how servers crash every 10 secs. It should be between [0, 1]", DownRate);
	cmd.AddValue("ErrorRate", "ErrorRate in the Global switch network.]", ErrorRate);
	cmd.AddValue("payloadSize", "Payload size in bytes", payloadSize);
    cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
    cmd.AddValue("nMpdu", "Simulation time in seconds", nMpdu);
	cmd.Parse(argc, argv);

	///// 1. Craete nodes, Channel, NetDevices
	////////// global switches 
	NodeContainer globalSwitches;
	globalSwitches.Create(numOfSwitches);

	PointToPointHelper gsP2p;
	gsP2p.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
	gsP2p.SetChannelAttribute("Delay", StringValue("10ms"));
	
	NetDeviceContainer gsLeftDevices;
	NetDeviceContainer gsRightDevices;
	// NetDeviceContainer GS_CrossDevices;
	for (uint32_t i = 0; i < numOfSwitches; i++)
	{
		NetDeviceContainer nd = gsP2p.Install(globalSwitches.Get(i % numOfSwitches), globalSwitches.Get((i + 1) % numOfSwitches));
		gsLeftDevices.Add(nd.Get(0));
		gsRightDevices.Add(nd.Get(1));
	}
	
	// for (uint32_t i = 0; i < numOfSwitches / 2; i++)
	// {
	// 	NetDeviceContainer nd = p2pGS.Install(globalSwitches.Get(i % numOfSwitches), globalSwitches.Get((i + 2) % numOfSwitches));
	// 	GS_CrossDevices.Add(nd.Get(0));
	// 	GS_CrossDevices.Add(nd.Get(1));
	// }





	// Define an error model
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(ErrorRate));
	for (uint32_t i = 0; i < 4; i++)
	{
		gsLeftDevices.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
	}

	////////// Server
	NodeContainer streamNodes, lbNode;
	streamNodes.Create(3);
	lbNode.Create(1);

	PointToPointHelper serverP2p;
	serverP2p.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
	serverP2p.SetChannelAttribute("Delay", StringValue("2ms"));

	NetDeviceContainer streamHubDevices;
	NetDeviceContainer streamDevices;
	for (uint32_t i = 0; i < numOfStream; i++)
	{
		NetDeviceContainer nd = serverP2p.Install(globalSwitches.Get(0), streamNodes.Get(i));
		streamHubDevices.Add(nd.Get(0));
		streamDevices.Add(nd.Get(1));
	}

	NetDeviceContainer lbHubDevices;
	NetDeviceContainer lbDevices;
	{
		NetDeviceContainer nd = serverP2p.Install(globalSwitches.Get(0), lbNode.Get(0));
		lbHubDevices.Add(nd.Get(0));
		lbDevices.Add(nd.Get(1));
	}


	////////// Global Users
	NodeContainer globalUserNodes;
	globalUserNodes.Create(numOfGlobalUsers);

	PointToPointHelper guP2p;
	guP2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
	guP2p.SetChannelAttribute("Delay", StringValue("20ms"));

	NetDeviceContainer guHubDevices;
	NetDeviceContainer guDevices;
	for (uint32_t i = 0; i < numOfGlobalUsers; i++)
	{
		NetDeviceContainer nd = guP2p.Install(globalSwitches.Get(2), globalUserNodes.Get(i));
		guHubDevices.Add(nd.Get(0));
		guDevices.Add(nd.Get(1));
	}
	

	////////// Wifi group
	NodeContainer wifiApNode;
    wifiApNode.Create(1);
	NodeContainer wifiStaNode;
    wifiStaNode.Create(numOfWifiUsers);

	//////////////////// inner 
	YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    phy.SetChannel(channel.Create());

	WifiMacHelper mac;
    Ssid ssid("streamWifi");
    uint32_t maxAmpduSize = nMpdu * (payloadSize + 100);
    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(false),
                "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
	
	WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("HtMcs7"), 
																						"ControlMode", StringValue("HtMcs0"));

	NetDeviceContainer wifiStaDevice;
    wifiStaDevice = wifi.Install(phy, mac, wifiStaNode);

    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid),
                "BeaconInterval", TimeValue(MicroSeconds(102400)),
                "BeaconGeneration", BooleanValue(true));

    NetDeviceContainer wifiApDevice;
    wifiApDevice = wifi.Install(phy, mac, wifiApNode);

	//////////////////// outter
	PointToPointHelper wifiP2p;
	wifiP2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
	wifiP2p.SetChannelAttribute("Delay", StringValue("5ms"));

	NetDeviceContainer wifiApOutHubDevice;
	NetDeviceContainer wifiApOutDevice;
	{
		NetDeviceContainer nd = wifiP2p.Install(globalSwitches.Get(1), wifiApNode.Get(0));
		wifiApOutHubDevice.Add(nd.Get(0));
		wifiApOutDevice.Add(nd.Get(1));
	}


	////////// CSMA
	//////////////////// inner
	NodeContainer csmaNodes;	
	csmaNodes.Add(globalSwitches.Get(3));
	csmaNodes.Create(numOfCsma);
	
	

	CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install(csmaNodes);

	///// 2. Install internet stack
	InternetStackHelper stack;
	// stack.Install(globalSwitches.Get(0));
	// stack.Install(globalSwitches.Get(1));
	// stack.Install(globalSwitches.Get(2));
	// stack.Install(streamServerNodes);
	// stack.Install(balanceServerNode);
	// stack.Install(globalUserNodes);
	// stack.Install(wifiApNode);
    // stack.Install(wifiStaNode);
	// stack.Install(csma_GS_Nodes);
	stack.InstallAll();

	///// 3. Assign address
	Ipv4AddressHelper GlobalAddress;
    GlobalAddress.SetBase("10.1.1.0", "255.255.255.0");

	////////// global switches 
	Ipv4InterfaceContainer gs_interfaceLeft;
	Ipv4InterfaceContainer gs_interfaceRight;
	// Ipv4InterfaceContainer interfacesGS_Cross;
	for (uint32_t i = 0; i < numOfSwitches; i++)
	{
		gs_interfaceLeft.Add(GlobalAddress.Assign(gsLeftDevices.Get(i)));
		gs_interfaceRight.Add(GlobalAddress.Assign(gsRightDevices.Get( (i+3) % numOfSwitches )));
		GlobalAddress.NewNetwork();
	}
	// for (uint32_t i = 0; i < (numOfSwitches / 2); i++)
	// {
	// 	interfacesGS_Cross.Add(GlobalAddress.Assign(GS_CrossDevices.Get(i*2)));
	// 	interfacesGS_Cross.Add(GlobalAddress.Assign(GS_CrossDevices.Get(i*2+1)));
	// 	GlobalAddress.NewAddress();
	// }

	////////// Server
	Ipv4InterfaceContainer streamHub_interfaces;
	Ipv4InterfaceContainer stream_interfaces;
	for (uint32_t i = 0; i < numOfStream; i++)
	{
		streamHub_interfaces.Add(GlobalAddress.Assign(streamHubDevices.Get(i)));
		stream_interfaces.Add(GlobalAddress.Assign(streamDevices.Get(i)));
		GlobalAddress.NewNetwork();
	}
	
	Ipv4InterfaceContainer lbHub_interfaces;
	Ipv4InterfaceContainer lb_interfaces;
	lbHub_interfaces.Add(GlobalAddress.Assign(lbHubDevices.Get(0)));
	lb_interfaces.Add(GlobalAddress.Assign(lbDevices.Get(0)));
	GlobalAddress.NewNetwork();


	////////// Global Users
	Ipv4InterfaceContainer guHub_interfaces;
	Ipv4InterfaceContainer gu_interfaces;
	for (uint32_t i = 0; i < guDevices.GetN(); i++)
	{
		guHub_interfaces.Add(GlobalAddress.Assign(guHubDevices.Get(i)));
		gu_interfaces.Add(GlobalAddress.Assign(guDevices.Get(i)));
		GlobalAddress.NewNetwork();
	}	
	
	
	////////// Wifi group
	//////////////////// outer
	Ipv4InterfaceContainer wifiApOutHub_interfaces;
	Ipv4InterfaceContainer wifiApOut_interfaces;
	wifiApOutHub_interfaces.Add(GlobalAddress.Assign(wifiApOutHubDevice));
	wifiApOut_interfaces.Add(GlobalAddress.Assign(wifiApOutDevice));
	GlobalAddress.NewNetwork();

	//////////////////// inner
	Ipv4AddressHelper wifiAddress;
    wifiAddress.SetBase("192.168.2.0", "255.255.255.0");
	Ipv4InterfaceContainer Ap_interface;
    Ap_interface = wifiAddress.Assign(wifiApDevice);
    Ipv4InterfaceContainer Sta_interface;
	Sta_interface = wifiAddress.Assign(wifiStaDevice);

	// Ap_interface.Add(GlobalAddress.Assign(wifiApDevice.Get(0)));
	// for (uint32_t i = 0; i < numOfWifiUsers; i++)
	// {
	// 	Sta_interface.Add(GlobalAddress.Assign(wifiStaDevice.Get(i)));
	// }
	// GlobalAddress.NewNetwork();

	MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

	positionAlloc->Add(Vector(0.0, 0.0, 0.0));
	for (uint32_t i = 0; i < numOfWifiUsers + 1; i++)
	{
		positionAlloc->Add(Vector(1.0 + (double)(std::pow(100, i)), 0.0, 0.0));
	}
    mobility.SetPositionAllocator(positionAlloc);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    mobility.Install(wifiApNode);
    mobility.Install(wifiStaNode);




	// ////////// CSMA
	// //////////////////// inner
	// Ipv4AddressHelper csmaAddress;
    // csmaAddress.SetBase("192.168.3.0", "255.255.255.0");

	Ipv4InterfaceContainer csma_interfaces;
	
	csma_interfaces.Add(GlobalAddress.Assign(csmaDevices.Get(0)));
	csma_interfaces.Add(GlobalAddress.Assign(csmaDevices.Get(1)));
	csma_interfaces.Add(GlobalAddress.Assign(csmaDevices.Get(2)));
	csma_interfaces.Add(GlobalAddress.Assign(csmaDevices.Get(3)));
	GlobalAddress.NewNetwork();

	///// 4. Set up applications
	// ////////////////////////////////////////////////////////
	int32_t test_case = 1;

	uint16_t lbPort = 1010;
	uint16_t dstPort = 2020;
	Address streamAddress0(InetSocketAddress(stream_interfaces.GetAddress(0), dstPort));
    Address streamAddress1(InetSocketAddress(stream_interfaces.GetAddress(1), dstPort));
    Address streamAddress2(InetSocketAddress(stream_interfaces.GetAddress(2), dstPort));

    LoadBalancerHelper lb;
    lb.SetAttribute("Port", UintegerValue(lbPort));
    lb.SetAttribute("FirstAddress", AddressValue(streamAddress0));
    lb.SetAttribute("FirstWeight", UintegerValue(1));
    lb.SetAttribute("SecondAddress", AddressValue(streamAddress1));
    lb.SetAttribute("SecondWeight", UintegerValue(1));
    lb.SetAttribute("ThirdAddress", AddressValue(streamAddress2));
    lb.SetAttribute("ThirdWeight", UintegerValue(1));
    ApplicationContainer lbApp = lb.Install(lbNode);
    lbApp.Start(Seconds(1.0));
    lbApp.Stop(Seconds(simulationTime + 1));
	if(test_case == 0)
	{
		lbApp.Get(0)->TraceConnect("Rx", "LB", MakeCallback(&vs::RxTime_0));
	}
	else
	{
		lbApp.Get(0)->TraceConnect("Rx", "LB", MakeCallback(&vs::RxTime));
	}
    

	// Address clientAddress0(InetSocketAddress(globalUsers_interfaces.GetAddress(0), lbPort));
    // Address clientAddress1(InetSocketAddress(globalUsers_interfaces.GetAddress(1), lbPort));
    // Address clientAddress2(InetSocketAddress(globalUsers_interfaces.GetAddress(1), lbPort));

	Address lbAddress(InetSocketAddress(lb_interfaces.GetAddress(0), lbPort));
	Ipv4Address lbv4Address = lb_interfaces.GetAddress(0);

	
	if(test_case == 0)
	{	
		auto server_node = globalUserNodes.Get(1);
		auto server_ip = gu_interfaces.GetAddress(1);
		auto client_node = lbNode.Get(0);

		UdpServerHelper myServer(9);
		ApplicationContainer serverApp = myServer.Install(server_node);
		serverApp.Start(Seconds(0.0));
		serverApp.Stop(Seconds(simulationTime + 1));

		UdpClientHelper myClient(server_ip, 9);
		myClient.SetAttribute("MaxPackets", UintegerValue(4294967295)); // Maximum number of packets = 2^ 32
		myClient.SetAttribute("Interval", TimeValue(Time("0.00002"))); // packets / s
		myClient.SetAttribute("PacketSize", UintegerValue(payloadSize));

		ApplicationContainer clientApp = myClient.Install(client_node);
		clientApp.Start(Seconds(1.0));
		clientApp.Stop(Seconds(simulationTime + 1));

		OnOffHelper src0("ns3::UdpSocketFactory", lbAddress);
		src0.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
		src0.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
		src0.SetAttribute("DataRate", DataRateValue(100000));
		ApplicationContainer src0App = src0.Install(globalUserNodes.Get(0));
		src0App.Start(Seconds(1.0));
		src0App.Stop(Seconds(simulationTime + 1));

		OnOffHelper src1("ns3::UdpSocketFactory", lbAddress);
		src1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
		src1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
		src1.SetAttribute("DataRate", DataRateValue(100000));
		ApplicationContainer src1App = src1.Install(globalUserNodes.Get(1));
		src1App.Start(Seconds(1.0));
		src1App.Stop(Seconds(simulationTime + 1));

		OnOffHelper src2("ns3::UdpSocketFactory", lbAddress);
		src2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
		src2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
		src2.SetAttribute("DataRate", DataRateValue(100000));
		ApplicationContainer src2App = src2.Install(globalUserNodes.Get(2));
		src2App.Start(Seconds(1.0));
    	src2App.Stop(Seconds(simulationTime + 1));
	
		PacketSinkHelper dst0("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), dstPort));
		ApplicationContainer dst0App = dst0.Install(streamNodes.Get(0));
		dst0App.Start(Seconds(1.0));
		dst0App.Stop(Seconds(5.0));
		dst0App.Get(0)->TraceConnect("Rx", "DST0", MakeCallback(&vs::RxTime_0));

		PacketSinkHelper dst1("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), dstPort));
		ApplicationContainer dst1App = dst1.Install(streamNodes.Get(1));
		dst1App.Start(Seconds(1.0));
		dst1App.Stop(Seconds(5.0));
		dst1App.Get(0)->TraceConnect("Rx", "DST1", MakeCallback(&vs::RxTime_0));

		PacketSinkHelper dst2("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), dstPort));
		ApplicationContainer dst2App = dst2.Install(streamNodes.Get(2));
		dst2App.Start(Seconds(1.0));
		dst2App.Stop(Seconds(5.0));
		dst2App.Get(0)->TraceConnect("Rx", "DST2", MakeCallback(&vs::RxTime_0));
	}
	else if (test_case == 1)
	{
		StreamingServerHelper streamHelper0(dstPort);
		streamHelper0.SetAttribute("Interval", TimeValue(Seconds(1. / 90.)));
		streamHelper0.SetAttribute("PacketSize", UintegerValue(100));
		streamHelper0.SetAttribute("PacketsPerFrame", UintegerValue(100));
		ApplicationContainer streamApp0 = streamHelper0.Install(streamNodes.Get(0));
		streamApp0.Start(Seconds(1.0));
		streamApp0.Stop(Seconds(5.0));
		streamApp0.Get(0)->TraceConnect("Tx", "Stream Server 0", MakeCallback(&vs::TxTime));
		streamApp0.Get(0)->TraceConnect("Rx", "Stream Server 0", MakeCallback(&vs::RxTime));
		streamApp0.Get(0)->TraceConnect("Rtx", "Stream Server 0", MakeCallback(&vs::RtxTime));
        
		StreamingServerHelper streamHelper1(dstPort);
		streamHelper1.SetAttribute("Interval", TimeValue(Seconds(1. / 90.)));
		streamHelper1.SetAttribute("PacketSize", UintegerValue(100));
		streamHelper1.SetAttribute("PacketsPerFrame", UintegerValue(100));
		ApplicationContainer streamApp1 = streamHelper1.Install(streamNodes.Get(1));
		streamApp1.Start(Seconds(1.0));
		streamApp1.Stop(Seconds(5.0));
		streamApp1.Get(0)->TraceConnect("Tx", "Stream Server 1", MakeCallback(&vs::TxTime));
		streamApp1.Get(0)->TraceConnect("Rx", "Stream Server 1", MakeCallback(&vs::RxTime));
		streamApp1.Get(0)->TraceConnect("Rtx", "Stream Server 1", MakeCallback(&vs::RtxTime));

		StreamingServerHelper streamHelper2(dstPort);
		streamHelper2.SetAttribute("Interval", TimeValue(Seconds(1. / 90.)));
		streamHelper2.SetAttribute("PacketSize", UintegerValue(100));
		streamHelper2.SetAttribute("PacketsPerFrame", UintegerValue(100));
		ApplicationContainer streamApp2 = streamHelper2.Install(streamNodes.Get(2));
		streamApp2.Start(Seconds(1.0));
		streamApp2.Stop(Seconds(5.0));
		streamApp2.Get(0)->TraceConnect("Tx", "Stream Server 2", MakeCallback(&vs::TxTime));
		streamApp2.Get(0)->TraceConnect("Rx", "Stream Server 2", MakeCallback(&vs::RxTime));
		streamApp2.Get(0)->TraceConnect("Rtx", "Stream Server 2", MakeCallback(&vs::RtxTime));
        
       /* 
		StreamingClientHelper clientHelper0(lbv4Address, lbPort);
		clientHelper0.SetAttribute("LossRate", DoubleValue(0.1));
		clientHelper0.SetAttribute("PacketSize", UintegerValue(100));
		clientHelper0.SetAttribute("PacketsPerFrame", UintegerValue(100));
		clientHelper0.SetAttribute("BufferingSize", UintegerValue(1));
		clientHelper0.SetAttribute("PauseSize", UintegerValue(30));
		clientHelper0.SetAttribute("ResumeSize", UintegerValue(25));
		clientHelper0.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
		clientHelper0.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
		clientHelper0.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
		ApplicationContainer clientGlobalUserApp0 = clientHelper0.Install(globalUserNodes.Get(0));
		clientGlobalUserApp0.Start(Seconds(1.0));
		clientGlobalUserApp0.Stop(Seconds(5.0));
		clientGlobalUserApp0.Get(0)->TraceConnect("Rx", "Global User 0", MakeCallback(&vs::RxTime));

		StreamingClientHelper clientGlobalUserHelper1(lbv4Address, lbPort);
		clientGlobalUserHelper1.SetAttribute("LossRate", DoubleValue(0.1));
		clientGlobalUserHelper1.SetAttribute("PacketSize", UintegerValue(100));
		clientGlobalUserHelper1.SetAttribute("PacketsPerFrame", UintegerValue(100));
		clientGlobalUserHelper1.SetAttribute("BufferingSize", UintegerValue(1));
		clientGlobalUserHelper1.SetAttribute("PauseSize", UintegerValue(30));
		clientGlobalUserHelper1.SetAttribute("ResumeSize", UintegerValue(25));
		clientGlobalUserHelper1.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
		clientGlobalUserHelper1.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
		clientGlobalUserHelper1.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
		ApplicationContainer clientGlobalUserApp1 = clientGlobalUserHelper1.Install(globalUserNodes.Get(1));
		clientGlobalUserApp1.Start(Seconds(1.0));
		clientGlobalUserApp1.Stop(Seconds(5.0));
		clientGlobalUserApp1.Get(0)->TraceConnect("Rx", "Global User 1", MakeCallback(&vs::RxTime));

		StreamingClientHelper clientGlobalUserHelper2(lbv4Address, lbPort);
		clientGlobalUserHelper2.SetAttribute("LossRate", DoubleValue(0.1));
		clientGlobalUserHelper2.SetAttribute("PacketSize", UintegerValue(100));
		clientGlobalUserHelper2.SetAttribute("PacketsPerFrame", UintegerValue(100));
		clientGlobalUserHelper2.SetAttribute("BufferingSize", UintegerValue(1));
		clientGlobalUserHelper2.SetAttribute("PauseSize", UintegerValue(30));
		clientGlobalUserHelper2.SetAttribute("ResumeSize", UintegerValue(25));
		clientGlobalUserHelper2.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
		clientGlobalUserHelper2.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
		clientGlobalUserHelper2.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
		ApplicationContainer clientGlobalUserApp2 = clientGlobalUserHelper2.Install(globalUserNodes.Get(2));
		clientGlobalUserApp2.Start(Seconds(1.0));
		clientGlobalUserApp2.Stop(Seconds(5.0));
		clientGlobalUserApp2.Get(0)->TraceConnect("Rx", "Global User 2", MakeCallback(&vs::RxTime));
        */
       
		StreamingClientHelper clientCsmaHelper0(lbv4Address, lbPort);
		clientCsmaHelper0.SetAttribute("LossRate", DoubleValue(0.1));
		clientCsmaHelper0.SetAttribute("PacketSize", UintegerValue(100));
		clientCsmaHelper0.SetAttribute("PacketsPerFrame", UintegerValue(100));
		clientCsmaHelper0.SetAttribute("BufferingSize", UintegerValue(1));
		clientCsmaHelper0.SetAttribute("PauseSize", UintegerValue(30));
		clientCsmaHelper0.SetAttribute("ResumeSize", UintegerValue(25));
		clientCsmaHelper0.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
		clientCsmaHelper0.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
		clientCsmaHelper0.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
		ApplicationContainer clientCsmaApp0 = clientCsmaHelper0.Install(csmaNodes.Get(0));
		clientCsmaApp0.Start(Seconds(1.0));
		clientCsmaApp0.Stop(Seconds(5.0));
		clientCsmaApp0.Get(0)->TraceConnect("Rx", "CSMA 0", MakeCallback(&vs::RxTime));

		StreamingClientHelper clientCsmaHelper1(lbv4Address, lbPort);
		clientCsmaHelper1.SetAttribute("LossRate", DoubleValue(0.1));
		clientCsmaHelper1.SetAttribute("PacketSize", UintegerValue(100));
		clientCsmaHelper1.SetAttribute("PacketsPerFrame", UintegerValue(100));
		clientCsmaHelper1.SetAttribute("BufferingSize", UintegerValue(1));
		clientCsmaHelper1.SetAttribute("PauseSize", UintegerValue(30));
		clientCsmaHelper1.SetAttribute("ResumeSize", UintegerValue(25));
		clientCsmaHelper1.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
		clientCsmaHelper1.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
		clientCsmaHelper1.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
		ApplicationContainer clientCsmaApp1 = clientCsmaHelper1.Install(csmaNodes.Get(1));
		clientCsmaApp1.Start(Seconds(1.0));
		clientCsmaApp1.Stop(Seconds(5.0));
		clientCsmaApp1.Get(0)->TraceConnect("Rx", "CSMA 1", MakeCallback(&vs::RxTime));

		StreamingClientHelper clientCsmaHelper2(lbv4Address, lbPort);
		clientCsmaHelper2.SetAttribute("LossRate", DoubleValue(0.1));
		clientCsmaHelper2.SetAttribute("PacketSize", UintegerValue(100));
		clientCsmaHelper2.SetAttribute("PacketsPerFrame", UintegerValue(100));
		clientCsmaHelper2.SetAttribute("BufferingSize", UintegerValue(1));
		clientCsmaHelper2.SetAttribute("PauseSize", UintegerValue(30));
		clientCsmaHelper2.SetAttribute("ResumeSize", UintegerValue(25));
		clientCsmaHelper2.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
		clientCsmaHelper2.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
		clientCsmaHelper2.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
		ApplicationContainer clientCsmaApp2 = clientCsmaHelper2.Install(csmaNodes.Get(2));
		clientCsmaApp2.Start(Seconds(1.0));
		clientCsmaApp2.Stop(Seconds(5.0));
		clientCsmaApp2.Get(0)->TraceConnect("Rx", "CSMA 2", MakeCallback(&vs::RxTime));
       
       /* 
		StreamingClientHelper clientWifiHelper0(lbv4Address, lbPort);
		clientWifiHelper0.SetAttribute("LossRate", DoubleValue(0.1));
		clientWifiHelper0.SetAttribute("PacketSize", UintegerValue(100));
		clientWifiHelper0.SetAttribute("PacketsPerFrame", UintegerValue(100));
		clientWifiHelper0.SetAttribute("BufferingSize", UintegerValue(1));
		clientWifiHelper0.SetAttribute("PauseSize", UintegerValue(30));
		clientWifiHelper0.SetAttribute("ResumeSize", UintegerValue(25));
		clientWifiHelper0.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
		clientWifiHelper0.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
		clientWifiHelper0.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
		ApplicationContainer clientWifiApp0 = clientWifiHelper0.Install(wifiStaNode.Get(0));
		clientWifiApp0.Start(Seconds(1.0));
		clientWifiApp0.Stop(Seconds(5.0));
		clientWifiApp0.Get(0)->TraceConnect("Rx", "Wifi 0", MakeCallback(&vs::RxTime));

		StreamingClientHelper clientWifiHelper1(lbv4Address, lbPort);
		clientWifiHelper1.SetAttribute("LossRate", DoubleValue(0.1));
		clientWifiHelper1.SetAttribute("PacketSize", UintegerValue(100));
		clientWifiHelper1.SetAttribute("PacketsPerFrame", UintegerValue(100));
		clientWifiHelper1.SetAttribute("BufferingSize", UintegerValue(1));
		clientWifiHelper1.SetAttribute("PauseSize", UintegerValue(30));
		clientWifiHelper1.SetAttribute("ResumeSize", UintegerValue(25));
		clientWifiHelper1.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
		clientWifiHelper1.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
		clientWifiHelper1.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
		ApplicationContainer clientWifiApp1 = clientWifiHelper1.Install(wifiStaNode.Get(1));
		clientWifiApp1.Start(Seconds(1.0));
		clientWifiApp1.Stop(Seconds(5.0));
		clientWifiApp1.Get(0)->TraceConnect("Rx", "Wifi 1", MakeCallback(&vs::RxTime));

		StreamingClientHelper clientWifiHelper2(lbv4Address, lbPort);
		clientWifiHelper2.SetAttribute("LossRate", DoubleValue(0.1));
		clientWifiHelper2.SetAttribute("PacketSize", UintegerValue(100));
		clientWifiHelper2.SetAttribute("PacketsPerFrame", UintegerValue(100));
		clientWifiHelper2.SetAttribute("BufferingSize", UintegerValue(1));
		clientWifiHelper2.SetAttribute("PauseSize", UintegerValue(30));
		clientWifiHelper2.SetAttribute("ResumeSize", UintegerValue(25));
		clientWifiHelper2.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));
		clientWifiHelper2.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.)));
		clientWifiHelper2.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));
		ApplicationContainer clientWifiApp2 = clientWifiHelper2.Install(wifiStaNode.Get(2));
		clientWifiApp2.Start(Seconds(1.0));
		clientWifiApp2.Stop(Seconds(5.0));
		clientWifiApp2.Get(0)->TraceConnect("Rx", "Wifi 2", MakeCallback(&vs::RxTime));
       */
	}


	// Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	
	
	Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("video_strreamer.routes", std::ios::out);
	Ipv4GlobalRoutingHelper globalRouting;
	globalRouting.PopulateRoutingTables();
	globalRouting.PrintRoutingTableAllAt (Seconds (0.1), routingStream);

	for (uint32_t i = 0; i < numOfSwitches; i++)
	{
		Ipv4Address left_add = Ipv4Address::ConvertFrom(gs_interfaceLeft.GetAddress(i));
		Ipv4Address right_add = Ipv4Address::ConvertFrom(gs_interfaceRight.GetAddress((i+3) % 4));
		NS_LOG_INFO( i << "th global swith address left: " << left_add << " right: " << right_add);
	}

	for (uint32_t i = 0; i < numOfStream; i++)
	{
		Ipv4Address hub_add = Ipv4Address::ConvertFrom(streamHub_interfaces.GetAddress(i));
		Ipv4Address own_add = Ipv4Address::ConvertFrom(stream_interfaces.GetAddress(i));
		NS_LOG_INFO( i << "th stream server address hub: " << hub_add << " own: " << own_add);
	}

	{
		Ipv4Address hub_add = Ipv4Address::ConvertFrom(lbHub_interfaces.GetAddress(0));
		Ipv4Address own_add = Ipv4Address::ConvertFrom(lb_interfaces.GetAddress(0));
		NS_LOG_INFO( "balance server address hub: " << hub_add << " own: " << own_add);
	}

	for (uint32_t i = 0; i < numOfGlobalUsers; i++)
	{
		Ipv4Address hub_add = Ipv4Address::ConvertFrom(guHub_interfaces.GetAddress(i));
		Ipv4Address own_add = Ipv4Address::ConvertFrom(gu_interfaces.GetAddress(i));
		NS_LOG_INFO( i << "th global user address hub: " << hub_add << " own: " << own_add);
	}

	{
		Ipv4Address own_add = Ipv4Address::ConvertFrom(csma_interfaces.GetAddress(0));
		NS_LOG_INFO(" csma address hub: " << own_add);
	}

	for (uint32_t i = 0; i < numOfCsma; i++)
	{
		Ipv4Address own_add = Ipv4Address::ConvertFrom(csma_interfaces.GetAddress(i+1));
		NS_LOG_INFO( i << "th csma user address " << " own: " << own_add);
	}

	{
		Ipv4Address own_add = Ipv4Address::ConvertFrom(wifiApOutHub_interfaces.GetAddress(0));
		NS_LOG_INFO(" wifi AP out Hub address hub: " << own_add);

		own_add = Ipv4Address::ConvertFrom(wifiApOut_interfaces.GetAddress(0));
		NS_LOG_INFO(" wifi AP out address hub: " << own_add);

		own_add = Ipv4Address::ConvertFrom(Ap_interface.GetAddress(0));
		NS_LOG_INFO(" wifi AP address hub: " << own_add);
	}

	for (uint32_t i = 0; i < numOfWifiUsers; i++)
	{
		Ipv4Address own_add = Ipv4Address::ConvertFrom(Sta_interface.GetAddress(i));
		NS_LOG_INFO( i << "th wifi user address " << " own: " << own_add);
	}

	{
		NodeContainer tmp;
		tmp.Add(globalSwitches.Get(0));
		gsP2p.EnablePcap("GS0", tmp);
	}

	{
		NodeContainer tmp;
		tmp.Add(globalSwitches.Get(2));
		gsP2p.EnablePcap("GS2", tmp);
	}
	

	Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    Simulator::Destroy();

	// uint32_t totalPacketsRecv = DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
    // double throughtput = totalPacketsRecv * payloadSize * 8 / (simulationTime * 1000000.0);
    // std::cout << "Throughput: " << throughtput << " Mbps" << "\n";


	return 0;
}
