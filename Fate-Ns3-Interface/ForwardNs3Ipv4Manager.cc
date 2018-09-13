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
#include "ForwardNs3Ipv4Manager.h"



ForwardNs3Ipv4Manager::ForwardNs3Ipv4Manager()
  :m_cacheStore (NULL)
   , m_cacheStoreName("")
{}

  ForwardNs3Ipv4Manager::ForwardNs3Ipv4Manager(ConfigWrapper &config)
  : //ModuleManager::ModuleManager(config)
    m_cacheStore (NULL)
    ,m_cacheStoreName ("")
{
   Config(config);
}
  
  ForwardNs3Ipv4Manager::~ForwardNs3Ipv4Manager() {}
  
bool
ForwardNs3Ipv4Manager::OnInit (UtilityExternalModule * module)
{
  ModuleManager::OnInit (module);
  //get cache store association
  if ( (NULL == m_cacheStore) && m_cacheStoreName.length())
    {

      m_cacheStore = dynamic_cast < TypicalCacheStore * >((module->GetDevice ()->GetSelfNode ())->GetStore (m_cacheStoreName));

    }
   return true;
}

   void
  ForwardNs3Ipv4Manager::Config(ConfigWrapper &config) {
  ModuleManager::Config(config);
  if (!m_useAlias)
    {
      m_name = IdName ();
    }

  m_cacheStoreName = config.GetAttribute ("associatedCacheStore", m_cacheStoreName.c_str ());
   
  
  }

  //const std::string &
  //ForwardNs3Ipv4Manager::Name() const { return m_name; }

  void
  ForwardNs3Ipv4Manager::OnPktIngress(PktType &pkt) {

     ModuleManager::OnPktIngress (pkt);       //let utilities judge it
     //check if cache hit, if yes, dont send interest, send data
    double cacheHit = 0.0;
    bool exist = pkt.GetNamedAttribute("CacheHit",cacheHit, true);
     //reverse IP and port address
    uint64_t ipProto = 0, L4Proto= 0;
    uint64_t tos,ttl,id,ecn,dscp;
    std::string toAddr, fromAddr;
    uint64_t srcPort = 0, dstPort = 0;
    uint64_t hops = 0;
    std::string chain;
    bool exist2 = pkt.GetUnsignedNamedAttribute("TtlHop",hops, false);
    bool exist3 = pkt.GetNamedAttribute("NAMECHAIN",chain, false);
    PktType newPacket;
    PktTxStatus status=Sent;
    //No change if data or 'missed' interest packet
    if ((cacheHit == 1.0) && exist && m_cacheStore) {
      bool stat = m_cacheStore->ExistData(pkt.GetAcclName(), newPacket);
      //newPacket = pkt;
      newPacket.ClearTempData();
      newPacket.SetNamedAttribute("CacheHit", cacheHit , false);
      //newPacket.SetUnsignedNamedAttribute("CacheHit", 1 , false);
      if (exist2) {
        //write back the TTL hop
        newPacket.SetUnsignedNamedAttribute("TtlHop", hops , false);
      }
      if (exist3) {
        //write back the TTL hop
        newPacket.SetNamedAttribute("NAMECHAIN", chain , false);
      }
      newPacket.SetNamedAttribute("CacheHitNodeName", m_myNodeRootModule->Name() , false);
      //m_cacheStore->GetData(pkt.GetAcclName(), newPacket);
      pkt.GetUnsignedNamedAttribute("L3Proto", ipProto, true); 
      //copy all the header information from interest to data
      if (ipProto == 0x800) {
        pkt.GetUnsignedNamedAttribute("Tos", tos, true);
        pkt.GetUnsignedNamedAttribute("TTL", ttl, true);
        pkt.GetUnsignedNamedAttribute("Id", id, true);
        pkt.GetUnsignedNamedAttribute("Ecn", ecn, true);
        pkt.GetUnsignedNamedAttribute("Dscp", dscp, true); 
        pkt.GetPrintedNamedAttribute("Ipv4Src", toAddr, true); 
        pkt.GetPrintedNamedAttribute("Ipv4Dst", fromAddr, true); 
        pkt.GetUnsignedNamedAttribute("L4Proto", L4Proto, true); 

        //copy to new packet
        newPacket.SetPrintedNamedAttribute("Ipv4Src", fromAddr, true); 
        newPacket.SetPrintedNamedAttribute("Ipv4Dst", toAddr, true); 
        newPacket.SetUnsignedNamedAttribute("L3Proto", 0x800, true);
        newPacket.SetUnsignedNamedAttribute("Tos", tos, true);
        newPacket.SetUnsignedNamedAttribute("TTL", ttl, true);
        newPacket.SetUnsignedNamedAttribute("Id", id, true);
        newPacket.SetUnsignedNamedAttribute("Ecn", ecn, true);
        newPacket.SetUnsignedNamedAttribute("Dscp", dscp, true); 
        newPacket.SetUnsignedNamedAttribute("L4Proto", L4Proto, true); 
      }
      if ((L4Proto == 17)  || (L4Proto==6)) { 
        pkt.GetUnsignedNamedAttribute("SrcPort", srcPort, true);
        pkt.GetUnsignedNamedAttribute("DstPort", dstPort, true);
        newPacket.SetUnsignedNamedAttribute("SrcPort", dstPort, true);
        newPacket.SetUnsignedNamedAttribute("DstPort", srcPort, true);
      }
      status=InterestDropCacheHit; // | NotSentLowValue;
      //pkt = newPacket; //send 'hit' data packet
      //bool stat = m_cacheStore->ExistData(pkt.GetAcclName(), newPacket);
      if (!stat)
         status = Corrupt;
      pkt = newPacket; //send 'hit' data packet
    } 
    OnPktEgress(pkt, status);
  }

  


