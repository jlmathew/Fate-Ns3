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
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/string.h"
#include "ns3/IcnName.h"
#include <iostream>
#include <ostream>
#include <string>
#include "ns3/PacketTypeBase.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/fateIpv4-interface.h"
#include "ns3/string.h"
#include "udp-fate-file-zipf-client.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpFateFileZipfClientApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpFateFileZipfClient);

TypeId
UdpFateFileZipfClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpFateFileZipfClient")
                      .SetParent<Application> ()
                      .SetGroupName("Applications")
                      .AddConstructor<UdpFateFileZipfClient> ()
                      .AddAttribute ("MaxPackets",
                                     "The maximum number of packets the application will send",
                                     UintegerValue (100),
                                     MakeUintegerAccessor (&UdpFateFileZipfClient::m_count),
                                     MakeUintegerChecker<uint32_t> ())
                      .AddAttribute ("Interval",
                                     "The time to wait between packets",
                                     TimeValue (Seconds (1.0)),
                                     MakeTimeAccessor (&UdpFateFileZipfClient::m_interval),
                                     MakeTimeChecker ())
                      .AddAttribute ("RemoteAddress",
                                     "The destination Address of the outbound packets",
                                     AddressValue (),
                                     MakeAddressAccessor (&UdpFateFileZipfClient::m_peerAddress),
                                     MakeAddressChecker ())
                      .AddAttribute ("RemotePort",
                                     "The destination port of the outbound packets",
                                     UintegerValue (0),
                                     MakeUintegerAccessor (&UdpFateFileZipfClient::m_peerPort),
                                     MakeUintegerChecker<uint16_t> ())
                      .AddAttribute ("BaseFileNumStart", "Start file request at this number",
                                     UintegerValue (0),
                                     MakeUintegerAccessor (&UdpFateFileZipfClient::m_fileNumStart),
                                     MakeUintegerChecker<uint32_t> ())
                      .AddAttribute ("NumFiles", "Number of unique files sent",
                                     UintegerValue (10),
                                     MakeUintegerAccessor (&UdpFateFileZipfClient::m_maxFiles),
                                     MakeUintegerChecker<uint32_t> ())
                      .AddAttribute ("AddTimeStamp", "Add a timestamp to each packet",
                                     BooleanValue (false),
                                     MakeBooleanAccessor (&UdpFateFileZipfClient::m_timestamp),
                                     MakeBooleanChecker())
                      .AddAttribute ("ZipfAlpha", "Set the alpha value for zipf",
                                     DoubleValue (1.0),
                                     MakeDoubleAccessor (&UdpFateFileZipfClient::m_alpha),
                                     MakeDoubleChecker<double>())
                      .AddTraceSource ("Tx", "A new packet is created and is sent",
                                       MakeTraceSourceAccessor (&UdpFateFileZipfClient::m_txTrace),
                                       "ns3::Packet::TracedCallback")
                      .AddAttribute ("NStaticDestination", "Use a producer created destination table to route packets",
                                     BooleanValue (false),
                                     MakeBooleanAccessor (&UdpFateFileZipfClient::m_nStaticDest),
                                     MakeBooleanChecker())
                      .AddAttribute ("matchString", "Match string used to match to server",
                                     StringValue("/test0/fileNum="),
                                     MakeStringAccessor(&UdpFateFileZipfClient::m_matchName),
                                     MakeStringChecker())
                      .AddAttribute ("matchByType", "Match name by 'location | filenum | segnum'",
                                     StringValue("filenum"),
                                     MakeStringAccessor(&UdpFateFileZipfClient::m_minMatchType),
                                     MakeStringChecker())
                      .AddAttribute ("PktPayload", "Ascii XML representation of a packet, for ipv4 payload",
                                     StringValue(""),
                                     MakeStringAccessor(&UdpFateFileZipfClient::m_xmlpayload),
                                     MakeStringChecker())
                      .AddAttribute ("uniqueDataPktNames", "Data packets have unique names",
                                     BooleanValue (false),
                                     MakeBooleanAccessor (&UdpFateFileZipfClient::m_uniqDataNames),
                                     MakeBooleanChecker())

                      ;
  return tid;
}

UdpFateFileZipfClient::UdpFateFileZipfClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
  m_segCnt = 0;
  m_fileCnt = 1;
  m_zipf = CreateObject<ZipfRandomVariable> ();
  m_statNumPktsTx=0; //interest
  m_statNumDataPktRx=0; //data
  m_statNumIRPktRx=0; //interest response
  m_statTotalTxSize=0;
  m_statTotalDataRxSize=0;
  m_statTotalIrRxSize=0;
  m_statNumErrorPkts=0;
  m_statTotalTime=Seconds(0);
  m_startHops=0;
  m_totalHops=0;
  m_fileNumStart=0;
  m_statNumPktHdrTx=0;
  m_statNumPktHdrRx=0;
  m_statNumBytesTx=0;
  m_statNumBytesRx=0;
}

