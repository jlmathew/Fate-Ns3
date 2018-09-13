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
#ifndef UDP_FATE_VIDEO_CLIENT_H
#define UDP_FATE_VIDEO_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/IcnName.h"
#include "ns3/PacketTypeBase.h"
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <list>
#include "ns3/random-variable-stream.h"

namespace ns3 {

class Socket;
class Packet;
struct videoReqStruct_t {
   std::string name;
   Ipv4Address addr;
   uint16_t cport;
   uint16_t vport;
   uint16_t aport;
};
/**
 * \ingroup udpecho
 * \brief A Udp Echo client
 *
 * Every packet sent should be returned by the server and received here.
 */
class UdpFateVideoClient : public Application 
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  UdpFateVideoClient ();

  virtual ~UdpFateVideoClient ();

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
void SetTimestamp ( bool timestamp);

void ReqManifest(Time dt, const std::string &name);
void parseTiming(const std::string &name, bool isVideo=true);

std::string getVideoSdf();
void getAudioFile();
void getVideoFile();


  uint32_t
  GetDataSize() const;
  void
  PrintStats(std::ostream &os) const;
protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
void decrypt(PktType &pkt, Address from, uint8_t *decryptData, uint16_t &length); 
virtual void playPkt(const PktType &pkt);
virtual void initPlayPkt();

  /**
   * \brief Schedule the next packet transmission
   * \param dt time interval between packets.
   */
  void ScheduleTransmit (Time dt);
  /**
   * \brief Send a packet
   */
  void Sendc (Ptr<Packet> p);
  void Sendv (Ptr<Packet> p);
  void Senda (Ptr<Packet> p);
  void SendManifest (const std::string &name);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleReadc (Ptr<Socket> socket);
  void HandleReadv (Ptr<Socket> socket);
  void HandleReada (Ptr<Socket> socket);

  uint32_t m_count; //!< Maximum number of packets the application will send
  Time m_interval; //!< Packet inter-send time
  uint32_t m_size; //!< Size of the sent packet
  uint32_t m_dataSize; //!< packet payload size (must be equal to m_size)

  uint32_t m_sent; //!< Counter for sent packets
  Ptr<Socket> m_socket; //!< Socket
  //Ptr<Socket> m_socketv; //!< Socket
  //Ptr<Socket> m_socketa; //!< Socket
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPortc, m_peerPort; //v, m_peerPorta;
  EventId m_sendEvent; //!< Event to send the next packet

  /// Callbacks for tracing the packet Tx events
  TracedCallback<Ptr<const Packet> > m_txTrace;

  PktType m_payload; //!< Fate packet payload

  uint32_t m_segCnt;
  uint32_t m_audioSeg, m_videoSeg;
  uint32_t m_segSize;
  uint32_t m_fileCnt;
  uint32_t m_maxVideos;
  bool m_timestamp;
  bool m_IsliceNC;
  bool m_playvideo;
  bool m_playaudio;
  bool m_tcp;
  uint32_t m_frameDecrypt;
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
  std::string m_filename, m_minMatchType, m_matchName, m_xmlpayload;
  std::string m_timerfilename;
  std::string m_currentVidName;
  std::vector<Time> m_videoTime, m_audioTime;
};





} // namespace ns3

#endif /* UDP_ECHO_CLIENT_H */
