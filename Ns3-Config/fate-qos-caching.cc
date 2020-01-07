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
NodeStats *stats;
GlobalModule *global;
DeviceModule *devMod;

std::string logName;
std::string fileLengths;
std::string fileLengthName;
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

std::string segstr[3];



void setProducers(void) {
  uint32_t numProd = producers.GetN();
  for(unsigned int i=0; i< numProd; i++)
  {
    //FIXME HERE
    //UdpFateServerHelper echoServer(100+i); //FIXME what if > 64k?
    UdpFateFileZipfServerHelper echoServer(100+i); //FIXME what if > 64k?
    echoServer.SetAttribute("ReturnSize", UintegerValue(500));
    std::string matchName="/test"+std::to_string(i+1);

    echoServer.SetAttribute("MinMatchName", StringValue(matchName));
    echoServer.SetAttribute("FileSizes", StringValue(segstr[i]));
    serverApps.Add( echoServer.Install(producers.Get(i)));
  }
  NS_LOG_INFO("actual post nFile servers are:" << serverApps.GetN());
}

void create1Producers(unsigned int nodeNum) {
    producers.Add(nodes.Get(nodeNum));
}


void createConsumers(void) {

  //add application to each
  unsigned int numCons = consumers.GetN();
  for(unsigned int i=0; i< numCons; i++)
  {
      UdpFateFileZipfClientHelper echoClient(ipic[0].GetAddress (1),9);   //based upon the name, it will map to an ip/port
      echoClient.SetAttribute ("MaxPackets", UintegerValue (maxPkts));
      echoClient.SetAttribute ("Interval", TimeValue (Seconds ((double) 1.0/reqRate)));
      //echoClient.SetAttribute ("NumSegments", UintegerValue (nSeg));
      echoClient.SetAttribute ("NumFiles", UintegerValue (nFiles));
      echoClient.SetAttribute ("QoS", UintegerValue (i+1));
      echoClient.SetAttribute ("AddTimeStamp", BooleanValue(true));
      echoClient.SetAttribute ("ZipfAlpha", DoubleValue(alpha));
      echoClient.SetAttribute ("NStaticDestination", BooleanValue(true)); //use the static index or not
      echoClient.SetAttribute ("matchByType", StringValue("location"));
      std::string match="/test"+std::to_string(i+1);
      echoClient.SetAttribute ("matchString", StringValue(match.c_str()));
     //echoClient.SetAttribute ("FileMultipart", BooleanValue(false));

      //broken FIXME TODO echoClient.SetAttribute ("matchByType", StringValue("location"));
echoClient.SetAttribute("FileMultipart", BooleanValue(true));

      PktType fatePkt;
      fatePkt.SetUnsignedNamedAttribute("Distance", 0);
      fatePkt.SetPacketPurpose(PktType::INTERESTPKT);
      IcnName<std::string> pktName;
      std::string conName="/test"+std::to_string(i+1);
      pktName.SetFullName(conName);
      fatePkt.SetName(pktName);
      std::stringstream out;
      out << fatePkt;
      echoClient.SetAttribute ("PktPayload", StringValue(out.str())); //use the static index or not
      //echoClient.SetAttribute ("test", StringValue("testing")); //use the static index or not
      clientApps.Add(echoClient.Install(consumers.Get(i)));
      //ApplicationContainer app = echoClient.Install(consumers.Get(i));
      //clientApps.Add(app);
      //Ptr<UdpFateZipfClient> zapp = DynamicCast<UdpFateZipfClient>(app.Get(j));
      //zapp->SetPktPayload(fatePkt);
  }
  
  //seems the only way to preload the packet type, is to do it here




}