UdpFateFileZipfClient::~UdpFateFileZipfClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void
UdpFateFileZipfClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
UdpFateFileZipfClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}
void
UdpFateFileZipfClient::SetTimestamp(bool timestamp) {
  m_timestamp = timestamp;
}

void
UdpFateFileZipfClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
UdpFateFileZipfClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  count = 0;
#ifdef ICN_STAT_ROUTES
  if (!m_nStaticDest)
  {
    NS_LOG_INFO( "cant use #define stat routes AND NOT set NStaticDestination\n");
    assert(0);
  }
  if (m_vectSocket.size() == 0) {
    std::string matchName;
    if (m_minMatchType=="filenum") {
      for(unsigned int i=0; i<m_maxFiles; i++) {
        matchName =m_matchName;
        std::stringstream out;
        out << i+1+m_fileNumStart;
        matchName.append(out.str());
        Ptr<Socket> sock = Socket::CreateSocket (GetNode (), tid);
        sock->Bind();
//FIXME TODO JLM
//need to add configurable options to match only by path, or by which options
        ipPort_t info = GetProdNodeIpv4(matchName);
        Ipv4Address ipv4(info.first);
        uint16_t port= info.second;
//std::cout << matchName << " goes to " << info.first << ":" << port << "\n";
        sock->Connect(InetSocketAddress (ipv4, port));
        sock->SetRecvCallback (MakeCallback (&UdpFateFileZipfClient::HandleRead, this));
        sock->SetAllowBroadcast (true);
        m_vectSocket.push_back(sock);
      }
    } else if (m_minMatchType=="location") {
      matchName=m_matchName;
      Ptr<Socket> sock = Socket::CreateSocket (GetNode (), tid);
      sock->Bind();
      ipPort_t info = GetProdNodeIpv4(matchName);
      Ipv4Address ipv4(info.first);
      uint16_t port= info.second;
      sock->Connect(InetSocketAddress (ipv4, port));
      sock->SetRecvCallback (MakeCallback (&UdpFateFileZipfClient::HandleRead, this));
      sock->SetAllowBroadcast (true);
      m_vectSocket.push_back(sock);

    } else {
      assert(0);
    }
  }
#else
  if (m_socket == 0)
  {
    m_socket = Socket::CreateSocket (GetNode (), tid);
    if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
    {
      m_socket->Bind();
      m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
    }
    else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
    {
      m_socket->Bind6();
      m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
    }
    else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
    {
      m_socket->Bind ();
      m_socket->Connect (m_peerAddress);
    }
    else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
    {
      m_socket->Bind6 ();
      m_socket->Connect (m_peerAddress);
    }
    else
    {
      NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
    }
  }

  m_socket->SetRecvCallback (MakeCallback (&UdpFateFileZipfClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
}
#endif
  //Make it random between intervals FIXME TODO
  if (m_xmlpayload.size()) {
    std::stringstream(m_xmlpayload) >> m_payload;
  }
  m_payload.GetUnsignedNamedAttribute("TtlHop", m_startHops);
  ScheduleTransmit (Seconds (0.));
}

void
UdpFateFileZipfClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  PrintStats(std::cout);
  if (m_socket != 0)
  {
    m_socket->Close ();
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    m_socket = 0;
  }

  Simulator::Cancel (m_sendEvent);
}


uint32_t
UdpFateFileZipfClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void
UdpFateFileZipfClient::SetPktPayload (const std::string &xml)
{
  NS_LOG_FUNCTION (this << xml);

  std::stringstream ss;
  ss << xml;
  ss >> m_payload;
  std::vector<uint8_t> fateData;
  m_payload.Serialize(fateData);
  m_size = fateData.size();
  m_payload.GetUnsignedNamedAttribute("TtlHop", m_startHops);

}

void
UdpFateFileZipfClient::SetPktPayload (const PktType &payload)
{
  m_payload = payload;
  std::vector<uint8_t> fateData;
  m_payload.Serialize(fateData);
  m_size = fateData.size();
  m_payload.GetUnsignedNamedAttribute("TtlHop", m_startHops);


}

void
UdpFateFileZipfClient::SetPktPayload (const IcnName<std::string> &name)
{
  m_payload.ClearData();
  m_payload.SetName(name);
}

void
UdpFateFileZipfClient::SetPktPayload (uint8_t *fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);

  std::vector<uint8_t> data;
  for(unsigned int i=0; i<dataSize; i++) {
    data[i] = fill[i];
  }
  m_payload.Serialize(data);
  m_size = dataSize;
  m_payload.GetUnsignedNamedAttribute("TtlHop", m_startHops);

}

