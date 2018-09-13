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

#ifndef FATE_HEADER_H
#define FATE_HEADER_H

#include <stdint.h>
#include <vector>
#include "ns3/header.h"
#include "ns3/nstime.h"
#include "ns3/address.h"
#include "ns3/packet.h"
//#include "fate-address.h"

namespace ns3 {


class FateHeader : public Header
{
public:
  FateHeader ();
  virtual ~FateHeader ();


private:
  uint16_t m_packetLength;          //!< The packet length.
  //uint16_t m_packetSequenceNumber;  //!< The packet sequence number.
  //std::string m_src, m_dst;
  //FateAddress m_src, m_dst;
  uint8_t m_protocol;
public:
  /**
   * \brief Get the type ID.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  //virtual uint8_t GetProtocol(void) const;
};



}  // namespace fate, ns3

#endif /* fate_HEADER_H */

