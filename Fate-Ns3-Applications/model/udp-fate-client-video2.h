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


#ifndef UDP_FATE_VIDEO_CLIENT2_H
#define UDP_FATE_VIDEO_CLIENT2_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/random-variable-stream.h"
#include "ns3/integer.h"
#include "ns3/global-fate.h"
#include <vector>
#include "ns3/fateIpv4-interface.h"

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup udpecho
 * \brief A Udp Echo client
 *
 * Every packet sent should be returned by the server and received here.
 */
class UdpFateVideoClient2 : public Application 
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  UdpFateVideoClient2 ();

  virtual ~UdpFateVideoClient2 ();

  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Address ip, uint16_t port);
  /**
   * \brief set the remote address
   * \param addr remote address
   */
  void SetRemote (Address addr);


  void SetPktPayload (const std::string &xml);

  void SetPktPayload (const PktType &payload);

  void SetPktPayload (uint8_t *fill, uint32_t dataSize);

  void SetPktPayload(const IcnName<std::string> &name);

  void SetTimestamp(bool timestamp);

  uint32_t
  GetDataSize() const;
void
PrintStats(std::ostream &os) const;

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
void parseTiming(const std::string &file, bool video);

void decrypt(PktType &pkt, Address from);
void playPkt( PktType &pkt);
void initPlayPkt();
 
  /**
   * \brief Schedule the next packet transmission
   * \param dt time interval between packets.
   */
  void ScheduleTransmit (Time dt);
  /**
   * \brief Send a packet
   */
  void Send (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  uint32_t m_count; //!< Maximum number of packets the application will send
  Time m_interval; //!< Packet inter-send time
  uint32_t m_size; //!< Size of the sent packet
  uint32_t m_segSize;
  uint32_t m_dataSize; //!< packet payload size (must be equal to m_size)
  uint8_t *m_data; //!< packet payload data

  uint32_t m_sent; //!< Counter for sent packets
  Ptr<Socket> m_socket; //!< Socket
  std::vector<Ptr<Socket> > m_vectSocket;
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet

  /// Callbacks for tracing the packet Tx events
  TracedCallback<Ptr<const Packet> > m_txTrace;

  PktType m_payload; //!< Fate packet payload
void getAudioFile (void);
void getVideoFile (void);
std::string getVideoSdp (void);
void Sendv(Ptr<Packet> p);
void Senda(Ptr<Packet> p);
void HandleReadv (Ptr<Socket> socket);
  uint32_t m_audioSeg=0;
  uint32_t m_videoSeg=0;
  bool m_timestamp;
  //IcnName<std::string> m_pktName;
    //local stats
  uint64_t m_statNumPktsTx; //interest
  uint64_t m_statNumDataPktRx; //data
  uint64_t m_statNumIRPktRx; //interest response
  uint64_t m_statTotalTxSize;
  uint64_t m_statTotalDataRxSize;
  uint64_t m_statTotalIrRxSize;
  uint64_t m_statNumErrorPkts;
  Time m_statTotalTime;
  bool m_nStaticDest;
  bool m_IsliceNC;
  bool m_playvideo;
  bool m_playaudio;
  bool m_tcp;
  uint32_t m_frameDecrypt;

  uint64_t m_startHops;
  uint64_t m_totalHops;
  uint32_t m_fileNumStart; 
  std::string m_minMatchType;
  std::string m_xmlpayload;
  std::string m_matchName;
  std::string m_location, m_filename;
  std::vector<Time> m_videoTime, m_audioTime;
  struct sockaddr_in clientaddr; /* client addr */
  struct sockaddr_in clientaddr2; /* client addr */
  unsigned int sockfdv, sockfda; 
  Address myTxAddress;
};

} // namespace ns3

#endif /* UDP_ECHO_CLIENT_H */
