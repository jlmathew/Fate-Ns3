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
#include "udp-fate-client-file.h"
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
#include "ns3/fateIpv4-interface.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpFateFileClientApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpFateFileClient);

TypeId
UdpFateFileClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpFateFileClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpFateFileClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpFateFileClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&UdpFateFileClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&UdpFateFileClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpFateFileClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("NumSegments", "Number of packet segments which compose a file",
                   UintegerValue (1),
                   MakeUintegerAccessor (&UdpFateFileClient::m_segSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("NumFiles", "Number of unique files sent stored in string",
                   StringValue (""),
                   MakeStringAccessor (&UdpFateFileClient::m_maxFiles),
                   MakeStringChecker ())
    .AddAttribute ("AddTimeStamp", "Add a timestamp to each packet",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateFileClient::m_timestamp),
                   MakeBooleanChecker())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&UdpFateFileClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

UdpFateFileClient::UdpFateFileClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
  m_segCnt = 0;
  m_fileCnt = 0;
   m_statNumPktsTx=0; //interest
   m_statNumDataPktRx=0; //data
   m_statNumIRPktRx=0; //interest response
   m_statTotalTxSize=0;
   m_statTotalDataRxSize=0;
   m_statTotalIrRxSize=0;
   m_statNumErrorPkts=0;
    m_statTotalTime=Seconds(0);
}

UdpFateFileClient::~UdpFateFileClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void 
UdpFateFileClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
UdpFateFileClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
UdpFateFileClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
UdpFateFileClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
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
  ParseFileSegments(); 
  m_socket->SetRecvCallback (MakeCallback (&UdpFateFileClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  ScheduleTransmit (Seconds (0.));
}
void
UdpFateFileClient::ParseFileSegments()
{
}

void 
UdpFateFileClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}


uint32_t 
UdpFateFileClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
UdpFateFileClient::SetPktPayload (const std::string &xml)
{
  NS_LOG_FUNCTION (this << xml);

  std::stringstream ss;
  ss << xml;
  ss >> m_payload;
   std::vector<uint8_t> fateData;
   m_payload.Serialize(fateData);
   m_size = fateData.size();
 
}

void 
UdpFateFileClient::SetPktPayload (const PktType &payload)
{
  m_payload = payload;
     std::vector<uint8_t> fateData;
   m_payload.Serialize(fateData);
   m_size = fateData.size();


}

void 
UdpFateFileClient::SetPktPayload (const IcnName<std::string> &name)
{
  m_payload.ClearData();
  m_payload.SetName(name);
}

void 
UdpFateFileClient::SetPktPayload (uint8_t *fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
 
  std::vector<uint8_t> data;
  for(unsigned int i=0; i<dataSize; i++) {
     data[i] = fill[i];
  }
  m_payload.Serialize(data);
  m_size = dataSize;
}
void 
UdpFateFileClient::SetTimestamp(bool timestamp) {
  m_timestamp = timestamp;
}

void 
UdpFateFileClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &UdpFateFileClient::Send, this);
}

void 
UdpFateFileClient::Send (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;

  //add segment
  m_segCnt++;
  m_segCnt %=  m_segSize;

  if (0 == m_segCnt) {
    ++m_fileCnt;
     m_fileCnt %= 1; // m_maxFiles;   
  }

  //add identifying file id and segment id, in name component
  IcnName<std::string> m_pktName = m_payload.GetName();
  m_pktName.SetUniqAttribute("segment", m_segCnt);
  m_pktName.SetUniqAttribute("fileNum", m_fileCnt);
  if(m_timestamp) {
     m_payload.SetObjectCpyNamedAttribute("Timestamp", ns3::Simulator::Now());
  }
  m_payload.SetName(m_pktName);
  
  /*PktType fatePkt;
   fatePkt.SetUnsignedNamedAttribute("TtlHop", 64);
   fatePkt.SetPacketPurpose(PktType::INTERESTPKT);
   IcnName<std::string> pktName;
   pktName.SetFullName("/test");
   //static uint64_t i=0;

   pktName.SetUniqAttribute("segment",0);
   //pktName.SetUniqAttribute("segment",(i++)%9);
   fatePkt.SetName(pktName); */
   std::vector<uint8_t> fateData;
   m_payload.ClearTempData();
   m_payload.Serialize(fateData);
   uint8_t *data = new uint8_t[fateData.size()];
   for(unsigned i = 0; i < fateData.size(); i++) {
      data[i] = fateData[i];
   }
   p = Create<Packet>(data, fateData.size());
   m_size = fateData.size();
   delete data; 

//
  m_txTrace (p);
  m_socket->Send (p);

  ++m_sent;
  ++m_statNumPktsTx;
  m_statTotalTxSize+=m_size;

  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
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

  if (m_sent < m_count) 
    {
      ScheduleTransmit (m_interval);
    }
}

void
UdpFateFileClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      PktType fatePkt;
      bool valid = FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
      NS_ASSERT(valid);
      if (fatePkt.GetPacketPurpose() == PktType::DATAPKT) {
        m_statNumDataPktRx++;
        m_statTotalDataRxSize += fatePkt.size();
          
      } else {
        m_statNumIRPktRx++;
        m_statTotalIrRxSize += fatePkt.size();

      }
      Time timesent;
      if (m_timestamp) {
              m_payload.GetObjectCpyNamedAttribute("Timestamp", timesent);
      }
      m_statTotalTime += (Simulator::Now()-timesent);
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now () << "/" << timesent << " 's client received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort () << " --\n " << fatePkt);
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
    }
}
void
UdpFateFileClient::PrintStats(std::ostream &os) const
{
   os << "m_statNumPktsTx:" << m_statNumPktsTx << "\n";
   os << "m_statNumDataPktRx:" << m_statNumDataPktRx << "\n";
   os << "m_statNumIRPktRx:" << m_statNumIRPktRx << "\n";
   os << "m_statTotalTxSize:" << m_statTotalTxSize << "\n";
   os << "m_statTotalDataRxSize:" << m_statTotalDataRxSize << "\n";
   os << "m_statTotalIrRxSize:" << m_statTotalIrRxSize << "\n";
   os << "m_statNumErrorPkts:" << m_statNumErrorPkts << "\n";
   os << "m_statTotalTime:" << m_statTotalTime << "\n";
}

} // Namespace ns3
