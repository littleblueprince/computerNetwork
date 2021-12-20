/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 The Boeing Company
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

 // This script configures two nodes on an 802.11b physical layer, with
 // 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
 // (application) bytes to the other node.  The physical layer is configured
 // to receive at a fixed RSS (regardless of the distance and transmit
 // power); therefore, changing position of the nodes has no effect.
 //
 // There are a number of command-line options available to control
 // the default behavior.  The list of available command-line options
 // can be listed with the following command:
 // ./waf --run "wifi-simple-adhoc --help"
 //
 // For instance, for this configuration, the physical layer will
 // stop successfully receiving packets when rss drops below -97 dBm.
 // To see this effect, try running:
 //
 // ./waf --run "wifi-simple-adhoc --rss=-97 --numPackets=20"
 // ./waf --run "wifi-simple-adhoc --rss=-98 --numPackets=20"
 // ./waf --run "wifi-simple-adhoc --rss=-99 --numPackets=20"
 //
 // Note that all ns-3 attributes (not just the ones exposed in the below
 // script) can be changed at command line; see the documentation.
 //
 // This script can also be helpful to put the Wifi layer into verbose
 // logging mode; this command will turn on all wifi logging:
 //
 // ./waf --run "wifi-simple-adhoc --verbose=1"
 //
 // When you are done, you will notice two pcap trace files in your directory.
 // If you have tcpdump installed, you can try this:
 //
 // tcpdump -r wifi-simple-adhoc-0-0.pcap -nn -tt
 //

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include<unistd.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiSimpleAdhoc");
int missing = 0;
const int range = 15;
const int nodenumber = 11;
double rss = -80;  // -dBm
uint32_t packetSize = 1000; // bytes
uint32_t numPackets = 1;
double interval = 1.0; // seconds
bool verbose = false;
Time interPacketInterval = Seconds(interval);
int StateTable[nodenumber] = { 0 };
int LevelTable[nodenumber] = { 0 };
int NeighborTable[nodenumber][nodenumber];
double getdistance(Ptr<Node> node1, Ptr<Node> node2) {
	Ptr<Object> object1 = node1;
	Ptr<MobilityModel> model1 = object1->GetObject<MobilityModel>();
	Ptr<Object> object2 = node2;
	Ptr<MobilityModel> model2 = object2->GetObject<MobilityModel>();
	return model1->GetDistanceFrom(model2);
}
void initial(NodeContainer c) {
	for (int i = 0; i < nodenumber; i++) {
		StateTable[i] = 2;
	}
	for (int i = 0; i < nodenumber - 2; i++) {
		LevelTable[i] = i / 3;
	}
	LevelTable[nodenumber - 2] = -1;
	LevelTable[nodenumber - 1] = 4;
	for (int i = 0; i < nodenumber; i++) {
		for (int j = 0; j < nodenumber; j++) {
			NeighborTable[i][j] = -2;
		}
	}
	for (int i = 0; i < nodenumber; i++) {
		for (int j = 0; j < nodenumber; j++) {
			if (getdistance(c.Get(i), c.Get(j)) <= range && LevelTable[j] > LevelTable[i]) {
				int k = 0;
				while (NeighborTable[i][k] != -2)k++;
				NeighborTable[i][k] = j;
			}
		}
	}
}
void ReceivePacket(Ptr<Socket> socket)
{
	while (socket->Recv())
	{
		NS_LOG_UNCOND("Received one packet!");
	}
}

