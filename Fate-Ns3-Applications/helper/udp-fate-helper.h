/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 James Mathewson 
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

#ifndef UDP_FATE_HELPER_H
#define UDP_FATE_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/IcnName.h"
#include "ns3/PacketTypeBase.h"

namespace ns3 {
typedef std::pair<Ipv4Address, uint16_t> ipPort;



class UdpFateVideoServerHelper 
{
public:
  /**
   * Create UdpFateServerHelper which will make life easier for people trying
   * to set up simulations with echos.
   *
   * \param port The port the server will wait on for incoming packets
   */
  UdpFateVideoServerHelper (uint16_t port);

void SetPartialMatch(Ptr<Application> app, const std::list< std::pair< std::string, std::string> > &match);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
void SetTimestamp (Ptr<Application> app, bool timestamp);
void DumpStats(Ptr<Node> node, std::ostream &os) const;
;
  /**
   * Create a UdpFateServerApplication on the specified Node.
   *
   * \param node The node on which to create the Application.  The node is
   *             specified by a Ptr<Node>.
   *
   * \returns An ApplicationContainer holding the Application created,
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a UdpFateServerApplication on specified node
   *
   * \param nodeName The node on which to create the application.  The node
   *                 is specified by a node name previously registered with
   *                 the Object Name Service.
   *
   * \returns An ApplicationContainer holding the Application created.
   */
  ApplicationContainer Install (std::string nodeName) const;

  /**
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   *
   * Create one udp echo server application on each of the Nodes in the
   * NodeContainer.
   *
   * \returns The applications created, one Application per Node in the 
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::UdpFateServer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an UdpFateServer will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory; //!< Object factory.
  bool m_timestamp;
};



/**
 * \ingroup udpecho
 * \brief Create a server application which waits for input UDP packets
 *        and sends them back to the original sender.
 */
class UdpFateServerHelper
{
public:
  /**
   * Create UdpFateServerHelper which will make life easier for people trying
   * to set up simulations with echos.
   *
   * \param port The port the server will wait on for incoming packets
   */
  UdpFateServerHelper (uint16_t port);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
void SetTimestamp (Ptr<Application> app, bool timestamp);
void DumpStats(Ptr<Node> node, std::ostream &os) const;

void
SetPartialMatch(Ptr<Application> app, const std::list< std::pair< std::string, std::string> > &match);
  /**
   * Create a UdpFateServerApplication on the specified Node.
   *
   * \param node The node on which to create the Application.  The node is
   *             specified by a Ptr<Node>.
   *
   * \returns An ApplicationContainer holding the Application created,
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a UdpFateServerApplication on specified node
   *
   * \param nodeName The node on which to create the application.  The node
   *                 is specified by a node name previously registered with
   *                 the Object Name Service.
   *
   * \returns An ApplicationContainer holding the Application created.
   */
  ApplicationContainer Install (std::string nodeName) const;

  /**
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   *
   * Create one udp echo server application on each of the Nodes in the
   * NodeContainer.
   *
   * \returns The applications created, one Application per Node in the 
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::UdpFateServer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an UdpFateServer will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory; //!< Object factory.
  bool m_timestamp;
};
class UdpFateVideoClientHelper
{
public:
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * not include a port value (e.g., Ipv4Address and Ipv6Address).
   *
   * \param ip The IP address of the remote udp echo server
   * \param port The port number of the remote udp echo server
   */
  UdpFateVideoClientHelper (Address ip, uint16_t port);
  UdpFateVideoClientHelper ();
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
   *
   * \param addr The address of the remote udp echo server
   */
  UdpFateVideoClientHelper (Address addr);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
void SetTimestamp (Ptr<Application> app, bool timestamp);

   void DumpStats(Ptr<Node> node, std::ostream &os) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a Ptr<Node>.
   *
   * \param node The Ptr<Node> on which to create the UdpFateClientApplication.
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a string name of a Node that has been previously 
   * associated using the Object Name Service.
   *
   * \param nodeName The name of the node on which to create the UdpFateClientApplication
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (std::string nodeName) const;

  /**
   * \param c the nodes
   *
   * Create one udp echo client application on each of the input nodes
   *
   * \returns the applications created, one application per input node.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::UdpFateClient on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an UdpFateClient will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.
  bool m_timestamp;
  bool uninit;
};

/**
 * \ingroup udpecho
 * \brief Create an application which sends a UDP packet and waits for an echo of this packet
 */
class UdpFateCbrClientHelper
{
public:
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * not include a port value (e.g., Ipv4Address and Ipv6Address).
   *
   * \param ip The IP address of the remote udp echo server
   * \param port The port number of the remote udp echo server
   */
  UdpFateCbrClientHelper (Address ip, uint16_t port);
  UdpFateCbrClientHelper ();
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
   *
   * \param addr The address of the remote udp echo server
   */
  UdpFateCbrClientHelper (Address addr);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
void SetTimestamp (Ptr<Application> app, bool timestamp);

  void SetPktPayload (Ptr<Application> app, const std::string &xml);
  void SetPktPayload (Ptr<Application> app, const IcnName<std::string> &name);
  void SetPktPayload (Ptr<Application> app, const PktType &payload);
   void SetPktPayload (Ptr<Application> app, uint8_t *fill, uint32_t dataLength);
   void DumpStats(Ptr<Node> node, std::ostream &os) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a Ptr<Node>.
   *
   * \param node The Ptr<Node> on which to create the UdpFateClientApplication.
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a string name of a Node that has been previously 
   * associated using the Object Name Service.
   *
   * \param nodeName The name of the node on which to create the UdpFateClientApplication
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (std::string nodeName) const;