void createFateNodes(const std::string &cConfig, const std::string &nConfig) {
  //cache only nodes are cConfig, all else are nConfig.
  UtilityConfigXml config;
  config.FirstNodeFileConfig(cConfig);
  FateIpv4Helper helper;
  helper.SetConfigFile(cConfig);
  stats= new CustomStats;
  GlobalModule *global = new GlobalModule;
  GlobalModuleTimerNs3 *timer = new GlobalModuleTimerNs3;
  global->SetGlobalTimer(timer);
  GlobalModuleLog *log = new CoutModuleLog;
  global->SetGlobalLog("default", log );
  helper.SetStats(stats);
  helper.SetLog(log);
  helper.SetGlobalModule(global);
  helper.Install(cachingNodes);


  helper.SetConfigFile(nConfig);
  helper.Install(nonCachingNodes);



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

  UdpFateFileZipfServerHelper echoServer(100);
  for(unsigned int i=0; i<producers.GetN(); i++) {
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
  for(unsigned int i=0; i<consumers.GetN(); i++) {
    fs3 << "\nConsumer Node(c" << i << ":n" << consumers.Get(i)->GetId()<< "):\n";
    echoConsumer.DumpStats(consumers.Get(i), fs3);
  }
  fs3 << "\n";
  fs3.close();


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

int
main (int argc, char *argv[])
{
  bool verbose = false;

  Packet::EnablePrinting();
  Packet::EnableChecking();
  //input="scratch/ns3-ATT-topology.txt";
  input="scratch/lrfu-size-qos-cache.orb";
  //std::string input("./Inet/inet.3200");
  format= "Orbis";
  //format= "Inet";
  //std::string nput ("./rocketfuel/maps/1239/latencies.intra");
  //std::string input ("./src/topology-read/examples/RocketFuel_toposample_1239_weights.txt");
  //std::string format ("Rocketfuel");
  //std::string input("./src/topology-read/examples/Orbis_toposample.txt");
  //std::string format ("Orbis");
  reqRate=1; //20; //20;  //req in seconds
  nProd=0;
  //double nCons=100;
  nCons=3;
  //std::string pTopo("Single"); //single, or random
  //std::string cTopo("Edge"); //edge or random
  pTopo="Single"; //single, or random
  //std::string cTopo("Random"); //edge or random
  cTopo="Edge"; //edge or random
  cacheTopo="Edge"; //edge or random
  //std::string cacheTopo("All"); //edge or random
  nFiles = 20; //10000;
  fileLengthName="fileLengths";
  nSeg=1;
  alpha = 1;
//FIXME HERE
cConfig="fateXmlConfigFiles/qoscacheA.xml";  //no config or name of xml file
  //cConfig="fateXmlConfigFiles/qoscacheB.xml";  //no config or name of xml file
  //cConfig="fateXmlConfigFiles/b.xml";  //no config or name of xml file
  //cConfig="fateXmlConfigFiles/c.xml";  //no config or name of xml file
  //cConfig="fateXmlConfigFiles/d.xml";  //no config or name of xml file
  //cConfig="fateXmlConfigFiles/b.xml";  //no config or name of xml file
  logName="logs/qoscacheA"; //"logs/default";
  //cConfig="fateXmlConfigFiles/Ns3-node.xml";
  nConfig="fateXmlConfigFiles/Ns3-node-qos.xml";
  bool exclusiveContent=true; //either producers are exclusive in content or they all share the same
  numClientPerNodes=1;
  seed = 1;
  totTime=81; //0;
  maxPkts=1000000; //000;
  std::string matchType="location"; //"filenum"; //"location";
  //std::string matchType="location"; //"location";

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
  cmd.AddValue("numFiles", "How many files to request", nFiles);
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
  cmd.AddValue("fileLengths", "file with string lengths",fileLengthName);
  cmd.AddValue("maxPkts", "maximum number of interest packets to send.  '0' = unlimited",maxPkts);
  cmd.Parse (argc,argv);

  for (unsigned int i=0 ; i<3; i++) {
  //parse fileLengthName into fileLengths
  if (fileLengthName.size()) {
    std::string strbuf;
    std::string fileName = fileLengthName+std::to_string(i)+".txt";
    std::ifstream is (fileName, std::ifstream::in);
    while (std::getline(is,strbuf))
    {
      segstr[i]  += strbuf;
      segstr[i] +="\n";
    }
    is.close();

  }
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
  createTopology(input, format, reqRate);
    create1Producers(7);
    create1Producers(15);
    create1Producers(11);

   for(unsigned int j=0; j< nodes.GetN(); j++)
{ 
  if (j == 1) //cache node
     continue;

  nonCachingNodes.Add(nodes.Get(j)); 
}
  cachingNodes.Add(nodes.Get(1));
  setProducers();

  consumers.Add(nodes.Get(0));
  consumers.Add(nodes.Get(2));
  consumers.Add(nodes.Get(3));


  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (totTime+1.0));  //set as param FIXME

  createFateNodes(cConfig, nConfig);
  createConsumers();
  clientApps.Start (Seconds (1.1));
  clientApps.Stop (Seconds (totTime+.5));
Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  CreateDestAssociation(producers);

  
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
