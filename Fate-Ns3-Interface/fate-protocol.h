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
#ifndef FATE_L3_H
#define FATE_L3_H

#include <stdint.h>
#include <list>
#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/net-device.h"
//#include "fate-address.h"
#include "ns3/address.h"
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/sgi-hashmap.h"
#include "ns3/output-stream-wrapper.h"

namespace ns3 {

class NetDevice;
//class FateInterface;
class FateHeader;

/**
 * \ingroup arp
 * \brief An ARP cache
 *
 * A cached lookup table for translating layer 3 addresses to layer 2.
 * This implementation does lookups from IPv4 to a MAC address
 */
class FateL3 : public Object
{
private:
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  FateL3 (FateL3 const &);
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   * \returns
   */
  FateL3& operator= (FateL3 const &);

   virtual void DoDispose (void);
 /**
   * This function will notify other components connected to the node that a new stack member is now connected
   * This will be used to notify Layer 3 protocol of layer 4 protocol stack to connect them together.
   */
 //  virtual void NotifyNewAggregate ();

public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static const uint32_t PROT_NUMBER = 0x900;
  static TypeId GetTypeId (void);
  FateL3 ();
  virtual ~FateL3 ();
 /**
   * \param packet packet to send
   * \param source source address of packet
   * \param destination address of packet
   * \param protocol number of packet
   * \param route route entry
   *
   * Higher-level layers call this method to send a packet
   * down the stack to the MAC and PHY layers.
   */
  void Send (Ptr<const Packet> packet,  uint32_t deviceIdx, const Address &l2Dest, uint16_t proto);

  void Receive ( Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);

  /*bool IsUp (uint32_t i) const;
  void SetUp (uint32_t i);
  void SetDown (uint32_t i);
  bool IsForwarding (uint32_t i) const;
  void SetForwarding (uint32_t i, bool val);*/


  /**
   * \brief Print the ARP cache entries
   *
   * \param stream the ostream the ARP cache entries is printed to
   */
  void PrintFateL3 (Ptr<OutputStreamWrapper> stream);


 
  };
} // namespace ns3

#endif /* ARP_CACHE_H */
