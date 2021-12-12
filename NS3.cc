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
				"MinX",DoubleValue(25.0),
				"MinY",DoubleValue(0.0),
				"DeltaX",DoubleValue(0.0),
				"DeltaY",DoubleValue(50.0),
				"GridWidth",UintegerValue(1),
				"LayoutType",StringValue("RowFirst"));
	m1.Install(endpoint);
	
	PointToPointHelper ptp;
	
	NetDeviceContainer devices;
	devices=ptp.Install(endpoint);
	
	NodeContainer nodes;
	nodes.Create(30);
	MobilityHelper m;
	m.SetPositionAllocator("ns3::GridPositionAllocator",
				"MinX",DoubleValue(10.0),
				"MinY",DoubleValue(10.0),
				"DeltaX",DoubleValue(5.0),
				"DeltaY",DoubleValue(10.0),
				"GridWidth",UintegerValue(10),
				"LayoutType",StringValue("RowFirst"));
	m.Install(nodes);
	
	Simulator::Stop(Seconds(15.0));
	AnimationInterface anim("NS3.xml");
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
