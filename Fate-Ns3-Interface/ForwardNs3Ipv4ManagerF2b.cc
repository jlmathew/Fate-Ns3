/*
 * Copyright (c) 2017 james mathewson
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
#include "ForwardNs3Ipv4ManagerF2b.h"

//extern std::map<ns3::Ipv4Address, std::string > dnsIpToName;

ForwardNs3Ipv4ManagerF2B::ForwardNs3Ipv4ManagerF2B()
  : ForwardNs3Ipv4Manager::ForwardNs3Ipv4Manager(),
    m_dns(nullptr)
{}

ForwardNs3Ipv4ManagerF2B::ForwardNs3Ipv4ManagerF2B(ConfigWrapper &config)
  : ForwardNs3Ipv4Manager::ForwardNs3Ipv4Manager(config)
  , m_dns(nullptr)
{
  Config(config);
}

ForwardNs3Ipv4ManagerF2B::~ForwardNs3Ipv4ManagerF2B() {}

bool
ForwardNs3Ipv4ManagerF2B::OnInit(UtilityExternalModule *data)
{

  m_dns = (std::map<ns3::Ipv4Address, std::string> *)(data->GetGlobalModule()->dnsEntry);
  return ForwardNs3Ipv4Manager::OnInit(data);
}
bool
ForwardNs3Ipv4ManagerF2B::isThisMyIpAddr(const std::string &myName,  ns3::Ipv4Address addr)
{
  assert(m_dns);
  auto findIp = m_dns->find(addr);
  bool findRetVal = false;
  if (findIp->second == myName)
  {
    findRetVal = true;
  }
  return findRetVal;
}

//Forward only, no caching
void
ForwardNs3Ipv4ManagerF2B::OnPktIngress(PktType &pkt) {


  ForwardNs3Ipv4Manager::OnPktIngress(pkt); //normal forwarding

  std::string dstAddr;
  std::string srcAddr;
  pkt.GetPrintedNamedAttribute("Ipv4Dst", dstAddr, true);
  pkt.GetPrintedNamedAttribute("Ipv4Src", srcAddr, true);
    std::string retChain="";
    pkt.GetPrintedNamedAttribute("ReturnChain", retChain);

  //if ReturnChain is empty, copy src to it
  if (!retChain.length()) {
     retChain=srcAddr+";";
     pkt.SetPrintedNamedAttribute("ReturnChain", retChain);
  }
  //assume dstChain is fixed
    std::string dstChain;
    bool routeExist = pkt.GetPrintedNamedAttribute("DstChain", dstChain);
    if (!routeExist)
	    return;

    std::size_t found = dstChain.find(';');
    if (found == std::string::npos)
	    return;

    std::string ip=dstChain.substr(0,found);



//first IP should be same as destination, if not switch it
    if ( (ip != (dstAddr))) // || (dstChain.find(dstAddr) == std::string::npos))
    {
         dstChain=dstChain+dstAddr+";";
	 dstAddr=ip;
    }
  const std::string nodeName= m_myNodeRootModule->Name() ; 
    //Is the IP our destination?   Pop it, and put new destination.

    if (isThisMyIpAddr(nodeName, ns3::Ipv4Address(dstAddr.c_str()))) {
       retChain =  ip+";"+retChain;
       dstChain = dstChain.substr(found+1);
       //if there is a next destination, update IP dest field
       std::size_t nextIpLoc=dstChain.find(";");
       if (nextIpLoc != std::string::npos) {
         ip=dstChain.substr(0,nextIpLoc);
         pkt.SetPrintedNamedAttribute("Ipv4Dst", ip, true);
       }
        pkt.SetPrintedNamedAttribute("ReturnChain", retChain);
        pkt.SetPrintedNamedAttribute("DstChain", dstChain);
	std::cout << "At node " << nodeName << "fixing paths\n";
    }
    std::cout << "node " << nodeName << " new dstchain is " << dstChain << ", retchain " << retChain << "\n";
    
}