void
UdpFateFileZipfClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &UdpFateFileZipfClient::Send, this);
}

void
UdpFateFileZipfClient::SendBody ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;

  PktType payload = m_payload;
  //add identifying file id and segment id, in name component
  IcnName<std::string> m_pktName = payload.GetName();
  NS_LOG_INFO(" (" << m_fileCnt << "," << m_segCnt << ") - ");
  m_pktName.SetUniqAttribute("fileNum", m_fileCnt+m_fileNumStart);
  if (m_uniqDataNames) {
    m_pktName.SetUniqAttribute("segment", m_segCnt);
  } else {
    payload.SetUnsignedNamedAttribute("Segment", m_segCnt);
    payload.SetUnsignedNamedAttribute("ByteStart", (m_segCnt-1)*m_segSize);
    payload.SetUnsignedNamedAttribute("ByteEnd", m_segCnt*m_segSize-1);
  }
  std::string matchName=m_matchName; //"/test1/fileNum=";
  std::stringstream out;
  out << (m_fileCnt+m_fileNumStart);
  matchName.append(out.str());
//std::cout << matchName << " = " << "filenum=" << m_fileCnt+m_fileNumStart << "\n";
  if(m_timestamp) {
    payload.SetObjectCpyNamedAttribute("Timestamp", ns3::Simulator::Now());
  }

  payload.SetUnsignedNamedAttribute("PktId", count++);

  payload.SetName(m_pktName);
  payload.SetPacketPurpose(PktType::INTERESTPKT);
  std::cout << "Summary:" << m_pktName << " Request body segment:" << m_segCnt << "\n" << payload << "\n";
  std::vector<uint8_t> fateData;
  payload.ClearTempData();
  payload.Serialize(fateData);
  uint8_t *data = new uint8_t[fateData.size()];
  for(unsigned i = 0; i < fateData.size(); i++) {
    data[i] = fateData[i];
  }
  p = Create<Packet>(data, fateData.size());
  delete[] data;

  m_txTrace (p);
  ++m_sent;
  ++m_statNumPktsTx;
  m_size = p->GetSize();
  m_statTotalTxSize+=m_size;
#ifdef ICN_STAT_ROUTES
  if (m_minMatchType=="location") {
    m_vectSocket[0]->Send(p);
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to server." );
  } else {
    m_vectSocket[m_fileCnt-1]->Send(p);
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to server:" << m_fileCnt );
  }

#else
  m_socket->Send (p);

  if (Ipv4Address::IsMatchingType (m_peerAddress))
  {
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                 Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort );
  }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
  {
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                 Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
  }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
  {
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                 InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
  }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
  {
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                 Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
  }
#endif
  NS_LOG_INFO("PKT TX:" << payload);
  if ((0 == m_count) || (m_sent < m_count))
  {
    ScheduleTransmit (m_interval);
  }
}



void
UdpFateFileZipfClient::Send (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  //still correct, but requests content from high to low

  if (m_segCnt) {
    SendBody();
    --m_segCnt;
    return;
  }
  Ptr<Packet> p;

  //make copy of 'base' packet
  PktType payload = m_payload;
  //add segment
  m_zipf->SetAttribute ("N", IntegerValue (m_maxFiles));
  m_zipf->SetAttribute ("Alpha", DoubleValue (m_alpha));
  m_fileCnt = m_zipf->GetInteger();

  //add identifying file id and segment id, in name component
  IcnName<std::string> m_pktName = payload.GetName();
  NS_LOG_INFO(" (" << m_fileCnt << "," << m_segCnt << ") - ");
  m_pktName.SetUniqAttribute("fileNum", m_fileCnt+m_fileNumStart);

  std::cout << "Summary: Requesting new file " << m_matchName << "/fileNum" << m_fileCnt+m_fileNumStart << "\n";
  std::string matchName=m_matchName; //"/test1/fileNum=";
  std::stringstream out;
  out << (m_fileCnt+m_fileNumStart);
  matchName.append(out.str());
  std::cout << matchName << " = " << "filenum=" << m_fileCnt+m_fileNumStart << "\n";
  if(m_timestamp) {
    payload.SetObjectCpyNamedAttribute("Timestamp", ns3::Simulator::Now());
  }
  payload.SetUnsignedNamedAttribute("Header", 1);

  payload.SetUnsignedNamedAttribute("PktId", count++);
  ++m_statNumPktHdrTx;

  payload.SetName(m_pktName);
  payload.SetPacketPurpose(PktType::INTERESTPKT);
  std::cout << "client tx Header:" << payload << "\n";
  std::vector<uint8_t> fateData;
  payload.ClearTempData();
  payload.Serialize(fateData);
  uint8_t *data = new uint8_t[fateData.size()];
  for(unsigned i = 0; i < fateData.size(); i++) {
    data[i] = fateData[i];
  }
  p = Create<Packet>(data, fateData.size());
  delete[] data;

  m_txTrace (p);
  ++m_sent;
  ++m_statNumPktsTx;
  m_size = p->GetSize();
  m_statTotalTxSize+=m_size;
#ifdef ICN_STAT_ROUTES
  if (m_minMatchType=="location") {
    m_vectSocket[0]->Send(p);
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to server." );
  } else {
    m_vectSocket[m_fileCnt-1]->Send(p);
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to server:" << m_fileCnt );
  }

#else
  m_socket->Send (p);

  if (Ipv4Address::IsMatchingType (m_peerAddress))
  {
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                 Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort );
  }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
  {
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                 Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
  }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
  {
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                 InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
  }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
  {
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                 Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
  }
