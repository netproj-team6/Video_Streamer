#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-grid.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/wifi-module.h"

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("week4");

int main(int argc, char *argv[])
{

	double LossRate = 0.;
	double DownRate = 0.0;
	double ErrorRate = 0.00001;
	uint32_t payloadSize = 1472; // bytes
	uint64_t simulationTime = 3;
    uint32_t nMpdu = 100;

	uint32_t numOfSwitches = 4;
	uint32_t numOfStream= 3;
	uint32_t numOfBalances= 2;
	uint32_t numOfGlobalUsers= 2;
	uint32_t numOfWifiUsers= 2;

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
	balanceServerNodes.Create(2);
	receptionServerNode.Create(1);

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
	//////////////////// inner 
    NodeContainer wifiApNode;
    wifiApNode.Create(1);
	NodeContainer wifiStaNode;
    wifiStaNode.Create(numOfWifiUsers);

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

	NetDeviceContainer staDevice;
    staDevice = wifi.Install(phy, mac, wifiStaNode);

    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid),
                "BeaconInterval", TimeValue(MicroSeconds(102400)),
                "BeaconGeneration", BooleanValue(true));

    NetDeviceContainer apDevice;
    apDevice = wifi.Install(phy, mac, wifiApNode);

	//////////////////// outter 
	NetDeviceContainer devices 




	///// 2. Install internet stack
	InternetStackHelper stack;
	stack.Install(globalSwitches);
	stack.Install(streamServerNodes);
	stack.Install(receptionServerNode);
	stack.Install(balanceServerNodes);
	stack.Install(globalUserNodes);
	stack.Install(wifiApNode);
    stack.Install(wifiStaNode);



	///// 3. Assign address
	Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

	////////// global switches 
	Ipv4InterfaceContainer interfacesGS_Left;
	Ipv4InterfaceContainer interfacesGS_Right;
	Ipv4InterfaceContainer interfacesGS_Cross;
	for (uint32_t i = 0; i < numOfSwitches; i++)
	{
		interfacesGS_Left.Add(address.Assign(GS_LeftDevices.Get(i)));
		interfacesGS_Right.Add(address.Assign(GS_RightDevices.Get(i)));
		address.NewNetwork();
	}
	for (uint32_t i = 0; i < GS_CrossDevices.GetN(); i++)
	{
		interfacesGS_Cross.Add(address.Assign(GS_CrossDevices.Get(i)));
		address.NewNetwork();
	}

	////////// Server
	Ipv4InterfaceContainer interfacesStreamHub;
	Ipv4InterfaceContainer interfacesStream;
	for (uint32_t i = 0; i < StreamDevices.GetN(); i++)
	{
		interfacesStreamHub.Add(address.Assign(StreamHubDevices));
		interfacesStream.Add(address.Assign(StreamDevices.Get(i)));
		address.NewNetwork();
	}

	Ipv4InterfaceContainer interfacesReceptionGlobal;
	interfacesStreamHub.Add(address.Assign(StreamHubDevices));
	interfacesReceptionGlobal.Add(address.Assign(ReceptionGlobalDevices));
	address.NewNetwork();


	Ipv4AddressHelper addressBalance;
    addressBalance.SetBase("192.168.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfacesReceptionVirtual;
	Ipv4InterfaceContainer interfacesBalances;
	for (uint32_t i = 0; i < BalancesDevices.GetN(); i++)
	{
		interfacesReceptionVirtual.Add(addressBalance.Assign(ReceptionDevices.Get(i)));
		interfacesBalances.Add(addressBalance.Assign(BalancesDevices.Get(i)));
		address.NewNetwork();
	}


	////////// Global Users
	Ipv4InterfaceContainer interfacesGlobalUserHub;
	Ipv4InterfaceContainer interfacesGlobalUsers;
	for (uint32_t i = 0; i < GlobalUsersDevices.GetN(); i++)
	{
		interfacesGlobalUserHub.Add(address.Assign(GlobalUserHubDevices));
		interfacesGlobalUsers.Add(address.Assign(GlobalUsersDevices.Get(i)));
		address.NewNetwork();
	}	


	////////// Wifi group




	///// 4. Set up applications
	auto server_node = globalSwitches.Get(0);
	auto server_ip = interfacesGS_Left.GetAddress(0);
	auto client_node = globalUserNodes.Get(0);

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

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    Simulator::Destroy();

	uint32_t totalPacketsRecv = DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
    double throughtput = totalPacketsRecv * payloadSize * 8 / (simulationTime * 1000000.0);
    std::cout << "Throughput: " << throughtput << " Mbps" << "\n";


	// nodes
	// ,
	// , wifiStaNode, csmaSwitchNode;

	// // Channels 


	// // Net Device 


	// // Stack 

	// // address 


	// // Server

	// // Client 


	// // Call back
	// NodeContainer nodes;
	// nodes.Create(2);
	

	// std::string my_data_rate, my_delay;
	// CommandLine cmd;
	// cmd.AddValue("DataRate", "Link Data rate", my_data_rate);
	// cmd.AddValue("Delay", "Link Delay", my_delay);
	// cmd.Parse(argc, argv);

	// PointToPointHelper p2p;
	// p2p.SetDeviceAttribute("DataRate", StringValue(my_data_rate));
	// p2p.SetChannelAttribute("Delay", StringValue(my_delay));

	// NetDeviceContainer devices;
	// devices = p2p.Install(nodes);

	// InternetStackHelper stack;
	// stack.Install(nodes);


	// Ipv4AddressHelper addr;
	// addr.SetBase("10.1.1.0", "255.255.255.0");
	// Ipv4InterfaceContainer interfaces = addr.Assign(devices);


	// UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
	// echoClient.SetAttribute("MaxPackets", UintegerValue(15000));
	// echoClient.SetAttribute("Interval", TimeValue(Seconds(0.001)));
	// echoClient.SetAttribute("PacketSize", UintegerValue(1024));

	// ApplicationContainer clientApps;
	// clientApps.Add(echoClient.Install(nodes.Get(0)));
	// clientApps.Start(Seconds(1.0));
	// clientApps.Stop(Seconds(10.0));

	// UdpEchoServerHelper echoServer(9);
	// ApplicationContainer serverApps(echoServer.Install(nodes.Get(1)));
	// serverApps.Start(Seconds(1.0));
	// serverApps.Stop(Seconds(10.0));

	// p2p.EnablePcapAll("2019314505");
	// Simulator::Run();
	// Simulator::Stop(Seconds(12.0));
	// Simulator::Destroy();

	return 0;

}
