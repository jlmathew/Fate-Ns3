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
#include "ForwardNs3Ipv4Manager3.h"



ForwardNs3Ipv4Manager3::ForwardNs3Ipv4Manager3()
  :m_cacheStore (NULL)
  , m_cacheStoreName("")
{}

ForwardNs3Ipv4Manager3::ForwardNs3Ipv4Manager3(ConfigWrapper &config)
  : //ModuleManager::ModuleManager(config)
    m_cacheStore (NULL)
  ,m_cacheStoreName ("")
{
  Config(config);
}

ForwardNs3Ipv4Manager3::~ForwardNs3Ipv4Manager3() {}

bool
ForwardNs3Ipv4Manager3::OnInit (UtilityExternalModule * module)
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
ForwardNs3Ipv4Manager3::Config(ConfigWrapper &config) {
  ModuleManager::Config(config);
  if (!m_useAlias)
  {
    m_name = IdName ();
  }

  m_cacheStoreName = config.GetAttribute ("associatedCacheStore", m_cacheStoreName.c_str ());


}

//const std::string &
//ForwardNs3Ipv4Manager3::Name() const { return m_name; }

void
ForwardNs3Ipv4Manager3::OnPktIngress(PktType &pkt) {
  ModuleManager::OnPktIngress (pkt);       //let utilities judge it
  //check if cache hit, if yes, dont send interest, send data
  double cacheHit = 0.0;
  bool exist = pkt.GetNamedAttribute("CacheHit",cacheHit, true);

  //reverse IP and port address
  uint64_t ipProto = 0, L4Proto= 0;
  std::string toAddr, fromAddr;
  uint64_t srcPort = 0, dstPort = 0;

  PktTxStatus status=Sent;
  //No change if data or 'missed' interest packet
  if ((cacheHit == 1.0) && exist && m_cacheStore) {
    //interest packets get changed to data packets
    if (pkt.GetPacketPurpose() & PktType::INTERESTPKT) {
      pkt.SetPacketPurpose(PktType::DATAPKT);
    }
    pkt.GetUnsignedNamedAttribute("L3Proto", ipProto, true);
    //copy all the header information from interest to data
    if (ipProto == 0x800) {
      pkt.GetPrintedNamedAttribute("Ipv4Src", toAddr, true);
      pkt.GetPrintedNamedAttribute("Ipv4Dst", fromAddr, true);
      pkt.GetUnsignedNamedAttribute("L4Proto", L4Proto, true);

      //copy to new packet
      pkt.SetPrintedNamedAttribute("Ipv4Src", fromAddr, true);
      pkt.SetPrintedNamedAttribute("Ipv4Dst", toAddr, true);
    }
    if ((L4Proto == 17)  || (L4Proto==6)) {
      pkt.GetUnsignedNamedAttribute("SrcPort", srcPort, true);
      pkt.GetUnsignedNamedAttribute("DstPort", dstPort, true);
      pkt.SetUnsignedNamedAttribute("SrcPort", dstPort, true);
      pkt.SetUnsignedNamedAttribute("DstPort", srcPort, true);
    }
  }
  OnPktEgress(pkt, status);
}