#endif
  NS_LOG_INFO("PKT TX:" << payload);
  if ((0 == m_count) || (m_sent < m_count))
  {
    ScheduleTransmit (m_interval);
  }
}


//should print out wrongly routed packets FIXME JLM
void
UdpFateFileZipfClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
  {
    PktType fatePkt;
    FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
    //bool valid = FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
    //NS_ASSERT(valid);
    //is it a header?
    uint64_t tmp;
    bool isHdr = fatePkt.GetUnsignedNamedAttribute("Header", tmp);
    if (isHdr)
    {
      fatePkt.GetUnsignedNamedAttribute("Segments", m_segCnt);
      fatePkt.GetUnsignedNamedAttribute("SegSize", m_segSize );
      std::cout << "SUMMARY: Received Header\n";
      m_statNumPktHdrRx++;
      m_statNumBytesRx+=m_segSize;
    } else {
      uint64_t temp=0;
      bool found = fatePkt.GetUnsignedNamedAttribute("Segment", temp );
      if (found) {
        std::cout << "SUMMARY: Received Body of segment " << temp << "\n";
        m_statNumBytesRx+=m_segSize;
      }
      else {
        std::cout << "SUMMARY: Rx ICN packet\n";
      }

    }

    Time timesent;
    uint64_t ttlrx=0;
    fatePkt.GetUnsignedNamedAttribute("TtlHop", ttlrx);
    m_totalHops += (m_startHops-ttlrx);

    if (m_timestamp) {
      fatePkt.GetObjectCpyNamedAttribute("Timestamp", timesent);
    }
    if (fatePkt.GetPacketPurpose() == PktType::DATAPKT) {
      m_statNumDataPktRx++;
      m_statTotalDataRxSize += packet->GetSize();

    } else {
      m_statNumIRPktRx++;
      m_statTotalIrRxSize += packet->GetSize();

    }
    m_statTotalTime += (Simulator::Now()-timesent);

    if (InetSocketAddress::IsMatchingType (from))
    {
      NS_LOG_INFO ("At time delta " << (Simulator::Now ()- timesent).GetSeconds() << " 's client received " << packet->GetSize () << " bytes from " <<
                   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                   InetSocketAddress::ConvertFrom (from).GetPort () << " for " << (Simulator::Now()-timesent) << "seconds and " << m_startHops-ttlrx << "/" << m_totalHops << " --\n " ); //<< fatePkt);
    }
    else if (Inet6SocketAddress::IsMatchingType (from))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                   Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                   Inet6SocketAddress::ConvertFrom (from).GetPort ());
    }
    NS_LOG_INFO("RX PKT:" << fatePkt);
    std::cout << "client RX PKT:" << fatePkt << "\n";
  }
}
void
UdpFateFileZipfClient::PrintStats(std::ostream &os) const
{
  os << "m_fileNumStart:" << m_fileNumStart << "\n";
  os << "m_statNumPktsTx:" << m_statNumPktsTx << "\n";
  os << "m_statNumDataPktRx:" << m_statNumDataPktRx << "\n";
  os << "m_statNumIRPktRx:" << m_statNumIRPktRx << "\n";
  os << "m_statTotalTxSize:" << m_statTotalTxSize << "\n";
  os << "m_statTotalDataRxSize:" << m_statTotalDataRxSize << "\n";
  os << "m_statTotalIrRxSize:" << m_statTotalIrRxSize << "\n";
  os << "m_statNumErrorPkts:" << m_statNumErrorPkts << "\n";
  os << "m_statTotalTime:" << m_statTotalTime << "\n";
  os << "m_totalHops:" << m_totalHops << "\n";
  os << "m_statNumPktHdrTx:" << m_statNumPktHdrTx << "\n";
  os << "m_statNumPktHdrRx:" << m_statNumPktHdrRx << "\n";
  os << "m_statNumBytesRx:" << m_statNumPktHdrRx << "\n";
  os << "m_statNumBytesTx:" << m_statNumPktHdrRx << "\n";
}

int UdpFateFileZipfClient::count=0;
} // Namespace ns3
