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

#include "fate-protocol.h"
#include "fate-header.h"
//#include "ipv4-interface.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FateL3");

NS_OBJECT_ENSURE_REGISTERED (FateL3);

  typedef NetDevice FateInterface;
TypeId 
FateL3::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FateL3")
    .SetParent<Object> ()
    .AddConstructor<FateL3> ()
    .SetGroupName ("Fate")
  ;
  return tid;
}

FateL3::FateL3 ()
{
  NS_LOG_FUNCTION (this);
}

FateL3::~FateL3 ()
{
  NS_LOG_FUNCTION (this);
}

void 
FateL3::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void FateL3::Send (Ptr<const Packet> packet, uint32_t deviceIdx, const Address &l2Dest, uint16_t proto)
 {
    Ptr<Node> node = this->GetObject<Node>();
std::cout << "ARI SEND on channel" << deviceIdx << ", node:" << node->GetId() << "!\n";
    Ptr<NetDevice> deviceEgress = node->GetDevice(deviceIdx);
    Ptr<Packet> p = packet->Copy();
    deviceEgress->Send(p, l2Dest, proto);
 }


void FateL3::Receive ( Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType) { 

Ptr<Node> node = device->GetNode();
//Ptr<NetDevice> device = node->GetDevice(deviceIdx);
uint32_t numDevices = node->GetNDevices();
std::cout << "devices:" << numDevices << "\n";
if (1 == numDevices) {
std::cout << node->GetId() << " FATE RX!\n";
} else {
   uint32_t idx = 1; 
   Send(p, idx, to, protocol);
}
}


void
FateL3::PrintFateL3 (Ptr<OutputStreamWrapper> stream)
{
  NS_LOG_FUNCTION (this << stream);
  //std::ostream* os = stream->GetStream ();
}


} // namespace ns3

