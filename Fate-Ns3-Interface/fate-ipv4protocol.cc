/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/names.h"

#include "fate-ipv4protocol.h"
#include "fate-header.h"

//#include "ipv4-interface.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FateIpv4L3");

NS_OBJECT_ENSURE_REGISTERED (FateIpv4L3);

TypeId 
FateIpv4L3::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FateIpv4L3")
    .SetParent<Object> ()
    .AddConstructor<FateIpv4L3> ()
    .SetGroupName ("Fate")
  ;
  return tid;
}

FateIpv4L3::FateIpv4L3 ()
{
  NS_LOG_FUNCTION (this);
}

FateIpv4L3::~FateIpv4L3 ()
{
  NS_LOG_FUNCTION (this);
}

void 
FateIpv4L3::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}


void FateIpv4L3::Receive ( Ptr<NetDevice> device, std::vector<uint8_t> &pv, uint16_t protocol, const Address &from, const Address &to, ns3::NetDevice::PacketType packetType, bool promis) { 
  
  //Ptr<Node> node = device->GetNode();
  //convert ns3 packet to fate packet
  Ptr<Packet> p = Create<Packet>((uint8_t *)&pv[0], pv.size(), true);
  
  PktType fatePkt;
  bool valid = FateIpv4Interface::Ipv4ToFatePkt(p, protocol, fatePkt);

  if (valid) {
     //call fate
     GetFateNode()->OnPktIngress(fatePkt);
     //convert fate packet to ns3
//std::cout << " Pre convert original pkt:";
//p->Print(std::cout);
//std::cout << "\n";
//Ptr<Packet> p2 = 0;    
    valid = FateIpv4Interface::FatePktToIpv4(fatePkt, p);
//std::cout << " Post convert original pkt:";
//p2->Print(std::cout);
//std::cout << "\n";
    
    if (!valid) {
       NS_ASSERT("Invalid fate to ipv4 conversion");
    }
    pv.clear();
    //set size
    uint32_t ms = p->GetSerializedSize();
    pv.resize(ms);
    p->Serialize((uint8_t *) &pv[0],ms);
  }
}


void
FateIpv4L3::PrintFateIpv4L3 (Ptr<OutputStreamWrapper> stream)
{
  NS_LOG_FUNCTION (this << stream);
  //std::ostream* os = stream->GetStream ();
}


void
FateIpv4L3::InsertFateNode(NodeManager* fateNode) 
{
   m_fateNode = fateNode;
}

NodeManager *
FateIpv4L3::GetFateNode() {
   return m_fateNode;
}

void
FateIpv4L3::InstantiateFate(UtilityExternalModule *mod)
{
    if (m_fateNode) {
      m_fateNode->OnInit(mod);
    } else {
      //assert FiXME TODO
    }
}

} // namespace ns3

