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

#include <cmath>

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
//#include "fate-address.h"
#include "fate-header.h"
#include "ns3/address-utils.h"
#include <iostream>     // std::cout, std::right, std::endl
#include <iomanip>      // std::setw
#include "ns3/buffer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FateHeader");


//using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED (FateHeader);

FateHeader::FateHeader ()
{
}

FateHeader::~FateHeader ()
{
}

TypeId
FateHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FateHeader")
    .SetParent<Header> ()
    .SetGroupName ("fate")
    .AddConstructor<FateHeader> ()
  ;
  return tid;
}
TypeId
FateHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
FateHeader::GetSerializedSize (void) const
{
  return 2;
//return 18;
}

void
FateHeader::Print (std::ostream &os) const
{
  os << 0x900;
}

void
FateHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU16 (0x900);
/*   i.WriteU8(m_dst.size());
  WriteTo(i, m_dst);
  i.WriteU8(m_src.size());
  WriteTo(i, m_src);
  i.WriteHtonU16 (m_packetSequenceNumber);
  */
  i.WriteHtonU16 (m_packetLength);
}

uint32_t
FateHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint16_t proto = i.ReadNtohU16();
  NS_ASSERT(proto == 0x900);

  m_packetLength  = i.ReadNtohU16 ();
  return GetSerializedSize ();
}

}  // namespace fate, ns3

