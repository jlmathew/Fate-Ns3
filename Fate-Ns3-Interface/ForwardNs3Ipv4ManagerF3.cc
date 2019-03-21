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
#include "ForwardNs3Ipv4ManagerF3.h"



ForwardNs3Ipv4ManagerF3::ForwardNs3Ipv4ManagerF3()
  :m_cacheStore (NULL)
  , m_cacheStoreName("")
  , m_dns(nullptr)
{}

ForwardNs3Ipv4ManagerF3::ForwardNs3Ipv4ManagerF3(ConfigWrapper &config)
  : //ModuleManager::ModuleManager(config)
    m_cacheStore (NULL)
  ,m_cacheStoreName ("")
  , m_dns(nullptr)
{
  Config(config);
}

ForwardNs3Ipv4ManagerF3::~ForwardNs3Ipv4ManagerF3() {}

bool
ForwardNs3Ipv4ManagerF3::OnInit (UtilityExternalModule * module)
{
  ModuleManager::OnInit (module);
  //get cache store association
  if ( (NULL == m_cacheStore) && m_cacheStoreName.length())
  {

    m_cacheStore = dynamic_cast < TypicalCacheStore * >((module->GetDevice ()->GetSelfNode ())->GetStore (m_cacheStoreName));

  }
  //should be a function to get DNS, but this works
  m_dns = (std::map<ns3::Ipv4Address, std::string> *)(module->GetGlobalModule()->dnsEntry);
  return true;
}

double
ForwardNs3Ipv4ManagerF3::PopNextDst(PktType &pkt, std::string &nextAddr)
{
  std::string dstAddr;
  std::string srcAddr;
  double nxtAddrHit=0.0;
  double cacheHit = 0.0;
  bool existHit = pkt.GetNamedAttribute("CacheHit",cacheHit, true);
  bool existFwd = pkt.GetPrintedNamedAttribute("Ipv4Dst", dstAddr, true);
  pkt.GetPrintedNamedAttribute("Ipv4Src", srcAddr, true);
  std::string dstChain;
  bool routeExist = pkt.GetPrintedNamedAttribute("DstChain", dstChain);

  //following forwarding conditions
  //next dst chain does not exist and does not auto create it

  //next dst chain does not exist and does auto create it

  //next dst chain does exist

  const std::string nodeName= m_myNodeRootModule->Name() ; //m_externalModule->GetDevice()->GetNodeName();
  if (!((existHit && (cacheHit == 1.0)) || !existFwd)) { //cache hit or not forwarding, do SOP
    //update RoutingChain, if we are the destination
    //if dstAddr does not exist on DstChain, add it (initial send)
    if (dstChain.find(dstAddr) == std::string::npos) //make sure dst is on the chain
    {
      dstChain.append(dstAddr);
      dstChain.push_back(';');
    }


//add source to return address if missing
    std::string retChain;
    bool retChainExist = pkt.GetPrintedNamedAttribute("ReturnChain", retChain);
    if (retChain.find(srcAddr) == std::string::npos) //make sure dst is on the chain
    {
      retChain.append(srcAddr);
      retChain.push_back(';');
    }


    std::string nextAddr;
    std::size_t found = dstChain.find(';');
    if (found != std::string::npos)
    {
      nextAddr = dstChain.substr(0,found); //we dont need \n
    }
    if (nextAddr != dstAddr) //dstAddr incorrect, redirect
    {
      dstAddr = nextAddr;
    }
    if (routeExist && (isThisMyIpAddr(nodeName, ns3::Ipv4Address(dstAddr.c_str())))) //check all IP addresses of node
    {

      //std::string retChain;
      //bool retChainExist = pkt.GetPrintedNamedAttribute("ReturnChain", retChain);
      if (retChainExist)  //so server can return via caches
      {
        nextAddr.push_back(';');
        nextAddr.append(retChain);
        //retChain.append(nextAddr);
        //retChain.push_back(';');
        pkt.SetPrintedNamedAttribute("ReturnChain", nextAddr);
      }
      //replace dst IP
      std::size_t found = dstChain.find(';');
      // dstChain = dstChain.substr(found+1); //pop off first match
      if (found != std::string::npos)
      {
        dstAddr = dstChain.substr(0,found);
      }
    }
    pkt.SetPrintedNamedAttribute("Ipv4Dst", dstAddr, true);
    pkt.SetPrintedNamedAttribute("DstChain", dstChain); //dont want ";"  at start

  } else if (existFwd && existHit && (cacheHit== 1.0)) //cache hits return directly
  {
    pkt.DeleteNamedAttribute("DstChain");
    pkt.DeleteNamedAttribute("ReturnChain");
  }
  return nxtAddrHit;

}



bool
ForwardNs3Ipv4ManagerF3::isThisMyIpAddr(const std::string &myName,  ns3::Ipv4Address addr)
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

