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
#include "ForwardNs3Ipv4ManagerF2c.h"

//extern std::map<ns3::Ipv4Address, std::string > dnsIpToName;

ForwardNs3Ipv4ManagerF2C::ForwardNs3Ipv4ManagerF2C()
  : ForwardNs3Ipv4Manager::ForwardNs3Ipv4Manager(),
    m_dns(nullptr)
{}

ForwardNs3Ipv4ManagerF2C::ForwardNs3Ipv4ManagerF2C(ConfigWrapper &config)
  : ForwardNs3Ipv4Manager::ForwardNs3Ipv4Manager(config)
  , m_dns(nullptr)
{
  Config(config);
}

ForwardNs3Ipv4ManagerF2C::~ForwardNs3Ipv4ManagerF2C() {}

bool
ForwardNs3Ipv4ManagerF2C::OnInit(UtilityExternalModule *data)
{

  m_dns = (std::map<ns3::Ipv4Address, std::string> *)(data->GetGlobalModule()->dnsEntry);
  return ForwardNs3Ipv4Manager::OnInit(data);
}
bool
ForwardNs3Ipv4ManagerF2C::isThisMyIpAddr(const std::string &myName,  ns3::Ipv4Address addr)
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

double
ForwardNs3Ipv4ManagerF2C::PopNextDst(PktType &pkt, std::string &nextAddr)
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
      dstAddr = dstChain.substr(0,dstChain.find(';'));
    }


//add source to return address if missing
    std::string retChain;
    bool retChainExist = pkt.GetPrintedNamedAttribute("ReturnChain", retChain);
    if (retChain.find(srcAddr) == std::string::npos) //make sure dst is on the chain
    {
      retChain.append(srcAddr);
      retChain.push_back(';');
    }

    if (!dstChain.empty() && dstChain != dstAddr)
    {    

      std::string nextAddr;
      std::size_t found = dstChain.find(';');
      if (found != std::string::npos)
      {
      nextAddr = dstChain.substr(0,found); //we dont need \n
      }
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

void
ForwardNs3Ipv4ManagerF2C::OnPktIngress(PktType &pkt) {


  ForwardNs3Ipv4Manager::OnPktIngress(pkt); //normal forwarding

  std::string dstAddr;
  std::string srcAddr;
  double cacheHit = 0.0;
  bool existHit = pkt.GetNamedAttribute("CacheHit",cacheHit,true);
  //uint64_t tmp;
  //bool existServer = pkt.GetUnsignedNamedAttribute("ServerHitNodeName", tmp, false);
  bool existFwd = pkt.GetPrintedNamedAttribute("Ipv4Dst", dstAddr, true);
  pkt.GetPrintedNamedAttribute("Ipv4Src", srcAddr, true);
  const std::string nodeName= m_myNodeRootModule->Name() ; //m_externalModule->GetDevice()->GetNodeName();
  if (!((existHit && (cacheHit == 1.0)) || !existFwd)) { //cache hit or not forwarding, do SOP
    //update RoutingChain, if we are the destination
    std::string dstChain;
    bool routeExist = pkt.GetPrintedNamedAttribute("DstChain", dstChain);
    //if dstAddr does not exist on DstChain, add it (initial send)
    if (dstChain.find(dstAddr) == std::string::npos) //make sure dst is on the ncchain
    {
      dstChain.append(dstAddr);
      dstChain.push_back(';');
    }
 
    //
    if (!dstChain.empty() && dstChain != dstAddr)
    {
       
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
      }
    }
    pkt.SetPrintedNamedAttribute("Ipv4Dst", dstAddr, true);
    pkt.SetPrintedNamedAttribute("DstChain", dstChain); //dont want ";"  at start

  } else if (existFwd && existHit && (cacheHit== 1.0)) //cache hits return directly
  {
    //pkt.DeleteNamedAttribute("DstChain");
    //pkt.DeleteNamedAttribute("ReturnChain");
  }
  //ForwardNs3Ipv4Manager::OnPktIngress(pkt); //normal forwarding
}



