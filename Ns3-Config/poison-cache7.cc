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
#include "ns3/GlobalModulesNs3.h"
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
#include "ns3/topology-read-module.h"
//#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/global-fate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include <fstream>
#include "ns3/NodeManager.h"
#include "ns3/UtilityExternalModule.h"
#include "ns3/UtilityConfigXml.h"
#include "ns3/fateIpv4-interface.h"
#include "ns3/fate-ipv4protocol.h"
#include "ns3/fateipv4-helper.h"

/* WHY and limiations


   */
using namespace ns3;


  double alpha[4] = {1.0, 1.0, 1.0, 1.0};
  double reqRate[4] = {50.0, 15.0, 15.0, 15.0};
NS_LOG_COMPONENT_DEFINE ("FateFileExample");
UtilityExternalModule *mod;
GlobalModuleTimerNs3 *timer;
InternetStackHelper stack;
NodeContainer* nc;
NetDeviceContainer* ndc;
NetDeviceContainer* ndconsumer;
PointToPointHelper p2p;
Ipv4InterfaceContainer* ipic;
NodeContainer nodes;
NodeContainer producers, preconsumers, consumers, cachingNodes, nonCachingNodes;
uint32_t nFiles, othernFiles;
Ipv4AddressHelper address;
Ipv4NixVectorHelper nixRouting;
Ptr<TopologyReader> inFile ;
Ptr<FlowMonitor> flowmon;
AsciiTraceHelper ascii;
FlowMonitorHelper flowmonHelper;

//hook up the consumer applications to pre 'consumer' nodes, since fate doesnt start with the consumer.

NetDeviceContainer *preConsumerLink;
PointToPointHelper pConsLink;
Ipv4InterfaceContainer* preConsInt;
int32_t totlinks;
ApplicationContainer serverApps;
ApplicationContainer clientApps[4];
uint32_t numClientPerNodes;
NodeStats *stats;
GlobalModule *global;
DeviceModule *devMod;

std::string logName;
std::string fileLengths;
std::string fileLengthName;
uint32_t maxPkts;
std::string input;
std::string format;
std::string pTopo;
std::string cTopo;
std::string cacheTopo;
uint32_t seed;
uint32_t totTime;
std::string cConfig;
std::string nConfig;

std::string segstr;


void setProducers(uint32_t nodeNum, uint32_t contentNum, bool drop) {
    //FIXME HERE
    //UdpFateServerHelper echoServer(100+i); //FIXME what if > 64k?
    //UdpFateFileZipfServerHelper echoServer(100+nodeNum); //FIXME what if > 64k?
    UdpFateServerHelper echoServer(100+nodeNum); //FIXME what if > 64k?
    echoServer.SetAttribute("ReturnSize", UintegerValue(500));
    std::string matchName="/test"+std::to_string(contentNum);//+"/fileNum=";
    //echoServer.SetAttribute("QoS",  UintegerValue(1 << contentNum));
    echoServer.SetAttribute("MinMatchName", StringValue(matchName));
    //echoServer.SetAttribute ("matchByType", StringValue("location"));
    //echoServer.SetAttribute("FileSizes", StringValue(segstr));
    echoServer.SetAttribute("DropPacket", BooleanValue(drop));
    serverApps.Add( echoServer.Install(nodes.Get(nodeNum)));
}

void create1Producers(unsigned int nodeNum, uint32_t contentNum, bool drop) {
    producers.Add(nodes.Get(nodeNum));
  setProducers(nodeNum, contentNum, drop);
}
void postConsumer() {

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}

