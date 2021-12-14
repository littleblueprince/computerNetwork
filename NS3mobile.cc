#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AdHocNetwork");
int main (int argc, char *argv[]){
	 // 激活日志组件
	LogComponentEnable("AdHocNetwork", LOG_LEVEL_INFO);
    	LogComponentEnable("PacketSink", LOG_LEVEL_ALL);

	NodeContainer endpoint;
	endpoint.Create(2);
	MobilityHelper m1;
	m1.SetPositionAllocator("ns3::GridPositionAllocator",
				"MinX",DoubleValue(0.0),
				"MinY",DoubleValue(0.0),
				"DeltaX",DoubleValue(0.0),
				"DeltaY",DoubleValue(50.0),
				"GridWidth",UintegerValue(1),
				"LayoutType",StringValue("RowFirst"));
	m1.Install(endpoint);
	
	//PointToPointHelper ptp;
	
	//NetDeviceContainer devices;
	//devices=ptp.Install(endpoint);
	
	NodeContainer nodes;
	nodes.Create(27);
	MobilityHelper m;
	m.SetPositionAllocator("ns3::GridPositionAllocator",
				"MinX",DoubleValue(-40.0),
				"MinY",DoubleValue(10.0),
				"DeltaX",DoubleValue(10.0),
				"DeltaY",DoubleValue(10.0),
				"GridWidth",UintegerValue(9),
				"LayoutType",StringValue("RowFirst"));
	m.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
		"Bounds",RectangleValue(Rectangle(-500,500,0,50)));
	m.Install(nodes);


	YansWifiChannelHelper channel = YansWifiChannelHelper::Default();   //使用默认的信道模型
	YansWifiPhyHelper phy;  //使用默认的PHY模型
	phy.SetChannel(channel.Create());  //创建通道对象并把他关联到物理层对象管理器

	WifiHelper wifi;
	wifi.SetRemoteStationManager("ns3::AarfWifiManager");  //设置wifi助手用AARF速率控制算法 

	//配置Mac类型以及基础设置SSID
	WifiMacHelper mac;
	Ssid ssid = Ssid("ns-3-aqiao");  //设置SSID的名字为ns-3-aqiao
	mac.SetType("ns3::StaWifiMac",    //指定mac层指定为ns3::StaWifiMac
		"Ssid", SsidValue(ssid),   //设置默认AP是ssid对象
		"ActiveProbing", BooleanValue(false));  //设置不会发出主动探测AP的指令，默认AP是ssid

	NetDeviceContainer staDevices;
	staDevices = wifi.Install(phy, mac, nodes);  //在MAC层和PHY层用以前安装方法来创建这些无线设备

	mac.SetType("ns3::ApWifiMac",   //指定为AP
		"Ssid", SsidValue(ssid));

	NetDeviceContainer apDevices;
	apDevices = wifi.Install(phy, mac, endpoint);   //一起共享PHY层的属性

	InternetStackHelper stack;
	stack.Install(endpoint);
	stack.Install(nodes);   //安装协议栈

	Ipv4AddressHelper address;

	address.SetBase("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer wifiInterfaces;
	wifiInterfaces = address.Assign(staDevices);
	address.Assign(apDevices);                 //地址分配

	//安装暗涌程序，使用UDP，与之前的UDP的使用方法一致
	UdpEchoServerHelper echoServer(9);

	ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
	serverApps.Start(Seconds(1.0));
	serverApps.Stop(Seconds(10.0));

	UdpEchoClientHelper echoClient(wifiInterfaces.GetAddress(0), 9);
	echoClient.SetAttribute("MaxPackets", UintegerValue(5));
	echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
	echoClient.SetAttribute("PacketSize", UintegerValue(1024));

	ApplicationContainer clientApps =
		echoClient.Install(nodes.Get(26));
	clientApps.Start(Seconds(2.0));
	clientApps.Stop(Seconds(10.0));

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();


	
	Simulator::Stop(Seconds(120.0));
	phy.EnablePcap("wifiwifi", apDevices.Get(0));
	AnimationInterface anim("NS3.xml");
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
