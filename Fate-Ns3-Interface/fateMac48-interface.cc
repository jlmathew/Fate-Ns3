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

#include "fateMac48-interface.h"
//#include "fate-l3-protocol.h"
#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/pointer.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FateMac48Interface");

NS_OBJECT_ENSURE_REGISTERED (FateMac48Interface);

TypeId 
FateMac48Interface::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FateMac48Interface")
    .SetParent<Object> ()
    .SetGroupName ("Fate")
  ;
  return tid;
}

/** 
 * By default, Fate interface are created in the "down" state
 *  with no IP addresses.  Before becoming useable, the user must 
 * invoke SetUp on them once an Fate address and mask have been set.
 */
FateMac48Interface::FateMac48Interface () 
  : m_ifup (false),
    m_forwarding (true),
    m_node (0), 
    m_device (0)
{
  NS_LOG_FUNCTION (this);
}

FateMac48Interface::~FateMac48Interface ()
{
  NS_LOG_FUNCTION (this);
}

void
FateMac48Interface::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_device = 0;
  Object::DoDispose ();
}

void 
FateMac48Interface::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
  DoSetup ();
}

void 
FateMac48Interface::SetDevice (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  m_device = device;
  DoSetup ();
}


void
FateMac48Interface::DoSetup (void)
{
  NS_LOG_FUNCTION (this);
  if (m_node == 0 || m_device == 0)
    {
      return;
    }
}

Ptr<NetDevice>
FateMac48Interface::GetDevice (void) const
{
  NS_LOG_FUNCTION (this);
  return m_device;
}

/*void
FateMac48Interface::SetMetric (uint16_t metric)
{
  NS_LOG_FUNCTION (this << metric);
  m_metric = metric;
}

uint16_t
FateMac48Interface::GetMetric (void) const
{
  NS_LOG_FUNCTION (this);
  return m_metric;
}*/


/**
 * These are IP interface states and may be distinct from 
 * NetDevice states, such as found in real implementations
 * (where the device may be down but IP interface state is still up).
 */
bool 
FateMac48Interface::IsUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifup;
}

bool 
FateMac48Interface::IsDown (void) const
{
  NS_LOG_FUNCTION (this);
  return !m_ifup;
}

void 
FateMac48Interface::SetUp (void)
{
  NS_LOG_FUNCTION (this);
  m_ifup = true;
}

void 
FateMac48Interface::SetDown (void)
{
  NS_LOG_FUNCTION (this);
  m_ifup = false;
}

bool 
FateMac48Interface::IsForwarding (void) const
{
  NS_LOG_FUNCTION (this);
  return m_forwarding;
}

void 
FateMac48Interface::SetForwarding (bool val)
{
  NS_LOG_FUNCTION (this << val);
  m_forwarding = val;
}


void
FateMac48Interface::Send (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << *p );
std::cout << "PACKET SEEN!\n"; 
  // is this packet aimed at a local interface ?
  /* for (FateMac48InterfaceAddressListCI i = m_ifaddrs.begin (); i != m_ifaddrs.end (); ++i)
    {
      if (dest == (*i).GetLocal ())
        {
          p->AddHeader (hdr);
          m_tc->Receive (m_device, p, FateL3Protocol::PROT_NUMBER,
                         m_device->GetBroadcast (),
                         m_device->GetBroadcast (),
                         NetDevice::PACKET_HOST);
          return;
        }
    } */
}


uint32_t
FateMac48Interface::GetNAddresses (void) const
{
  NS_LOG_FUNCTION (this);
  return 0; //m_ifaddrs.size ();
}

/*bool
FateMac48Interface::AddAddress (FateMac48InterfaceAddress addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_ifaddrs.push_back (addr);
  return true;
}

FateMac48InterfaceAddress
FateMac48Interface::GetAddress (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  if (index < m_ifaddrs.size ())
    {
      uint32_t tmp = 0;
      for (FateMac48InterfaceAddressListCI i = m_ifaddrs.begin (); i!= m_ifaddrs.end (); i++)
        {
          if (tmp  == index)
            {
              return *i;
            }
          ++tmp;
        }
    }
  else
    {
      NS_FATAL_ERROR ("index " << index << " out of bounds");  
    }
  FateMac48InterfaceAddress addr;
  return (addr);  // quiet compiler
}

FateMac48InterfaceAddress
FateMac48Interface::RemoveAddress (uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  if (index >= m_ifaddrs.size ())
    {
      NS_FATAL_ERROR ("Bug in FateMac48Interface::RemoveAddress");
    }
  FateMac48InterfaceAddressListI i = m_ifaddrs.begin ();
  uint32_t tmp = 0;
  while (i != m_ifaddrs.end ())
    {
      if (tmp  == index)
        {
          FateMac48InterfaceAddress addr = *i;
          m_ifaddrs.erase (i);
          return addr;
        }
      ++tmp;
      ++i;
    }
  NS_FATAL_ERROR ("Address " << index << " not found");
  FateMac48InterfaceAddress addr;
  return (addr);  // quiet compiler
}

FateMac48InterfaceAddress
FateMac48Interface::RemoveAddress(FateAddress address)
{
  NS_LOG_FUNCTION(this << address);

  if (address == address.GetLoopback())
    {
      NS_LOG_WARN ("Cannot remove loopback address.");
      return FateMac48InterfaceAddress();
    }

  for(FateMac48InterfaceAddressListI it = m_ifaddrs.begin(); it != m_ifaddrs.end(); it++)
    {
      if((*it).GetLocal() == address)
        {
          FateMac48InterfaceAddress ifAddr = *it;
          m_ifaddrs.erase(it);
          return ifAddr;
        }
    }
  return FateMac48InterfaceAddress();
}*/

} // namespace ns3