void create1PreConsumers(uint32_t nodeId, double alpha, double reqRate, uint32_t reqNum, uint32_t type, uint32_t numfiles) {
    preconsumers.Add(nodes.Get(nodeId));
    consumers.Add(nodes.Get(nodeId));
  //
    std::string matchName="/test"+std::to_string(reqNum); //+"/fileNum=";
static int clientAppNum=0;
  //add application to each
      UdpFateZipfClientHelper echoClient(ipic[0].GetAddress (1),9);   //based upon the name, it will map to an ip/port
      //UdpFateFileZipfClientHelper echoClient(ipic[0].GetAddress (1),9);   //based upon the name, it will map to an ip/port
      echoClient.SetAttribute ("MaxPackets", UintegerValue (maxPkts));
      echoClient.SetAttribute ("Interval", TimeValue (Seconds ((double) 1.0/reqRate)));
      echoClient.SetAttribute ("NumFiles", UintegerValue (numfiles));
      echoClient.SetAttribute ("PktType", UintegerValue (type));
      echoClient.SetAttribute ("AddTimeStamp", BooleanValue(true));
      echoClient.SetAttribute ("ZipfAlpha", DoubleValue(alpha));
      echoClient.SetAttribute ("NStaticDestination", BooleanValue(true)); //use the static index or not
      echoClient.SetAttribute ("matchByType", StringValue("location"));
      echoClient.SetAttribute ("matchString", StringValue(matchName));
      //true for files, false for packet requests
      //echoClient.SetAttribute ("FileMultipart", BooleanValue(false));

      PktType fatePkt;
      fatePkt.SetUnsignedNamedAttribute("TtlHop", 128);
      fatePkt.SetPacketPurpose(PktType::INTERESTPKT);
      IcnName<std::string> pktName;
      std::string conName("/test");
      conName.append(std::to_string(reqNum));
      pktName.SetFullName(conName);
      fatePkt.SetName(pktName);
      std::stringstream out;
      out << fatePkt;
      echoClient.SetAttribute ("PktPayload", StringValue(out.str())); //use the static index or not
      clientApps[clientAppNum++].Add(echoClient.Install(nodes.Get(nodeId)));

}



void createFateNodes(const std::string &nconfig, uint32_t nodeId) {
  //cache only nodes are cConfig, all else are nConfig.
  UtilityConfigXml config;
  config.FirstNodeFileConfig(nconfig);
  FateIpv4Helper helper;
  helper.SetConfigFile(nconfig);

  stats= new CustomStats;
  GlobalModule *global = new GlobalModule;
  GlobalModuleTimerNs3 *timer = new GlobalModuleTimerNs3;
  global->SetGlobalTimer(timer);
  GlobalModuleLog *log = new CoutModuleLog;
  global->SetGlobalLog("default", log );
  helper.SetStats(stats);
  helper.SetLog(log);
  helper.SetGlobalModule(global);
  helper.Install(nodes.Get(nodeId));


}
void
createFateLogs() {
  std::string name=logName;
  name=logName;
  name.append("-server.stat");
  std::fstream fs2;
  fs2.open(name,std::fstream::out );

  UdpFateFileZipfServerHelper echoServer(100);
  for(unsigned int i=0; i<producers.GetN(); i++) {
    echoServer.DumpStats(producers.Get(i), fs2);
  }
  fs2 << "\n";
  fs2.close();


  name=logName;
  name.append("-client.stat");
  std::fstream fs3;
  fs3.open(name,std::fstream::out );

  //preclients.DumpStats(consumers.Get (0),std::cout);
  UdpFateFileZipfClientHelper echoConsumer(ipic[0].GetAddress(1),9);
  for(unsigned int i=0; i<consumers.GetN(); i++) {
    echoConsumer.DumpStats(consumers.Get(i), fs3);
  }
  fs3 << "\n";
  fs3.close();

  name=logName;
  name.append("-cache.stat");
  std::fstream fs4;
  fs4.open(name,std::fstream::out );

  //preclients.DumpStats(consumers.Get (0),std::cout);
  for(unsigned int i=0; i<cachingNodes.GetN(); i++) {
    fs4 << "\nCached Node(cache" << i << ":n" << consumers.Get(i)->GetId()<< "):\n";
    FateIpv4Helper::DumpStats(cachingNodes.Get(i), fs4);
  }
  
  fs4 << "\n";
  fs4.close();
  
  /*std::fstream fs;
  name=logName;
  name.append("-fate.stat");
  fs.open(name,std::fstream::out );
  for(unsigned int i=0; i<5; i++) { //nodes.GetN(); i++) {
    fs << "\nNode(" << nodes.Get(i)->GetId()<< "):\n";
     FateIpv4Helper::DumpStats(consumers.Get(i), fs);
  }
  fs.close();
*/

}

//option to enable/disable logging
void setLogging() {
  std::string name=logName;
  name.append(".tr1");
  p2p.EnableAsciiAll(ascii.CreateFileStream(name));

  flowmon = flowmonHelper.InstallAll ();
  flowmon->CheckForLostPackets ();
  name=logName;
  name.append(".flowmon");
  flowmon->SerializeToXmlFile(name, true, true);

  name=logName;
  name.append("-pcap");
  p2p.EnablePcapAll (name);

  Ipv4GlobalRoutingHelper globalRouting;
  name=logName;
  name.append(".routes");
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (name, std::ios::out);
  globalRouting.PrintRoutingTableAllAt (Seconds(1.1), routingStream );

}