static void GenerateTraffic(Ptr<Socket> socket, uint32_t pktSize,
	uint32_t pktCount, Time pktInterval,int targetid,int sourceid)
{
	if (pktCount > 0)
	{
		socket->Send(Create<Packet>(pktSize));
		Simulator::Schedule(pktInterval, &GenerateTraffic,
			socket, pktSize, pktCount - 1, pktInterval, targetid, sourceid);
	}
	else
	{
		socket->Close();
		StateTable[targetid] = StateTable[sourceid] = 2;
	}
}
void send(int sourceid, int targetid, NodeContainer c, TypeId tid, double time) {
	missing++;
	StateTable[sourceid] = 0;
	StateTable[targetid] = 1;
	Ptr<Socket> recvSink = Socket::CreateSocket(c.Get(targetid), tid);
	InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
	recvSink->Bind(local);
	recvSink->SetRecvCallback(MakeCallback(&ReceivePacket));

	Ptr<Socket> source = Socket::CreateSocket(c.Get(sourceid), tid);
	InetSocketAddress remote = InetSocketAddress(Ipv4Address("255.255.255.255"), 80);
	source->SetAllowBroadcast(true);
	source->Connect(remote);
	Simulator::ScheduleWithContext(source->GetNode()->GetId(),
		Seconds(time), &GenerateTraffic,
		source, packetSize, numPackets, interPacketInterval, targetid, sourceid);
	//GenerateTraffic(source, packetSize, numPackets, interPacketInterval);
}
int sendexecute(int sourceid, NodeContainer c, TypeId tid, double time) {
	for (int i = 0; i < nodenumber; i++) {
		int judge = NeighborTable[sourceid][i];
		if (judge == -2)continue;
		if (StateTable[judge] != 2)continue;
		send(sourceid, judge, c, tid, time);
		return judge;
	}
	return -1;
}
void work(int currentid,NodeContainer c, TypeId tid, double time) {
	int nextleap = sendexecute(currentid, c, tid, time);
	if (nextleap == -1) {
		//Time _now = Simulator::Now();
		//double now = _now.GetSeconds();
		//while (Simulator::Now() != Time(now * 1.01));
		//work(currentid, c, tid, time * 1.1);
		Simulator::ScheduleWithContext(c.Get(currentid)->GetId(), Seconds(time * 1.01), &work, currentid, c, tid, time * 1.01);
	}
	else if (nextleap == nodenumber - 1) {
		return;
	}
	else {
		Simulator::ScheduleWithContext(c.Get(nextleap)->GetId(), Seconds(time), &work, nextleap, c, tid, time);
	}
}
int main(int argc, char* argv[])
{
	std::string phyMode("DsssRate1Mbps");

	CommandLine cmd;

	cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
	cmd.AddValue("rss", "received signal strength", rss);
	cmd.AddValue("packetSize", "size of application packet sent", packetSize);
	cmd.AddValue("numPackets", "number of packets generated", numPackets);
	cmd.AddValue("interval", "interval (seconds) between packets", interval);
	cmd.AddValue("verbose", "turn on all WifiNetDevice log components", verbose);

	cmd.Parse(argc, argv);
	// Convert to time object
	// disable fragmentation for frames below 2200 bytes
	Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
		StringValue(phyMode));

	NodeContainer c;
	c.Create(nodenumber);

	// The below set of helpers will help us to put together the wifi NICs we want
	WifiHelper wifi;
	if (verbose)
	{
		wifi.EnableLogComponents();  // Turn on all Wifi logging
	}
	wifi.SetStandard(WIFI_STANDARD_80211b);

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper();
	// This is one parameter that matters when using FixedRssLossModel
	// set it to zero; otherwise, gain will be added
	wifiPhy.Set("RxGain", DoubleValue(0));
	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	// The below FixedRssLossModel will cause the rss to be fixed regardless
	// of the distance between the two stations, and the transmit power
	Config::SetDefault("ns3::RangePropagationLossModel::MaxRange", DoubleValue(range));
	wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel");
	//wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
	wifiPhy.SetChannel(wifiChannel.Create());

	// Add a mac and disable rate control
	WifiMacHelper wifiMac;
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
		"DataMode", StringValue(phyMode),
		"ControlMode", StringValue(phyMode));
	// Set it to adhoc mode
	wifiMac.SetType("ns3::AdhocWifiMac");
	NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

	// Note that with FixedRssLossModel, the positions below are not
	// used for received signal strength.
	MobilityHelper m2;
	m2.SetPositionAllocator("ns3::GridPositionAllocator",
		"MinX", DoubleValue(10.0),
		"MinY", DoubleValue(10.0),
		"DeltaX", DoubleValue(10.0),
		"DeltaY", DoubleValue(10.0),
		"GridWidth", UintegerValue(3),
		"LayoutType", StringValue("RowFirst"));
	m2.Install(c);
	m2.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
		"Bounds", RectangleValue(Rectangle(-500, 500, 0, 40)));
	NodeContainer source;
	source.Add(c.Get(nodenumber - 2));
	NodeContainer sink;
	sink.Add(c.Get(nodenumber - 1));
	NodeContainer endpoints;
	endpoints.Add(source);
	endpoints.Add(sink);
	MobilityHelper m3;
	m3.SetPositionAllocator("ns3::GridPositionAllocator",
		"MinX", DoubleValue(20.0),
		"MinY", DoubleValue(0.0),
		"DeltaX", DoubleValue(0.0),
		"DeltaY", DoubleValue(40.0),
		"GridWidth", UintegerValue(1),
		"LayoutType", StringValue("RowFirst"));
	m3.Install(endpoints);
	initial(c);

	InternetStackHelper internet;
	internet.Install(c);

	Ipv4AddressHelper ipv4;
	NS_LOG_INFO("Assign IP Addresses.");
	ipv4.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i = ipv4.Assign(devices);

	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

	// Tracing
	wifiPhy.EnablePcap("wifi-simple-adhoc", devices);
	Simulator::ScheduleWithContext(c.Get(nodenumber-2)->GetId(), Seconds(1.0), &work, nodenumber - 2, c, tid, 4.0);
	Simulator::ScheduleWithContext(c.Get(nodenumber - 2)->GetId(), Seconds(2.0), &work, nodenumber - 2, c, tid, 4.0);
	//send(1,4,c,tid,2.0);

	// Output what we are doing
	NS_LOG_UNCOND("Testing " << numPackets << " packets sent with receiver rss " << rss);
	NS_LOG_UNCOND("missing:" << missing);

	AnimationInterface anim("wifi-simple-adhoc.xml");
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
