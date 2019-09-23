/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 James Mathewson 
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
 * Author: James Mathewson <jlmathew@soe.ucsc.edu> 
 */

#include "global-fate.h"
#include "ns3/RangeData.h"

namespace ns3 {

static std::map<std::string, std::string> nodeNameToProd;
static std::map<std::string, ipPort_t > nodeNameToIPort;
static std::map<std::string, ipPort_t > matchNameToIPort;
//static std::multimap<std::string, Ipv4Address > dnsNameToIP;
static std::map<Ipv4Address, std::string > dnsIpToName;
static std::map<std::string, Ipv4Address > dnsNameToIp;

static std::map<std::string, RangeData<unsigned int> > cacheHashToName;
static unsigned int cacheHashModulus=0;
//std::map<ns3::Ipv4Address, std::string > dnsIpToName;
void CreateDnsAssociation()
{
  NodeContainer allNodes = NodeContainer::GetGlobal();  
  for(unsigned int i=0; i< allNodes.GetN(); i++) 
    {
     //get node name
     Ptr<Node> node = allNodes.Get(i);
     Ptr<Ipv4> ipv4 = node -> GetObject<Ipv4>(); 
     uint32_t numIntf = ipv4->GetNInterfaces();
       Ipv4Address loopback("127.0.0.1");
std::cout << "node" << i << " {";
     for(unsigned j=0; j<  numIntf; j++) 
     {
        Ipv4Address addr = ipv4->GetAddress(j,0).GetLocal();
        if (addr == loopback)
             continue;
     std::string name = "Node";
     name.append(std::to_string(i));
std::cout << addr << ",";
        dnsIpToName.insert(std::pair<Ipv4Address, std::string>(addr,name));
	dnsNameToIp.insert(std::pair<std::string, Ipv4Address>(name,addr));
        //dnsNameToIP.insert(std::pair<std::string, Ipv4Address>(name,addr));
     }
std::cout << "}\n";
    }
}



void CreateDnsHashAssociation(const NodeContainer &cacheNodes, unsigned int perCacheSetting)
{
   cacheHashModulus = cacheNodes.GetN()*perCacheSetting;
   std::cout << "Cache Nodes Hash Range:\n";
   for(unsigned int i=0; i<cacheNodes.GetN(); i++) {
     std::string name;
     name.append(std::to_string(cacheNodes.Get(i)->GetId()));
      ::RangeData<unsigned int> rng(i*perCacheSetting, (i+1)*perCacheSetting-1);  
      cacheHashToName[name]=rng;  
	std::cout << "\tNode:" << name << " , hash:" << rng << "\n";
   }
   std::cout << "}\n";


}

RangeData<unsigned int> 
GetCacheHashRange(std::string name)
{
    auto it=cacheHashToName.find(name);
    if (it == cacheHashToName.end())
	    assert(0);
    return it->second;

}

std::string 
GetNameFromRange(unsigned int rngOrig)
{
	std::string retName;
	unsigned int rng = rngOrig % cacheHashModulus;
  for(auto it=cacheHashToName.begin(); it!=cacheHashToName.end(); it++)
  {
     if (it->second.IsInRange(rng)) {
	retName=it->first; break;
		     } 
  }
  retName="Node"+retName;
	std::cout << "RngOrig:" << rngOrig << " , % " << cacheHashModulus << " = " << rng << " going to " << retName << "\n";
  return retName; 
}

Ipv4Address
GetIpFromRange(unsigned int rng)
{
   Ipv4Address retIp("0.0.0.0");
       //Ipv4Address loopback("127.0.0.1");
   std::string matchStr=GetNameFromRange(rng);
   if (!matchStr.empty())
   {
      std::map<std::string,std::pair<Ipv4Address, uint16_t>  >::const_iterator cit = nodeNameToIPort.find(matchStr);
  std::cout << "first node is " << nodeNameToIPort.begin()->first << " looking for " << matchStr << "\n";
      if (cit != nodeNameToIPort.end())
      {
         retIp=cit->second.first;
      }
   }
   std::cout << "   IP:" << retIp << " from hash value of " << rng << "\n";
   return retIp;
}

std::map<Ipv4Address, std::string> *
 GetDns()
{
  return &dnsIpToName;   

}
/*
bool isThisMyIpAddr(const std::string &myName,  Ipv4Address addr) 
{
    auto findRange = dnsNameToIP.equal_range(myName);
    bool findRetVal = false;
    for (auto cit = findRange.first; cit != findRange.second; ++cit)
    {
       if (cit->second == addr)
       {
          findRetVal = true;
          break;
       }
    }
    return findRetVal;
}

bool isThisMyIpAddr(const std::string &myName,  Ipv4Address addr) 
{
    auto findIp = dnsIpToName.find(addr);
    bool findRetVal = false;
    if (findIp->second == myName)
    {
       findRetVal = true;
    }
    return findRetVal;
}
*/
void CreateDestAssociation(const NodeContainer &producers)
{
  for(unsigned int i=0; i< producers.GetN(); i++) 
    {
     //get node name
     Ptr<Node> node = producers.Get(i);
     bool notFound=true;
     std::string name="Node";
     name += std::to_string(node->GetId());
     uint16_t port=0;
     //get node IP
     Ptr<Ipv4> ipv4 = node -> GetObject<Ipv4>(); 
     Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
     Ipv4Address addri = iaddr.GetLocal ();

     for(unsigned int j=0; j<node->GetNApplications(); j++)
     {
       
       Ptr<UdpFateServer> app = DynamicCast<UdpFateServer>(node->GetApplication(j));
      
       std::string match; 
       if (0 != app)  //fate server app 
       {
         notFound = false;
         app->GetMatchNameAndPort(match, port);
         std::cout << "CDA node:" << i << "(" << j << ") ipv4 producer:" << name << "= " << addri << "," << port <<"\n";
         ipPort_t info= std::make_pair(addri, port);
         nodeNameToIPort.insert(std::make_pair(name,info));
         matchNameToIPort.insert(std::make_pair(match,info));
       } else { //check video servers
        Ptr<UdpFateVideoServer> app = DynamicCast<UdpFateVideoServer>(node->GetApplication(j));
	if (0 != app) {
         notFound = false;
         std::string match; 
         app->GetMatchNameAndPort(match, port);
         //std::cout << "node:" << i << "(" << j << ") ipv4 producer:" << name << "= " << addri << "," << port <<"\n";
         ipPort_t info= std::make_pair(addri, port);
         nodeNameToIPort.insert(std::make_pair(name,info));
         matchNameToIPort.insert(std::make_pair(match,info));
        } else {
        Ptr<UdpFateFileZipfServer> app = DynamicCast<UdpFateFileZipfServer>(node->GetApplication(j));
	if (0 != app) {
         notFound = false;
         std::string match; 
         app->GetMatchNameAndPort(match, port);
         //std::cout << "node:" << i << "(" << j << ") ipv4 producer:" << name << "= " << addri << "," << port <<"\n";
         ipPort_t info= std::make_pair(addri, port);
         nodeNameToIPort.insert(std::make_pair(name,info));
         matchNameToIPort.insert(std::make_pair(match,info));
        } 

	}

       }
     }
     if (notFound) { //non producers
	 auto ci2=dnsNameToIp.find(name);
         if (ci2==dnsNameToIp.end()) { assert(0); }
         ipPort_t info= std::make_pair(ci2->second, 0); //no port
         nodeNameToIPort.insert(std::make_pair(name,info));
	 std::cout << name << " maps (non producer) maps to " << ci2->second << "\n";
         notFound=false;
       //assert(0); //didnt find information
     }
    } 
    //int size=nodeNameToIPort.size();
    //std:://cout << "there are " << size << " entries\n";
}

 std::string
GetProdNodeName(const std::string &bestMatchName)
{

   std::map<std::string, std::string>::const_iterator cit = nodeNameToProd.find(bestMatchName);
   if (cit == nodeNameToProd.end()) { //not found
     assert(0);
   } 
   return cit->second;
}

//trie structure would be best for this ...
ipPort_t 
GetProdNodeIpv4(const std::string &bestMatchName)
{
         //matchNameToIPort.insert(std::make_pair(match,info));
  std::map<std::string,std::pair<Ipv4Address, uint16_t>  >::const_iterator cit = matchNameToIPort.find(bestMatchName);
  //debug
  auto debug = matchNameToIPort.begin();
    std::cout << "access match there are " << debug->first << ":" << debug->second.first << " entries\n";
     if (cit == matchNameToIPort.end()) { //not found
     assert(0);
   } 
   return cit->second;

}
} // namespace ns3

