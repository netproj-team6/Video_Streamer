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

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("skku_chat");

int main(int argc, char *argv[])
{

	double LossRate = 0.;
	double DownRate = 0.0;
	double ErrorRate = 0.00001;
	uint32_t payloadSize = 1472; // bytes
	uint64_t simulationTime = 3;
    uint32_t nMpdu = 10;

	uint32_t numOfSwitches = 4;
	uint32_t numOfStream= 3;
	uint32_t numOfBalances= 2;
	uint32_t numOfGlobalUsers= 2;
	uint32_t numOfWifiUsers= 1;
	uint32_t numOfCsma= 3;

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
	NodeContainer streamServerNodes, receptionServerNode, balanceServerNodes;
	streamServerNodes.Create(3);
	receptionServerNode.Create(1);
	balanceServerNodes.Create(2);

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

	NetDeviceContainer ReceptionGlobalDevices;
	{
		NetDeviceContainer nd = p2pServer.Install(globalSwitches.Get(0), receptionServerNode.Get(0));
		StreamHubDevices.Add(nd.Get(0));
		ReceptionGlobalDevices.Add(nd.Get(1));
	}

	NetDeviceContainer ReceptionDevices;
	NetDeviceContainer BalancesDevices;
	for (uint32_t i = 0; i < numOfBalances; i++)
	{
		NetDeviceContainer nd = p2pServer.Install(receptionServerNode.Get(0), balanceServerNodes.Get(i));
		ReceptionDevices.Add(nd.Get(0));
		BalancesDevices.Add(nd.Get(1));
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
	// NetDeviceContainer csmaHubDevices;
	// csmaHubDevices = csma.Install(globalSwitches.Get(3));

	//////////////////// outer
	// PointToPointHelper p2pCsma;
	// p2pCsma.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
	// p2pCsma.SetChannelAttribute("Delay", StringValue("5ms"));

	
	// csmaHubDevices = p2pCsma.Install(globalSwitches.Get(3), csmaNodes.Get(0));



	///// 2. Install internet stack
	InternetStackHelper stack;
	stack.Install(globalSwitches.Get(0));
	stack.Install(globalSwitches.Get(1));
	stack.Install(globalSwitches.Get(2));
	stack.Install(streamServerNodes);
	stack.Install(receptionServerNode);
	stack.Install(balanceServerNodes);
	stack.Install(globalUserNodes);
	stack.Install(wifiApNode);
    stack.Install(wifiStaNode);
	stack.Install(csma_GS_Nodes);
	// stack.InstallAll();

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
	
	Ipv4InterfaceContainer receptionGlobal_interfaces;
	streamHub_interfaces.Add(GlobalAddress.Assign(StreamHubDevices));
	receptionGlobal_interfaces.Add(GlobalAddress.Assign(ReceptionGlobalDevices));
	GlobalAddress.NewAddress();

	
	Ipv4AddressHelper balanceAddress;
    balanceAddress.SetBase("192.168.1.0", "255.255.255.0");
	Ipv4InterfaceContainer receptionVirtual_interfaces;
	Ipv4InterfaceContainer balances_interfaces;
	for (uint32_t i = 0; i < numOfBalances; i++)
	{
		receptionVirtual_interfaces.Add(balanceAddress.Assign(ReceptionDevices.Get(i)));
		balances_interfaces.Add(balanceAddress.Assign(BalancesDevices.Get(i)));
		balanceAddress.NewAddress();
	}

	
	////////// Global Users
	Ipv4InterfaceContainer globalUserHub_interfaces;
	Ipv4InterfaceContainer globalUsers_interfaces;
	for (uint32_t i = 0; i < GlobalUsersDevices.GetN(); i++)
	{
		globalUserHub_interfaces.Add(GlobalAddress.Assign(GlobalUserHubDevices.Get(i)));
		globalUsers_interfaces.Add(GlobalAddress.Assign(GlobalUsersDevices.Get(i)));
		GlobalAddress.NewAddress();
	}	
	
	
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
	// Ipv4InterfaceContainer csmaHub_interfaces;
	// csmaHub_interfaces.Add(csmaAddress.Assign(csmaHubDevices));
	// for (uint32_t i = 0; i < csmaHub_interfaces.GetN(); i++)
	// {
	// 	std::cout << csmaHub_interfaces.GetAddress(i) << std::endl;
	// }
	csma_GS_interfaces = csmaAddress.Assign(csma_GS_Devices);
	// for (uint32_t i = 0; i < csma_GS_interfaces.GetN(); i++)
	// {
	// 	std::cout << csma_GS_interfaces.GetAddress(i) << std::endl;
	// }


	// Ipv4InterfaceContainer csma_interfaces;
	// {
	// 	NetDeviceContainer tmp;
	// 	tmp.Add(csmaHubDevices);
	// 	tmp.Add(csmaDevices);
	// 	csma_interfaces = csmaAddress.Assign(tmp);
	// }

	// for (uint32_t i = 0; i < csma_interfaces.GetN(); i++)
	// {
	// 	std::cout << csma_interfaces.GetAddress(i) << std::endl;
	// }

	//////////////////// outer
	// Ipv4InterfaceContainer csmaHub_interfaces;
	// csmaHub_interfaces = (GlobalAddress.Assign(csmaHubDevices));
	// // GlobalAddress.NewAddress();
	// for (uint32_t i = 0; i < csmaHub_interfaces.GetN(); i++)
	// {
	// 	std::cout << csmaHub_interfaces.GetAddress(i) << std::endl;
	// }

	///// 4. Set up applications
	auto server_node = csma_GS_Nodes.Get(1);
	auto server_ip = csma_GS_interfaces.GetAddress(1);
	auto client_node = globalUserNodes.Get(1);

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
	// Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	
	Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("skku_chat.routes", std::ios::out);
	Ipv4GlobalRoutingHelper globalRouting;
	globalRouting.PopulateRoutingTables();
	globalRouting.PrintRoutingTableAllAt (Seconds (0.1), routingStream);


	Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    Simulator::Destroy();

	uint32_t totalPacketsRecv = DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
    double throughtput = totalPacketsRecv * payloadSize * 8 / (simulationTime * 1000000.0);
    std::cout << "Throughput: " << throughtput << " Mbps" << "\n";


	return 0;
}