  /**
   * \param c the nodes
   *
   * Create one udp echo client application on each of the input nodes
   *
   * \returns the applications created, one application per input node.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::UdpFateClient on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an UdpFateClient will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.
  bool m_timestamp;
  bool uninit;
};

/**
 * \ingroup udpecho
 * \brief Create an application which sends a UDP packet and waits for an echo of this packet
 */
class UdpFateZipfClientHelper
{
public:
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * not include a port value (e.g., Ipv4Address and Ipv6Address).
   *
   * \param ip The IP address of the remote udp echo server
   * \param port The port number of the remote udp echo server
   */
  UdpFateZipfClientHelper();
  UdpFateZipfClientHelper (Address ip, uint16_t port);
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
   *
   * \param addr The address of the remote udp echo server
   */
  UdpFateZipfClientHelper (Address addr);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  void SetTimestamp (Ptr<Application> app, bool timestamp);

void
SetPktPayload (Ptr<Node> node, unsigned int appNum, const PktType &payload );

  void SetPktPayload (Ptr<Application> app, const std::string &xml);
  void SetPktPayload (Ptr<Application> app, const IcnName<std::string> &name);
  void SetPktPayload (Ptr<Application> app, const PktType &payload);
   void SetPktPayload (Ptr<Application> app, uint8_t *fill, uint32_t dataLength);
   void DumpStats(Ptr<Node> node, std::ostream &os) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a Ptr<Node>.
   *
   * \param node The Ptr<Node> on which to create the UdpFateClientApplication.
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a string name of a Node that has been previously 
   * associated using the Object Name Service.
   *
   * \param nodeName The name of the node on which to create the UdpFateClientApplication
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (std::string nodeName) const;

  /**
   * \param c the nodes
   *
   * Create one udp echo client application on each of the input nodes
   *
   * \returns the applications created, one application per input node.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::UdpFateClient on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an UdpFateClient will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.
  bool m_timestamp;
  bool uninit;
};

/**
 * \ingroup udpecho
 * \brief Create an application which sends a UDP packet and waits for an echo of this packet
 */
class UdpFateFileClientHelper
{
public:
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * not include a port value (e.g., Ipv4Address and Ipv6Address).
   *
   * \param ip The IP address of the remote udp echo server
   * \param port The port number of the remote udp echo server
   */
  UdpFateFileClientHelper (Address ip, uint16_t port);
  UdpFateFileClientHelper (ipPort(*getAddrPort)(const std::string &name));
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
   *
   * \param addr The address of the remote udp echo server
   */
  UdpFateFileClientHelper (Address addr);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
void SetTimestamp (Ptr<Application> app, bool timestamp);


  void SetPktPayload (Ptr<Application> app, const std::string &xml);
  void SetPktPayload (Ptr<Application> app, const IcnName<std::string> &name);
  void SetPktPayload (Ptr<Application> app, const PktType &payload);
   void SetPktPayload (Ptr<Application> app, uint8_t *fill, uint32_t dataLength);
   void DumpStats(Ptr<Node> node, std::ostream &os) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a Ptr<Node>.
   *
   * \param node The Ptr<Node> on which to create the UdpFateClientApplication.
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a string name of a Node that has been previously 
   * associated using the Object Name Service.
   *
   * \param nodeName The name of the node on which to create the UdpFateClientApplication
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (std::string nodeName) const;

  /**
   * \param c the nodes
   *
   * Create one udp echo client application on each of the input nodes
   *
   * \returns the applications created, one application per input node.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::UdpFateClient on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an UdpFateClient will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.
  bool m_timestamp;
  bool uninit;
};
class UdpFateVideoClientHelper2
{
public:
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * not include a port value (e.g., Ipv4Address and Ipv6Address).
   *
   * \param ip The IP address of the remote udp echo server
   * \param port The port number of the remote udp echo server
   */
  UdpFateVideoClientHelper2();
  UdpFateVideoClientHelper2 (Address ip, uint16_t port);
  /**
   * Create UdpFateClientHelper which will make life easier for people trying
   * to set up simulations with echos. Use this variant with addresses that do
   * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
   *
   * \param addr The address of the remote udp echo server
   */
  UdpFateVideoClientHelper2 (Address addr);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  void SetTimestamp (Ptr<Application> app, bool timestamp);

void
SetPktPayload (Ptr<Node> node, unsigned int appNum, const PktType &payload );

  void SetPktPayload (Ptr<Application> app, const std::string &xml);
  void SetPktPayload (Ptr<Application> app, const IcnName<std::string> &name);
  void SetPktPayload (Ptr<Application> app, const PktType &payload);
   void SetPktPayload (Ptr<Application> app, uint8_t *fill, uint32_t dataLength);
   void DumpStats(Ptr<Node> node, std::ostream &os) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a Ptr<Node>.
   *
   * \param node The Ptr<Node> on which to create the UdpFateClientApplication.
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a udp echo client application on the specified node.  The Node
   * is provided as a string name of a Node that has been previously 
   * associated using the Object Name Service.
   *
   * \param nodeName The name of the node on which to create the UdpFateClientApplication
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (std::string nodeName) const;

  /**
   * \param c the nodes
   *
   * Create one udp echo client application on each of the input nodes
   *
   * \returns the applications created, one application per input node.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::UdpFateClient on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an UdpFateClient will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.
  bool m_timestamp;
  bool uninit;
};

} // namespace ns3

#endif /* UDP_ECHO_HELPER_H */