void
ForwardNs3Ipv4ManagerF3::Config(ConfigWrapper &config) {
  ModuleManager::Config(config);
  if (!m_useAlias)
  {
    m_name = IdName ();
  }

  m_cacheStoreName = config.GetAttribute ("associatedCacheStore", m_cacheStoreName.c_str ());


}


void
ForwardNs3Ipv4ManagerF3::OnPktIngress(PktType &pkt) {
  ModuleManager::OnPktIngress (pkt);       //let utilities judge it
  //check if cache hit, if yes, dont send interest, send data
  double cacheHit = 0.0;
  bool exist = pkt.GetNamedAttribute("CacheHit",cacheHit, true);


  PktTxStatus status=Sent;
  //No change if data or 'missed' interest packet
  if ((cacheHit == 1.0) && exist && m_cacheStore) {
    //interest packets get changed to data packets
    if (pkt.GetPacketPurpose() & PktType::INTERESTPKT) {
      pkt.SetPacketPurpose(PktType::DATAPKT);
    }
    if (!ForwardChainPkt(pkt)) {
      uint64_t ipProto = 0, L4Proto= 0;
      bool exists = pkt.GetUnsignedNamedAttribute("L3Proto", ipProto, true);
      exists &= pkt.GetUnsignedNamedAttribute("L4Proto", L4Proto, true);

      //copy all the header information from interest to data
      if (exist && ipProto == 0x800 && ((L4Proto == 17) || (L4Proto ==6))) {
        IpUdpTcpReturn(pkt);
      }
    }
  }
  OnPktEgress(pkt, status);  //TODO FIXME should be called at node level
}

bool
ForwardNs3Ipv4ManagerF3::ForwardChainPkt(PktType &pkt)
{
  bool retVal = false;
  std::string dstChain;
  //reverse IP and port address
  std::string toAddr, fromAddr;
  bool routeExist = pkt.GetPrintedNamedAttribute("DstChain", dstChain);
  //if dstAddr does not exist on DstChain, add it (initial send)
  std::string dstAddr, srcAddr;
  if (dstChain.find(dstAddr) == std::string::npos) //make sure dst is on the ncchain
  {
    dstChain.append(dstAddr);
    dstChain.push_back(';');
  }
  std::string nextAddr;
  std::size_t found = dstChain.find(';');
  if (found != std::string::npos)
  {
    nextAddr = dstChain.substr(0,found); //we dont need \n
  }
  if (nextAddr != dstAddr) //dstAddr incorrect, redirect
  {
    dstAddr = nextAddr;
  }
  const std::string nodeName= m_myNodeRootModule->Name() ; //m_externalModule->GetDevice()->GetNodeName();
  bool ipFound = isThisMyIpAddr(nodeName, ns3::Ipv4Address(dstAddr.c_str()));
  if (routeExist && ipFound) {
    std::string retChain;
    bool retChainExist = pkt.GetPrintedNamedAttribute("ReturnChain", retChain);

    if (retChainExist)  //so server can return via caches
    {
      nextAddr.push_back(';');
      nextAddr.append(retChain);
      //retChain.append(nextAddr);
      //retChain.push_back(';');
      pkt.SetPrintedNamedAttribute("ReturnChain", nextAddr);
    }
    //replace dst IP
    std::size_t found = dstChain.find(';');
    // dstChain = dstChain.substr(found+1); //pop off first match
    if (found != std::string::npos)
    {
      dstChain = dstChain.substr(found+1);
      found = dstChain.find(';');
      if (found != std::string::npos)
      {
        dstAddr = dstChain.substr(0,found);
      }
      found = dstChain.find(';');
      if (found != std::string::npos)
      {
        dstAddr=dstChain.substr(0,found);
      }

      pkt.SetPrintedNamedAttribute("Ipv4Dst", dstAddr, true);
      pkt.SetPrintedNamedAttribute("DstChain", dstChain); //dont want ";"  at start
      retVal = true;
    }


  }
    return retVal;
}
//return packet,swap ip/ports for src/dst
  void
  ForwardNs3Ipv4ManagerF3::IpUdpTcpReturn(PktType &pkt)
  {
  //reverse IP and port address
  std::string toAddr, fromAddr;
  uint64_t srcPort = 0, dstPort = 0;
    pkt.GetPrintedNamedAttribute("Ipv4Src", toAddr, true);
    pkt.GetPrintedNamedAttribute("Ipv4Dst", fromAddr, true);

    //copy to new packet
    pkt.SetPrintedNamedAttribute("Ipv4Src", fromAddr, true);
    pkt.SetPrintedNamedAttribute("Ipv4Dst", toAddr, true);

    pkt.GetUnsignedNamedAttribute("SrcPort", srcPort, true);
    pkt.GetUnsignedNamedAttribute("DstPort", dstPort, true);
    pkt.SetUnsignedNamedAttribute("SrcPort", dstPort, true);
    pkt.SetUnsignedNamedAttribute("DstPort", srcPort, true);

  }




