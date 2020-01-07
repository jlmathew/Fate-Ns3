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
   This program creates 4 types of nodes.   
   Producers: Producers produce file content.   There are nFiles of unique content, and each file consists of nSegments.  
   It is assumed (for now), that 1 files uniquely is produced by one producer.   A producer CAN producer more than 1 file content, but there are no
   redundant sources for file creation (e.g. node 3 and 5 wont be producers for file n5, but node 3 can produce for files n5, n6).

   preconsumers are 1 hop from consumers (see below). 

   consumers: consumers, due to a poor ns3 implementation, are not included in tx'ing a packet (tracing a packet shows the ns3 application transmits without going 
   through 'the node'). In addition, we need an easy way to identify 1-hop cache node, and since the 'original consumer', due to the poor ns3-fate implementation, wont
   be able to return an interest packet, requires an extra hop of a consumer (where the preconsumer can cache).   For calculations, subtract 1 from TTL or ignore last 
   node name hop.

   nodes = all nodes, from the map, EXCEPT consumers (as they are added separately, and not part of the original map.
   ==========================================================
   Maps: 3 type of maps are supported (Inet, rocketfuel, orbis).   rocketfuel is an actual map of an ISP.   Inet and Orbis generate ISP-like random topologies.   The 
   generator for orbis does not work correctly, so the number of maps is limited.    Inet map generation is working correctly.
   ==================================================================
Inputs:
   input    : name of the topology file
   format   : format of the topology file
   reqRate - How many interest requests in seconds (converted to 1/reqRate)
   nProd - How many producers of unique content.   If nProd >=1 it is the actual # of producers.   If it is 0< nProd < 1, then it is the percentage of all nodes,
      which are producers.  e.g. (.25) means 1/4 or 25% of all nodes are producers.
   nCons - How many consumers. If nCons >=1 it is the actual # of consumers.   If it is 0< nCons < 1, then it is the percentage of all nodes,
      which are consumers.  e.g. (.25) means 1/4 or 25% of all nodes are consumers.  Inet papers suggest 23% is a good number.
   pTopo - How producers are selected in the topology.   Either a 'Single' producer is choosen (and nProd is the Id of the node, e.g. if pTopo='Single' and nProd=5, then 
      node 5 is the single producer; if 'Random' is choosen, then a node at random is choosen to be producer (with nProd nodes).
   pTopo - How consumers are selected in the topology.   Either by 'Edge', where only those nodes with a single link are choosen (disregarding nCons), or 'Random',
      which will create nCons random nodes to be selected.
   nFiles - How many unique files are to be produced/requested in the network.    For zipf, file1 is requested most often, and filen is requested least often.
   numSegments - How many segments per file.   assume all files are same length.   The purpose of having segments allows prefetching for future experiments, and stress the
      cache more.
   
   
   */
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FateMassCache3c");
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
uint32_t nFiles, nSeg, reqRate;
double alpha;
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
ApplicationContainer clientApps;
uint32_t numClientPerNodes;
NodeStats *stats=NULL;
GlobalModule *global=NULL;
DeviceModule *devMod=NULL;

std::string logName;
uint32_t maxPkts;
std::string input;
std::string format;
double nProd;
double nCons;
std::string pTopo;
std::string cTopo;
std::string cacheTopo;
uint32_t seed;
uint32_t totTime;
std::string cConfig;
std::string nConfig;
std::map<Ipv4Address, std::string> *dns;
uint32_t nCaches=5; //1; //20;
//NodeContainer availNodes;
//NodeContainer consumers;
//NodeContainer producers;
NodeContainer caches;
std::vector<unsigned int> availNodes;
std::string sendOff="false";
std::string hashLoc="true";

//Fischer-Yates shuffle
void shuffle(std::vector<unsigned int> &v)
{
    int n = v.size();
    Ptr<UniformRandomVariable> unifRandom = CreateObject<UniformRandomVariable> ();
    unifRandom->SetAttribute ("Min", DoubleValue (0));
    unifRandom->SetAttribute ("Max", DoubleValue (n-1));
    for(int a=n-1; a>0; a--)
    {
        int j = unifRandom->GetInteger(0, a);
        //int j = unifRandom->GetInteger(a, n);
        int tmp = v[a];
        v[a] = v[j];
        v[j] = tmp;
    }

}
void setProducers(void) {
    /*uint32_t numProd = producers.GetN();
    std::vector<unsigned int> count(nFiles,0);
    for(unsigned int i=0; i< nFiles; i++)
    {
       count[i]=i;
    }
    shuffle(count);*/
    for(unsigned int i=0; i< producers.GetN(); i++)
    {
      UdpFateServerHelper echoServer(100+i);
      echoServer.SetAttribute("ReturnSize", UintegerValue(500));
      ///std::string matchName="/test"+std::to_string(i+1)+"/fileNum=";
      std::string matchName="/test"+std::to_string(i+1);
      //std::stringstream out;
      //out << count[i]+1;
      //matchName.append(out.str());

      echoServer.SetAttribute("MinMatchName", StringValue(matchName));
      /*int p;
      if (nFiles >= numProd) {
        p = i % numProd;
      } else {
        assert(0);
      }*/
      serverApps.Add( echoServer.Install(producers.Get(i)));
    }
    NS_LOG_INFO("actual post nFile servers are:" << serverApps.GetN());
}

void create1Producers(unsigned int nodeNum) {
    Ptr<UniformRandomVariable> unifRandom = CreateObject<UniformRandomVariable> ();
    unifRandom->SetAttribute ("Min", DoubleValue (0));
    unifRandom->SetAttribute ("Max", DoubleValue (nodes.GetN() - 1));
      producers.Add(nodes.Get(nodeNum));
    setProducers();
}

void createAllConsumers(void) {
    // Ptr<Node> newNode = CreateObject<Node>();
    //consumers.Add(nodes.Get(0));
    //stack.Add(consumers);
    //stack.InstallAll();
    //stack.Install(consumers);
         //p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
         //p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));


  //  
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  for(unsigned int i=0; i<consumers.GetN(); i++) {

    //add application to each
     UdpFateZipfClientHelper echoClient(ipic[0].GetAddress (1),9);   //based upon the name, it will map to an ip/port
       echoClient.SetAttribute ("MaxPackets", UintegerValue (maxPkts));
       echoClient.SetAttribute ("Interval", TimeValue (Seconds ((double) 1.0/reqRate)));
       echoClient.SetAttribute ("NumSegments", UintegerValue (nSeg));
       echoClient.SetAttribute ("NumFiles", UintegerValue (nFiles));
       echoClient.SetAttribute ("AddTimeStamp", BooleanValue(true));
       echoClient.SetAttribute ("ZipfAlpha", DoubleValue(alpha));
       echoClient.SetAttribute ("NStaticDestination", BooleanValue(true)); //use the static index or not
       echoClient.SetAttribute ("NStaticDestination", BooleanValue(true)); //use the static index or not
       echoClient.SetAttribute ("matchByType", StringValue("location")); //filenum, location, segnum
       bool val = (sendOff=="true" ? true : false);
       echoClient.SetAttribute("sendToOffPathCache", BooleanValue (val));
       val = (hashLoc=="true" ? true : false);
       echoClient.SetAttribute("hashOnLocation", BooleanValue(val));
       
      //Need to make this variable AND loop for all consumers 
       std::string temp="/test"+std::to_string( (i%producers.GetN())+1);
       echoClient.SetAttribute("matchString", StringValue(temp));
       PktType fatePkt;
       fatePkt.SetUnsignedNamedAttribute("TtlHop", 128);
       fatePkt.SetPacketPurpose(PktType::INTERESTPKT);
       //std::string cacheTrip="10.0.0.13"; //<--redirect cache IP
       //cacheTrip.push_back(';');
       //cacheTrip.append("10.0.0.22");
       //cacheTrip += '\n';
       //fatePkt.SetPrintedNamedAttribute("DstChain",cacheTrip);
       //fatePkt.SetPrintedNamedAttribute("ReturnChain","");
       IcnName<std::string> pktName;
       //pktName.SetFullName("/test1/segnum=");
       pktName.SetFullName(temp);
       fatePkt.SetName(pktName);
       std::stringstream out;
       out << fatePkt;
       echoClient.SetAttribute ("PktPayload", StringValue(out.str())); //use the static index or not
       clientApps.Add(echoClient.Install(consumers.Get(i)));
       //auto app = echoClient.Install(consumers.Get(0)); //i));
       //app.SetPktPayload(fatePkt);
       //clientApps.Add(app);
       //Ptr<UdpFateZipfClient> zapp = DynamicCast<UdpFateZipfClient>(app.Get(j)); 
       //zapp->SetPktPayload(fatePkt);
   //seems the only way to preload the packet type, is to do it here
   }
   

    
}


void
createAllCacheNodes()
{
    //Ptr<Node> node = nodes.Get(4); //4
    //  cachingNodes.Add(node);

      //nonCachingNodes.Add(node1);
}

void createSubFateNodes(const std::string &config, const NodeContainer &nodes)
{
	UtilityConfigXml cconfig;
	//system sed file ? ; TODO FIXME JLM
	
      cconfig.FirstNodeFileConfig(config);
      FateIpv4Helper helper;
      helper.SetConfigFile(config);
      //if (!stats)
        stats= new CustomStats;
     // if(!global) {
	      GlobalModule *global = new GlobalModule;
        GlobalModuleTimerNs3 *timer = new GlobalModuleTimerNs3;
        global->SetGlobalTimer(timer);
        GlobalModuleLog *log = new CoutModuleLog;
        global->SetGlobalLog("default", log );
        global->dnsEntry = GetDns(); //(void *) (dns);
     // }
      helper.SetStats(stats);
      helper.SetLog(log);
      helper.SetGlobalModule(global);
      helper.Install(nodes);

}

void createFateNodes(const std::string &cConfig, const std::string &nConfig) {
    //cache only nodes are cConfig, all else are nConfig.
//was cancelled out ... necessary?
//
createSubFateNodes(cConfig, caches);
createSubFateNodes(nConfig, producers );
createSubFateNodes(nConfig, consumers );
NodeContainer restNodes;
for(unsigned int i=0; i<availNodes.size(); i++)
{
    restNodes.Add(NodeContainer::GetGlobal().Get(availNodes[i]));
}
createSubFateNodes(nConfig, restNodes  ); //non cache/producer/consumer nodes

std::cout << " initialized " << caches.GetN() + producers.GetN() + consumers.GetN()+ restNodes.GetN() << "nodes out of " << NodeContainer::GetGlobal().GetN() << "\n";
}
void
createFateLogs() {
    std::string name=logName;
    std::fstream fs;
    name.append("-fate.stat");
    fs.open(name,std::fstream::out );
    stats->DumpStats(fs);
    fs.close();

    name=logName;
    name.append("-server.stat"); 
      std::fstream fs2;
     fs2.open(name,std::fstream::out );

    UdpFateServerHelper echoServer(100); 
    for(unsigned int i=0; i<producers.GetN();i++) {
          fs2 << "\nProducer Node(p" << i << ":n" <<  producers.Get(i)->GetId()<< ":n):\n";
          echoServer.DumpStats(producers.Get(i), fs2);
    }
    fs2 << "\n";
    fs2.close();


    name=logName;
    name.append("-client.stat"); 
      std::fstream fs3;
     fs3.open(name,std::fstream::out );

    //preclients.DumpStats(consumers.Get (0),std::cout);
    UdpFateZipfClientHelper echoConsumer(ipic[0].GetAddress(1),9); 
    for(unsigned int i=0; i<consumers.GetN();i++) {
          fs3 << "\nConsumer Node(c" << i << ":n" << consumers.Get(i)->GetId()<< "):\n";
          echoConsumer.DumpStats(consumers.Get(i), fs3);
    }
    fs3 << "\n";
    fs3.close();


  name=logName;
  name.append("-cache.stat");
  std::fstream fs4;
  fs4.open(name,std::fstream::out );

  //preclients.DumpStats(consumers.Get (0),std::cout);
  for(unsigned int i=0; i<caches.GetN(); i++) {
    fs4 << "\nCached Node(cache" << i << ":n" << caches.Get(i)->GetId()<< "):\n";
    FateIpv4Helper::DumpStats(caches.Get(i), fs4);
  }
  
  fs4 << "\n";
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
  fs2 << input << "\t" << NodeContainer::GetGlobal().GetN() << "\t" << totlinks <<"\t" << format << "\t" << reqRate << "\t" << nFiles << "\t" << nSeg << "\t" << numClientPerNodes << "\t" << consumers.GetN() << "\t" << producers.GetN() << "\t" << seed << "\t" << totTime << "\t" << maxPkts << "\t" << std::setprecision (15) << alpha << "\t" << pTopo << "\t" << cTopo << "\t" << cConfig << "\t" << cConfig << "\t" << nConfig << "\n";
    fs2.close();
   

}

void createTopology(std::string input, std::string format, int ratePerSec) {
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

void prepNodes()
{
  //availNodes= NodeContainer::GetGlobal();
  for(unsigned int i=0; i<NodeContainer::GetGlobal().GetN() ; i++) 
  {    availNodes.push_back(i); }
  shuffle(availNodes);

}
void createProducers(unsigned int n)
{
	std::cout << "Producers:";
   for(unsigned int i=0; i<n; i++)
   {
       unsigned int num=availNodes.back();
       producers.Add(NodeContainer::GetGlobal().Get(num));
       availNodes.pop_back();
       std::cout << num << ",";
   } 
   std::cout << "\n";
   setProducers();
}
void createConsumers(unsigned int n)
{
	std::cout << "Consumers:";
   for(unsigned int i=0; i<n; i++)
   {
       unsigned int num=availNodes.back();
       consumers.Add(NodeContainer::GetGlobal().Get(num));
       availNodes.pop_back();
       std::cout << num << ",";
   } 
   std::cout << "\n";

}
void createCaches(unsigned int n)
{
	std::cout << "Caches:";
   for(unsigned int i=0; i<n; i++)
   {
       unsigned int num=availNodes.back();
       caches.Add(NodeContainer::GetGlobal().Get(num));
       availNodes.pop_back();
       std::cout << num << ",";
   } 
   std::cout << "\n";
}
void createAllCaches()
{
     caches=NodeContainer::GetGlobal();
}
int 
main (int argc, char *argv[])
{
  bool verbose = false;

Packet::EnablePrinting();
Packet::EnableChecking();
    input="scratch/ns3-ATT-topology.txt";
  //input="scratch/simple3.orb";
  //std::string input("./Inet/inet.3200");
  //format= "Orbis";
     format= "Inet";
  //std::string nput ("./rocketfuel/maps/1239/latencies.intra");
  //std::string input ("./src/topology-read/examples/RocketFuel_toposample_1239_weights.txt");
  //std::string format ("Rocketfuel");
  //std::string input("./src/topology-read/examples/Orbis_toposample.txt");
  //std::string format ("Orbis");
  reqRate=20;  //req in seconds
  nProd=10;
  //double nCons=100;
  nCons=60;
  //std::string pTopo("Single"); //single, or random
  //std::string cTopo("Edge"); //edge or random
  pTopo="Random";//Single"; //single, or random
  //std::string cTopo("Random"); //edge or random
  cTopo="Random";//Edge"; //edge or random
  cacheTopo="Random";//Edge"; //edge or random
  //std::string cacheTopo("All"); //edge or random
  nFiles = 50000;
  logName="logs/default";
  nSeg=1;
  alpha = 1;
  //std::string cConfig("");  //no config or name of xml file
   //cConfig="fateXmlConfigFiles/Ns3-node-configC.xml";  //no config or name of xml file
   cConfig="fateXmlConfigFiles/Lru-reroute3.xml";  //no config or name of xml file
  //std::string nConfig("");
  nConfig="fateXmlConfigFiles/nocache.xml";
  bool exclusiveContent=true; //either producers are exclusive in content or they all share the same
  numClientPerNodes=1;
   seed = 1;
   totTime=900; //0;
  maxPkts=1000000; //000;
  unsigned int cSize=10;

  CommandLine cmd;

  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("format", "Format to use for data input [Orbis|Inet|Rocketfuel].",
                format);
  cmd.AddValue ("input", "Name of the input file.",
                input);
  cmd.AddValue("reqRate", "Request Rate", reqRate);
  cmd.AddValue("producerTopo", "How producers are selected from topology", pTopo);
  cmd.AddValue("numProducers", "How many producer nodes are created", nProd);
  cmd.AddValue("consumerTopo", "How consumers are selected from topology", cTopo);
  cmd.AddValue("numConsumers", "How many consumer nodes are created", nCons);
  cmd.AddValue("cacheSize", "the size of each cache, for hashing", cSize);
  cmd.AddValue("numFiles", "How many files to request", nFiles);
  cmd.AddValue("numCaches", "How many caches to request", nCaches);
  cmd.AddValue("numSegments", "How many Segments per file", nSeg);
  cmd.AddValue("cacheNodeConfig", "config file name for cache nodes", cConfig);
  cmd.AddValue("nonCacheNodeConfig", "config file name for non cache nodes", nConfig);
  cmd.AddValue("exclusiveProducers", "each producer only servers content unique to itself", exclusiveContent);
  cmd.AddValue("zipfAlpha", "Alpha value for the zipf generator for consumers", alpha);
  //rnd seed
  //num client apps per node
  cmd.AddValue("numClientsPerNode", "How many clients per client-nodes", numClientPerNodes);
  cmd.AddValue("rndSeed", "Random Seed Value", seed);
  cmd.AddValue("time", "how long the network simulator should run, in seconds", totTime);
  cmd.AddValue("logName", "Log name",logName);
  cmd.AddValue("maxPkts", "maximum number of interest packets to send.  '0' = unlimited",maxPkts);
  cmd.AddValue("sendToOffPathCache", "redirect packets to cache", sendOff);
  cmd.AddValue("hashOnLocation", "hash to location or by name", hashLoc);
  
  cmd.Parse (argc,argv);
  std::cout << "\nsendOffPath:" << sendOff << ", hashloc:" << hashLoc << "\n";
  RngSeedManager::SetSeed(seed);
  if (!verbose)
    {
     LogComponentDisableAll(LOG_PREFIX_ALL);
  } else {
     PacketMetadata::Enable ();
     Packet::EnablePrinting ();
      LogComponentEnable ("UdpFateZipfClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpFateServerApplication", LOG_LEVEL_INFO);
    }
 Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("10Gbps"));
  p2p.SetQueue("ns3::DropTailQueue");
  //Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("1000000")); 
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("1ms"));
      createTopology(input, format, reqRate);
      //create1Producers(3);
    prepNodes();
    createProducers(nProd);
    createConsumers(nCons);
    createCaches(nCaches);

    createFateNodes(cConfig, nConfig);
    createAllConsumers();

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    CreateDnsAssociation();
    //cache hashing
    CreateDnsHashAssociation(caches,cSize);

    dns = GetDns();
    //CreateDestAssociation(producers);
    CreateDestAssociation(NodeContainer::GetGlobal());
    //how to do caching
    /*if (cConfig.size()) {
      mod = new UtilityExternalModule;
      timer = new GlobalModuleTimerNs3; //change to ns3 timer
      stats = new CustomStats;
      global = new GlobalModule;
      global->SetGlobalTimer(timer);
      //(mod->GetGlobalModule ()).SetGlobalLog ("default", new CoutModuleLog);
      global->SetGlobalLog("default", new Ns3InfoLog); //need a global file
      devMod = new DeviceModule;
      devMod->SetNodeStats(stats);

      global->dnsEntry = (void *) (dns);

        //createEdgeCacheNodes();
    }*/

    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (totTime+1.0));  //set as param FIXME

//createAllCacheNodes();
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (totTime+.5));

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
