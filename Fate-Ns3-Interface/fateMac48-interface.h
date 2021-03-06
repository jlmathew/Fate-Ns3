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
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef FATE_INTERFACE_H
#define FATE_INTERFACE_H

#include <list>
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/address.h"

namespace ns3 {

class NetDevice;
class Packet;
class Node;
class FateHeader;

/**
 * \ingroup fate
 *
 * \brief The IPv4 representation of a network interface
 *
 * This class roughly corresponds to the struct in_device
 * of Linux; the main purpose is to provide address-family
 * specific information (addresses) about an interface.
 *
 * By default, Fate interface are created in the "down" state
 * no IP addresses.  Before becoming usable, the user must
 * add an address of some type and invoke Setup on them.
 */
class FateMac48Interface  : public Object
{
public:
  /**
   * \brief Get the type ID
   * \return type ID
   */
  static TypeId GetTypeId (void);

  FateMac48Interface ();
  virtual ~FateMac48Interface();

  /**
   * \brief Set node associated with interface.
   * \param node node
   */
  void SetNode (Ptr<Node> node); 
  /**
   * \brief Set the NetDevice.
   * \param device NetDevice
   */
  void SetDevice (Ptr<NetDevice> device);

  /**
   * \returns the underlying NetDevice. This method cannot return zero.
   */
  Ptr<NetDevice> GetDevice (void) const;


  /**
   * These are IP interface states and may be distinct from 
   * NetDevice states, such as found in real implementations
   * (where the device may be down but IP interface state is still up).
   */
  /**
   * \returns true if this interface is enabled, false otherwise.
   */
  bool IsUp (void) const;

  /**
   * \returns true if this interface is disabled, false otherwise.
   */
  bool IsDown (void) const;

  /**
   * Enable this interface
   */
  void SetUp (void);

  /**
   * Disable this interface
   */
  void SetDown (void);

  /**
   * \returns true if this interface is enabled for IP forwarding of input datagrams
   */
  bool IsForwarding (void) const;

  /**
   * \param val Whether to enable or disable IP forwarding for input datagrams
   */
  void SetForwarding (bool val);

  /**
   * \param p packet to send
   * \param hdr IPv4 header
   * \param dest next hop address of packet.
   *
   * This method will eventually call the private
   * SendTo method which must be implemented by subclasses.
   */ 
  void Send (Ptr<Packet> p);

  /**
   * \param address The FateInterfaceAddress to add to the interface
   * \returns true if succeeded
   */
 // bool AddAddress (FateInterfaceAddress address);

  /**
   * \param index Index of FateInterfaceAddress to return
   * \returns The FateInterfaceAddress address whose index is i
   */
  //FateInterfaceAddress GetAddress (uint32_t index) const;

  /**
   * \returns the number of FateInterfaceAddresss stored on this interface
   */
  uint32_t GetNAddresses (void) const;

  /**
   * \param index Index of FateInterfaceAddress to remove
   * \returns The FateInterfaceAddress address whose index is index 
   */
  //FateInterfaceAddress RemoveAddress (uint32_t index);

  /**
   * \brief Remove the given Fate address from the interface.
   * \param address The Fate address to remove
   * \returns The removed Fate interface address 
   * \returns The null interface address if the interface did not contain the 
   * address or if loopback address was passed as argument
   */
  //FateInterfaceAddress RemoveAddress (FateAddress address);

protected:
  virtual void DoDispose (void);
private:
  /**
   * \brief Initialize interface.
   */
  void DoSetup (void);



  bool m_ifup; //!< The state of this interface
  bool m_forwarding;  //!< Forwarding state.
  uint16_t m_metric;  //!< Interface metric
  //FateInterfaceAddressList m_ifaddrs; //!< Address list
  Ptr<Node> m_node; //!< The associated node
  Ptr<NetDevice> m_device; //!< The associated NetDevice
};

} // namespace ns3

#endif