void logInfo() {
  std::string name = logName;
  name.append(".info");
  std::fstream fs2;
  fs2.open(name,std::fstream::out | std::fstream::out);
  fs2 << "Map\tNodes\tLinks\tformat\tReqRate\tnumFilesRq\tnumSegmentsPerFile\tnumConsumersPerNode\tnumConsumerNodes\tnumProducerNodes\tRndSeed\tsimTime\tmaxPktsPerConsumer\tzipfAlpha\tproducerTopology\t\tconsumerTopology\t\tedge-CacheXml\tnonedge-CacheXml\t\tnon-cacheXml\n";
  fs2 << input << "\t" << NodeContainer::GetGlobal().GetN() << "\t" << totlinks <<"\t" << format << "\t" << reqRate[0] << "\t" << nFiles << "\t"  << numClientPerNodes << "\t" << consumers.GetN() << "\t" << producers.GetN() << "\t" << seed << "\t" << totTime << "\t" << maxPkts << "\t" << std::setprecision (15) << alpha[0] << "\t" << pTopo << "\t" << cTopo << "\t" << cConfig << "\t" << cConfig << "\t" << nConfig << "\n";
  fs2.close();


}

void createTopology(std::string input, std::string format) {
  TopologyReaderHelper topoHelp;
  topoHelp.SetFileName (input);
  topoHelp.SetFileType (format);
  inFile = topoHelp.GetTopologyReader ();


  if (inFile != 0)
  {
    nodes = inFile->Read ();
  }

  if (inFile->LinksSize () == 0)
  {
    NS_LOG_ERROR ("Problems reading the topology file. Failing.");
    assert(0);
  }

  NS_LOG_INFO ("creating internet stack");

  // Setup NixVector Routing
  //stack.SetRoutingHelper (nixRouting);  // has effect on the next Install ()
  //stack.SetRoutingHelper();
  stack.Install (nodes);

  NS_LOG_INFO ("creating ip4 addresses");
  address.SetBase ("10.0.0.0", "255.255.255.252");

  totlinks = inFile->LinksSize ();

  NS_LOG_INFO ("creating node containers");
  nc = new NodeContainer[totlinks];
  TopologyReader::ConstLinksIterator iter;
  int i = 0;
  for ( iter = inFile->LinksBegin (); iter != inFile->LinksEnd (); iter++, i++ )
  {
    nc[i] = NodeContainer (iter->GetFromNode (), iter->GetToNode ());
  }

  NS_LOG_INFO ("creating net device containers");
  ndc = new NetDeviceContainer[totlinks];
  for (int i = 0; i < totlinks; i++)
  {
    // p2p.SetChannelAttribute ("Delay", TimeValue(MilliSeconds(weight[i])));
    //p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    //p2p.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
    ndc[i] = p2p.Install (nc[i]);
  }

  // it crates little subnets, one for each couple of nodes.
  NS_LOG_INFO ("creating ipv4 interfaces");
  ipic = new Ipv4InterfaceContainer[totlinks];
  for (int i = 0; i < totlinks; i++)
  {
    ipic[i] = address.Assign (ndc[i]);
    address.NewNetwork ();
  }
}
int
main (int argc, char *argv[])
{
  bool verbose = false;

  Packet::EnablePrinting();
  Packet::EnableChecking();
  input="scratch/poison.orb";
  pTopo="Single"; //single, or random
  cTopo="Edge"; //edge or random
  cacheTopo="Edge"; //edge or random
  nFiles = 100000; //10000;
  othernFiles = 10; 
  logName="logs/testing"; //"logs/default";
  fileLengthName=""; //"fileLengths.txt";
  uint32_t runMask = 4;
  format="Orbis"; //RocketFuel, Inet, Orbis
//FIXME HERE
//cConfig="fateXmlConfigFiles/lfuxlru2.xml";  //no config or name of xml file
  cConfig="fateXmlConfigFiles/cache-poison-c50.xml";  //no config or name of xml file
  //cConfig="fateXmlConfigFiles/Ns3-node.xml";
  nConfig="fateXmlConfigFiles/Ns3-node.xml";
  numClientPerNodes=1;
  seed = 1;
  totTime=90;
  maxPkts=40900; //1000000; //000;
  std::string matchType="filenum"; //"location";
  //std::string matchType="location"; //"location";
  double alpha2=1.0; //.9999999999;
  CommandLine cmd;

  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("alpha", "Tell echo applications to log if true", alpha2);
  cmd.AddValue ("format", "Format to use for data input [Orbis|Inet|Rocketfuel].",
                format);
  cmd.AddValue("consumerTopo", "How consumers are selected from topology", cTopo);
  //cmd.AddValue("numFiles", "How many files to request", nFiles);
  cmd.AddValue("runMask", "run mask 1 (regular),2 (bad),4 (great)", runMask);
  cmd.AddValue("otherNumFiles", "How many files to request", othernFiles);
  cmd.AddValue("numFiles", "How many files to request", nFiles);
  cmd.AddValue("cacheNodeConfig", "config file name for cache nodes", cConfig);
  cmd.AddValue("nonCacheNodeConfig", "config file name for non cache nodes", nConfig);
  //rnd seed
  //num client apps per node
  cmd.AddValue("rndSeed", "Random Seed Value", seed);
  cmd.AddValue("time", "how long the network simulator should run, in seconds", totTime);
  cmd.AddValue("logName", "Log name",logName);
  //cmd.AddValue("fileLengths", "file with string lengths",fileLengthName);
  cmd.AddValue("maxPkts", "maximum number of interest packets to send.  '0' = unlimited",maxPkts);
  cmd.Parse (argc,argv);

  //parse fileLengthName into fileLengths
  if (fileLengthName.size()) {
    std::string strbuf;
    std::ifstream is (fileLengthName, std::ifstream::in);
    while (std::getline(is,strbuf))
    {
      segstr  += strbuf;
      segstr +="\n";
    }
    is.close();

  }
//FIXME TODO
//


  RngSeedManager::SetSeed(seed);
  if (!verbose)
  {
    LogComponentDisableAll(LOG_PREFIX_ALL);
  } else {
    PacketMetadata::Enable ();
    Packet::EnablePrinting ();
    LogComponentEnable ("UdpFateFileZipfClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpFateFileZipfServerApplication", LOG_LEVEL_INFO);
  }
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("10Gbps"));
  p2p.SetQueue("ns3::DropTailQueue");
  //Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("1000000"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("1ms"));
  createTopology(input, format);
    create1Producers(4, 1, false); //return packet
    create1Producers(2, 2, true); //drop packet
    create1Producers(3, 1, true); //drop packet

