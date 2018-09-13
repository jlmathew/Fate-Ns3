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
#ifndef UDP_FATE_VIDEO_SERVER_H
#define UDP_FATE_VIDEO_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "udp-fate-server.h"

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup udpecho UdpEcho
 */

/**
 * \ingroup udpecho
 * \brief A Udp Echo server
 *
 * Every packet received is sent back.
 */
class UdpFateVideoServer : public UdpFateServer //Application 
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  UdpFateVideoServer ();
  virtual ~UdpFateVideoServer ();
void
SetPartialMatch(const std::list< std::pair< std::string, std::string > > &match);
void
PrintStats(std::ostream &os) const;
void
GetMatchNameAndPort(std::string &matchName, uint16_t &port);
protected:
  virtual void DoDispose (void);
  virtual bool getPktData(PktType &pkt, Address from);
private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  uint16_t m_port; //!< Port on which we listen for incoming packets.
  uint16_t m_size;
  Ptr<Socket> m_socket; //!< IPv4 Socket
  Ptr<Socket> m_socket6; //!< IPv6 Socket
  Address m_local; //!< local multicast address
  //std::list< std::pair< std::string, std::string> > m_partMatch;
  std::string m_setLocation;

  uint64_t m_statNumPktsTx; //
  uint64_t m_statNumIntPktRx; //
  uint64_t m_statNumDataPktTx; //
  
  uint64_t m_statNumNotMatchPktTx; //
  uint64_t m_statTotalDataTxSize;
  uint64_t m_statTotalNotMatchTxSize;
  uint64_t m_statTotalIntRxSize;
  uint64_t m_statNumErrorPkts;
  void parseHash(const std::string &name);
  bool m_encISlice, m_encVideo, m_encAudio, m_netEnc;
std::string m_setNameMatch;
struct vidInfo
{
   uint64_t hash;
   char frameType;
};
  std::map<uint32_t, vidInfo> m_IsliceHash;
  std::map<uint32_t, uint64_t> m_audioHash;
};

} // namespace ns3

#endif /* UDP_ECHO_SERVER_H */

