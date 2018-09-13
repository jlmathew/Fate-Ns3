/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "ns3/IcnName.h"
#include <iostream>
#include <ostream>
#include <string>
#include "ns3/PacketTypeBase.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/node.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");
UtilityExternalModule *mod;
GlobalModuleTimerLinux *timer;

void printRoutingTable (Ptr<Node> node) { 
        Ipv4StaticRoutingHelper helper; 
        Ptr<Ipv4> stack = node -> GetObject<Ipv4>(); 
        Ptr<Ipv4StaticRouting> staticrouting = helper.GetStaticRouting 
(stack); 
        uint32_t numroutes=staticrouting->GetNRoutes(); 
        Ipv4RoutingTableEntry entry; 
        std::cout << "Routing table for device: " << 
Names::FindName(node) << 
"\n"; 
        std::cout << "Destination\tMask\t\tGateway\t\tIface\n"; 

        for (uint32_t i =0 ; i<numroutes;i++) { 
                entry =staticrouting->GetRoute(i); 
                std::cout << entry.GetDestNetwork()  << "\t" 
                                << entry.GetDestNetworkMask() << "\t" 
                                << entry.GetGateway() << "\t\t" 
                                << entry.GetInterface() << "\n"; 
        } 

Ipv4GlobalRoutingHelper globalRouting;
Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("global.routes", std::ios::out);
globalRouting.PrintRoutingTableAllAt (Seconds(0.1), routingStream );

        return; 

} 




int 
main (int argc, char *argv[])
{
  bool verbose = true;
ns3::PacketMetadata::Enable ();
  ns3::Packet::EnablePrinting ();

  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpFateCbrClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpFateServerApplication", LOG_LEVEL_INFO);
    }
  NodeContainer all;
  all.Create(8);
  PointToPointHelper p2pMap;
  p2pMap.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2pMap.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NodeContainer consumers = NodeContainer ( all.Get(0),all.Get(1),all.Get(2));
  NodeContainer producers = NodeContainer( all.Get(5), all.Get(6), all.Get(7));
  NodeContainer networkNodes = NodeContainer (all.Get(3), all.Get(4));

  //consumers
  Ipv4InterfaceContainer consumerInterface, producerInterface, networkInterface;
  InternetStackHelper ipStackNet;
  ipStackNet.Install(all);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.240");
  NetDeviceContainer netDevice, producerDevice, consumerDevice;
  for(unsigned int i=0; i<3; i++) {
    consumerDevice.Add(p2pMap.Install(networkNodes.Get(0),consumers.Get(i))); 
    consumerInterface.Add(address.Assign(consumerDevice));
address.NewNetwork();
  }


  //network
  address.SetBase ("10.1.2.0", "255.255.255.0");
  netDevice.Add(p2pMap.Install(networkNodes.Get(0), networkNodes.Get(1)));
  networkInterface.Add(address.Assign(netDevice));
  address.NewNetwork();

printRoutingTable(networkNodes.Get(1));
printRoutingTable(consumers.Get(1));
  //producers
  address.SetBase ("10.1.3.0", "255.255.255.240");
  for(unsigned int i=0; i<3; i+=2) {
    producerDevice.Add(p2pMap.Install(networkNodes.Get(1),producers.Get(i))); 
producerInterface.Add(address.Assign(producerDevice));
address.NewNetwork();
  }

    NetDeviceContainer server;
    server.Add(p2pMap.Install(networkNodes.Get(1),producers.Get(1))); 
    producerDevice.Add(server); 
    Ipv4InterfaceContainer serverInterface; 
    serverInterface = address.Assign(server);
    producerInterface.Add(serverInterface);
address.NewNetwork();

UdpFateServerHelper echoServer (9);
echoServer.SetAttribute("ReturnSize", UintegerValue(10));

     Ptr<Ipv4> ipv4 = producers.Get(1)->GetObject<Ipv4>();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
        Ipv4Address addri = iaddr.GetLocal ();
        std::cout << "producer 0 ip addr:" << addri << std::endl;
  ApplicationContainer serverApps = echoServer.Install (producers.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (100.0));

  UdpFateCbrClientHelper echoClient (serverInterface.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (9));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (.1)));
  echoClient.SetAttribute ("NumSegments", UintegerValue (4));
  echoClient.SetAttribute ("NumFiles", UintegerValue (1));
  echoClient.SetAttribute ("AddTimeStamp", BooleanValue(true));
  
   PktType fatePkt;
   fatePkt.SetUnsignedNamedAttribute("TtlHop", 64);
   //fatePkt.SetObjectCpyNamedAttribute("Timestamp", ns3::Simulator::Now());
   fatePkt.SetPacketPurpose(PktType::INTERESTPKT);
   IcnName<std::string> pktName;
   pktName.SetFullName("/test");

   //seems the only way to preload the packet type, is to do it here
  ApplicationContainer clientApps = echoClient.Install (consumers.Get (0));
  echoClient.SetPktPayload(clientApps.Get(0) , fatePkt);

  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (12.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  p2pMap.EnablePcapAll ("fate-pcap");
  //pointToPoint.EnablePcapAll ("second");
  //csma.EnablePcap ("second", .Get (1), true); 

//install fate nodes
   NodeManager *fate=0;

  //fate in consumers
  mod = new UtilityExternalModule;
  timer = new GlobalModuleTimerLinux;

  (mod->GetGlobalModule ()).SetGlobalTimer (timer);
  (mod->GetGlobalModule ()).SetGlobalLog ("default", new CoutModuleLog);
  
    UtilityConfigXml config;
    config.FirstNodeFileConfig("fateXmlConfigFiles/Ns3-node-configC.xml");
    fate = new NodeManager(config);
    std::string nodeName="Node";
    uint32_t nodeNum = networkNodes.Get(1)->GetId();
    std::ostringstream convert;
    convert << nodeNum;
    nodeName.append(convert.str());

//Ptr<Node> node = allNodes.Get (i); // Get pointer to ith node in container
    
    (mod->GetDevice ()).SetNodeName (nodeName);
    fate->OnInit(mod);
    (networkNodes.Get(1))->InsertFateNode(fate);
       UtilityConfigXml config2;
       config2.FirstNodeFileConfig("fateXmlConfigFiles/Ns3-node-configB.xml");
for(unsigned int i=0; i<8; i++) {
     if (i != nodeNum) {
       UtilityExternalModule *mod2;
       mod2 = new UtilityExternalModule;
       (mod2->GetGlobalModule ()).SetGlobalTimer (timer);
       (mod2->GetGlobalModule ()).SetGlobalLog ("default", new CoutModuleLog);
       fate = new NodeManager(config2);
       std::string nodeName="Node";
       std::ostringstream convert;
       convert << i;
       nodeName.append(convert.str());
       (mod2->GetDevice ()).SetNodeName (nodeName);
       fate->OnInit(mod2);
       all.Get(i)->InsertFateNode(fate);
     }

}
    


Packet::EnablePrinting ();
AsciiTraceHelper ascii;
p2pMap.EnableAsciiAll(ascii.CreateFileStream("simulation1.tr1"));

  Ptr<FlowMonitor> flowmon;
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
  Simulator::Stop (Seconds(15.0));

Simulator::Run ();
	  flowmon->CheckForLostPackets ();
	  flowmon->SerializeToXmlFile("fate-test1.flowmon", true, true);

  Simulator::Destroy ();
  return 0;
}
