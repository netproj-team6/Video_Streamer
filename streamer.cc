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

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("skku_chat");

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


int main(int argc, char *argv[])
{

	double LossRate = 0.;
	double DownRate = 0.0;
	double ErrorRate = 0.00001;
	uint32_t payloadSize = 1472; // bytes
	uint64_t simulationTime = 1;
    uint32_t nMpdu = 10;

	uint32_t numOfSwitches = 4;
	uint32_t numOfStream= 3;
	uint32_t numOfGlobalUsers= 3;
	uint32_t numOfWifiUsers= 1;
	uint32_t numOfCsma= 3;

	LogComponentEnable("LoadBalancer", LOG_LEVEL_ALL);

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

	PointToPointHelper p2pGS;
	p2pGS.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
	p2pGS.SetChannelAttribute("Delay", StringValue("10ms"));

	NetDeviceContainer GS_LeftDevices;
	NetDeviceContainer GS_RightDevices;
	NetDeviceContainer GS_CrossDevices;
	for (uint32_t i = 0; i < numOfSwitches; i++)
	{
		NetDeviceContainer nd = p2pGS.Install(globalSwitches.Get(i % numOfSwitches), globalSwitches.Get((i + 1) % numOfSwitches));
		GS_LeftDevices.Add(nd.Get(0));
		GS_RightDevices.Add(nd.Get(1));
	}
	
	for (uint32_t i = 0; i < numOfSwitches / 2; i++)
	{
		NetDeviceContainer nd = p2pGS.Install(globalSwitches.Get(i % numOfSwitches), globalSwitches.Get((i + 2) % numOfSwitches));
		GS_CrossDevices.Add(nd.Get(0));
		GS_CrossDevices.Add(nd.Get(1));
	}


	// Define an error model
    // Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    // em->SetAttribute("ErrorRate", DoubleValue(ErrorRate));
	// for (uint32_t i = 0; i < 4; i++)
	// {
	// 	devicesGS.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
	// }
	// std::vector<>

	////////// Server
	NodeContainer streamServerNodes, balanceServerNode;
	streamServerNodes.Create(3);
	balanceServerNode.Create(1);

	PointToPointHelper p2pServer;
	p2pServer.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
	p2pServer.SetChannelAttribute("Delay", StringValue("2ms"));

	NetDeviceContainer StreamHubDevices;
	NetDeviceContainer StreamDevices;
	for (uint32_t i = 0; i < numOfStream; i++)
	{
		NetDeviceContainer nd = p2pServer.Install(globalSwitches.Get(0), streamServerNodes.Get(i));
		StreamHubDevices.Add(nd.Get(0));
		StreamDevices.Add(nd.Get(1));
	}

	NetDeviceContainer balanceDevices;
	{
		NetDeviceContainer nd = p2pServer.Install(globalSwitches.Get(0), balanceServerNode.Get(0));
		StreamHubDevices.Add(nd.Get(0));
		balanceDevices.Add(nd.Get(1));
	}


	////////// Global Users
	NodeContainer globalUserNodes;
	globalUserNodes.Create(numOfGlobalUsers);

	PointToPointHelper p2pGlobalUser;
	p2pGlobalUser.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
	p2pGlobalUser.SetChannelAttribute("Delay", StringValue("20ms"));

	NetDeviceContainer GlobalUserHubDevices;
	NetDeviceContainer GlobalUsersDevices;
	for (uint32_t i = 0; i < numOfGlobalUsers; i++)
	{
		NetDeviceContainer nd = p2pGlobalUser.Install(globalSwitches.Get(2), globalUserNodes.Get(i));
		GlobalUserHubDevices.Add(nd.Get(0));
		GlobalUsersDevices.Add(nd.Get(1));
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
	PointToPointHelper p2pWifi;
	p2pWifi.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
	p2pWifi.SetChannelAttribute("Delay", StringValue("5ms"));

	NetDeviceContainer WifiHubDevice;
	NetDeviceContainer WifiGlobalDevice;
	{
		NetDeviceContainer nd = p2pWifi.Install(globalSwitches.Get(1), wifiApNode.Get(0));
		WifiHubDevice.Add(nd.Get(0));
		WifiGlobalDevice.Add(nd.Get(1));
	}


	////////// CSMA
	//////////////////// inner
	NodeContainer csmaNodes;	
	csmaNodes.Create(numOfCsma);

	NodeContainer csma_GS_Nodes;
	csma_GS_Nodes.Add(globalSwitches.Get(3));
	csma_GS_Nodes.Add(csmaNodes);
	

	CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

	NetDeviceContainer csma_GS_Devices;
	csma_GS_Devices = csma.Install(csma_GS_Nodes);

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
	Ipv4InterfaceContainer interfacesGS_Left;
	Ipv4InterfaceContainer interfacesGS_Right;
	Ipv4InterfaceContainer interfacesGS_Cross;
	for (uint32_t i = 0; i < numOfSwitches; i++)
	{
		interfacesGS_Left.Add(GlobalAddress.Assign(GS_LeftDevices.Get(i)));
		interfacesGS_Right.Add(GlobalAddress.Assign(GS_RightDevices.Get(i)));
		GlobalAddress.NewAddress();
	}
	for (uint32_t i = 0; i < (numOfSwitches / 2); i++)
	{
		interfacesGS_Cross.Add(GlobalAddress.Assign(GS_CrossDevices.Get(i*2)));
		interfacesGS_Cross.Add(GlobalAddress.Assign(GS_CrossDevices.Get(i*2+1)));
		GlobalAddress.NewAddress();
	}

	////////// Server
	Ipv4InterfaceContainer streamHub_interfaces;
	Ipv4InterfaceContainer Stream_interfaces;
	for (uint32_t i = 0; i < StreamDevices.GetN(); i++)
	{
		streamHub_interfaces.Add(GlobalAddress.Assign(StreamHubDevices));
		Stream_interfaces.Add(GlobalAddress.Assign(StreamDevices.Get(i)));
		GlobalAddress.NewAddress();
	}
	
	Ipv4InterfaceContainer balance_interfaces;
	streamHub_interfaces.Add(GlobalAddress.Assign(StreamHubDevices));
	balance_interfaces.Add(GlobalAddress.Assign(balanceDevices));
	GlobalAddress.NewAddress();

		

	
	
	////////// Wifi group
	//////////////////// inner
	Ipv4AddressHelper wifiAddress;
    wifiAddress.SetBase("192.168.2.0", "255.255.255.0");
    Ipv4InterfaceContainer Sta_interface;
	Sta_interface = wifiAddress.Assign(wifiStaDevice);
    Ipv4InterfaceContainer Ap_interface;
    Ap_interface = wifiAddress.Assign(wifiApDevice);
	

	MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

	positionAlloc->Add(Vector(0.0, 0.0, 0.0));
	for (uint32_t i = 0; i < numOfWifiUsers; i++)
	{
		positionAlloc->Add(Vector(1.0 * (double)(i+1), 0.0, 0.0));
	}
    mobility.SetPositionAllocator(positionAlloc);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    mobility.Install(wifiApNode);
    mobility.Install(wifiStaNode);

	//////////////////// outer
	
	Ipv4InterfaceContainer WifiHub_interfaces;
	WifiHub_interfaces.Add(GlobalAddress.Assign(WifiHubDevice));
	WifiHub_interfaces.Add(GlobalAddress.Assign(WifiGlobalDevice));
	GlobalAddress.NewAddress();


	////////// CSMA
	//////////////////// inner
	Ipv4AddressHelper csmaAddress;
    csmaAddress.SetBase("192.168.3.0", "255.255.255.0");

	Ipv4InterfaceContainer csma_GS_interfaces;
	csma_GS_interfaces = csmaAddress.Assign(csma_GS_Devices);

		////////// Global Users
	GlobalAddress.NewNetwork();
	Ipv4InterfaceContainer globalUserHub_interfaces;
	Ipv4InterfaceContainer globalUsers_interfaces;
	for (uint32_t i = 0; i < GlobalUsersDevices.GetN(); i++)
	{
		globalUserHub_interfaces.Add(GlobalAddress.Assign(GlobalUserHubDevices.Get(i)));
		globalUsers_interfaces.Add(GlobalAddress.Assign(GlobalUsersDevices.Get(i)));
		GlobalAddress.NewAddress();
	}	

	///// 4. Set up applications
	auto server_node = globalUserNodes.Get(1);
	auto server_ip = globalUsers_interfaces.GetAddress(1);
	auto client_node = balanceServerNode.Get(0);

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

	// ////////////////////////////////////////////////////////

	uint16_t lbPort = 1010;
	uint16_t dstPort = 2020;
	Address streamAddress0(InetSocketAddress(Stream_interfaces.GetAddress(0), dstPort));
    Address streamAddress1(InetSocketAddress(Stream_interfaces.GetAddress(1), dstPort));
    Address streamAddress2(InetSocketAddress(Stream_interfaces.GetAddress(2), dstPort));

    LoadBalancerHelper lb;
    lb.SetAttribute("Port", UintegerValue(lbPort));
    lb.SetAttribute("FirstAddress", AddressValue(streamAddress0));
    lb.SetAttribute("FirstWeight", UintegerValue(1));
    lb.SetAttribute("SecondAddress", AddressValue(streamAddress1));
    lb.SetAttribute("SecondWeight", UintegerValue(1));
    lb.SetAttribute("ThirdAddress", AddressValue(streamAddress2));
    lb.SetAttribute("ThirdWeight", UintegerValue(1));
    ApplicationContainer lbApp = lb.Install(balanceServerNode);
    lbApp.Start(Seconds(1.0));
    lbApp.Stop(Seconds(simulationTime + 1));
    lbApp.Get(0)->TraceConnect("Rx", "LB", MakeCallback(&RxTime));

	// Address clientAddress0(InetSocketAddress(globalUsers_interfaces.GetAddress(0), lbPort));
    // Address clientAddress1(InetSocketAddress(globalUsers_interfaces.GetAddress(1), lbPort));
    // Address clientAddress2(InetSocketAddress(globalUsers_interfaces.GetAddress(1), lbPort));

	Address lbAddress(InetSocketAddress(balance_interfaces.GetAddress(0), lbPort));

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
    ApplicationContainer dst0App = dst0.Install(streamServerNodes.Get(0));
    dst0App.Start(Seconds(1.0));
    dst0App.Stop(Seconds(5.0));
    dst0App.Get(0)->TraceConnect("Rx", "DST0", MakeCallback(&RxTime));

    PacketSinkHelper dst1("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), dstPort));
    ApplicationContainer dst1App = dst1.Install(streamServerNodes.Get(1));
    dst1App.Start(Seconds(1.0));
    dst1App.Stop(Seconds(5.0));
    dst1App.Get(0)->TraceConnect("Rx", "DST1", MakeCallback(&RxTime));

    PacketSinkHelper dst2("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), dstPort));
    ApplicationContainer dst2App = dst2.Install(streamServerNodes.Get(2));
    dst2App.Start(Seconds(1.0));
    dst2App.Stop(Seconds(5.0));
    dst2App.Get(0)->TraceConnect("Rx", "DST2", MakeCallback(&RxTime));


	// Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	
	
	Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("skku_chat.routes", std::ios::out);
	Ipv4GlobalRoutingHelper globalRouting;
	globalRouting.PopulateRoutingTables();
	globalRouting.PrintRoutingTableAllAt (Seconds (0.1), routingStream);


	Simulator::Stop(Seconds(simulationTime + 2));
    Simulator::Run();
    Simulator::Destroy();

	uint32_t totalPacketsRecv = DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
    double throughtput = totalPacketsRecv * payloadSize * 8 / (simulationTime * 1000000.0);
    std::cout << "Throughput: " << throughtput << " Mbps" << "\n";


	return 0;
}