//consumers
    create1PreConsumers(0, alpha2, reqRate[0], 1, 4, nFiles); //send interest packet

    create1PreConsumers(5, alpha2, reqRate[1], 2, 8, nFiles); //send data packet
    create1PreConsumers(6, alpha2, reqRate[2], 1, 8, othernFiles); //send data packet

    postConsumer();
//node 1 is cache
//
  CreateDestAssociation(producers);
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (totTime+1.0));  //set as param FIXME

  for(int i=0; i<7; i++) {
    switch (i) {
	    //individual cache nodes
	    case 1:
   		createFateNodes(cConfig,i);
		cachingNodes.Add(nodes.Get(i));
		break;

	default:
   		createFateNodes(nConfig,i);
		break;
    }

  }
  //createConsumers();
  if (runMask & 1) {
  clientApps[0].Start (Seconds (2.0));
  clientApps[0].Stop (Seconds (totTime+.5));
std::cout << "activating good consumer\n";
  }
  if (runMask & 2) {
  clientApps[1].Start (Seconds (3.0));
  clientApps[1].Stop (Seconds (totTime+.5));
std::cout << "activating bad consumer\n";
  }
  if (runMask & 4) {
  clientApps[2].Start (Seconds (3.1));
  clientApps[2].Stop (Seconds (totTime+.5));
std::cout << "activating helpful consumer\n";
  }
  if(verbose) {
    setLogging();
  }
  Simulator::Stop(Seconds(totTime+4.0));
  for(unsigned int i=0; i<nodes.GetN(); i++) {
    //std::cout << nodes.Get(i)->GetFateNode()->Name() << "\n";
  }
  std::cout << "total nodes: " << NodeContainer::GetGlobal().GetN() << "\n";
  std::cout << "total consumers:" << consumers.GetN() << "\n";
  Simulator::Run ();
  logInfo();
  createFateLogs();
  Simulator::Destroy ();
  return 0;
}
