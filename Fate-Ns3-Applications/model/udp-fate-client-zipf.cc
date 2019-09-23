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
#include "udp-fate-client-zipf.h"

#include "ns3/IcnName.h"
#include <iostream>
#include <ostream>
#include <string>
#include <functional>
#include "ns3/PacketTypeBase.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/fateIpv4-interface.h"
#include "ns3/string.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpFateZipfClientApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpFateZipfClient);

TypeId
UdpFateZipfClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpFateZipfClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpFateZipfClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpFateZipfClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&UdpFateZipfClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&UdpFateZipfClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpFateZipfClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("NumSegments", "Number of packet segments which compose a file",
                   UintegerValue (1),
                   MakeUintegerAccessor (&UdpFateZipfClient::m_segSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("BaseFileNumStart", "Start file request at this number",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpFateZipfClient::m_fileNumStart),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("NumFiles", "Number of unique files sent",
                   UintegerValue (10),
                   MakeUintegerAccessor (&UdpFateZipfClient::m_maxFiles),
                   MakeUintegerChecker<uint32_t> ())
     .AddAttribute ("AddTimeStamp", "Add a timestamp to each packet",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateZipfClient::m_timestamp),
                   MakeBooleanChecker())
     .AddAttribute ("ZipfAlpha", "Set the alpha value for zipf",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&UdpFateZipfClient::m_alpha),
                   MakeDoubleChecker<double>())
     .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&UdpFateZipfClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
     .AddAttribute ("NStaticDestination", "Use a producer created destination table to route packets",
                    BooleanValue (false),
                    MakeBooleanAccessor (&UdpFateZipfClient::m_nStaticDest),
                    MakeBooleanChecker())
     .AddAttribute ("matchString", "Match string used to match to server",
                   StringValue("/test1/fileNum="),
                   MakeStringAccessor(&UdpFateZipfClient::m_matchName), 
                   MakeStringChecker())
     .AddAttribute ("matchByType", "Match name by 'location | filenum | segnum'",
                   StringValue("filenum"),
                   MakeStringAccessor(&UdpFateZipfClient::m_minMatchType), 
                   MakeStringChecker())
     .AddAttribute ("PktPayload", "Ascii XML representation of a packet, for ipv4 payload",
                   StringValue(""),
                   MakeStringAccessor(&UdpFateZipfClient::m_xmlpayload), 
                   MakeStringChecker())
     .AddAttribute ("sendToOffPathCache", "Send packet to an intermediate destination",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateZipfClient::m_offPathCache),
                   MakeBooleanChecker())
  ;
  return tid;
}

UdpFateZipfClient::UdpFateZipfClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
  m_segCnt = 0;
  m_fileCnt = 0;
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
}

UdpFateZipfClient::~UdpFateZipfClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void 
UdpFateZipfClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
UdpFateZipfClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}
void 
UdpFateZipfClient::SetTimestamp(bool timestamp) {
  m_timestamp = timestamp;
}

void
UdpFateZipfClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
UdpFateZipfClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
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
            sock->SetRecvCallback (MakeCallback (&UdpFateZipfClient::HandleRead, this));
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
             sock->SetRecvCallback (MakeCallback (&UdpFateZipfClient::HandleRead, this));
            sock->SetAllowBroadcast (true);
            m_vectSocket.push_back(sock);

       } else if (m_minMatchType=="segnum") {
         assert(0);//not done yet, need a filenum AND segnum loop
     
         
       } else { assert(0);}
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

    m_socket->SetRecvCallback (MakeCallback (&UdpFateZipfClient::HandleRead, this));
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
UdpFateZipfClient::StopApplication ()
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
UdpFateZipfClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
UdpFateZipfClient::SetPktPayload (const std::string &xml)
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
UdpFateZipfClient::SetPktPayload (const PktType &payload)
{
  m_payload = payload;
     std::vector<uint8_t> fateData;
   m_payload.Serialize(fateData);
   m_size = fateData.size();
   m_payload.GetUnsignedNamedAttribute("TtlHop", m_startHops);


}

void 
UdpFateZipfClient::SetPktPayload (const IcnName<std::string> &name)
{
  m_payload.ClearData();
  m_payload.SetName(name);
}

void 
UdpFateZipfClient::SetPktPayload (uint8_t *fill, uint32_t dataSize)
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
UdpFateZipfClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &UdpFateZipfClient::Send, this);
}

void 
UdpFateZipfClient::Send (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;

  //add segment
  m_zipf->SetAttribute ("N", IntegerValue (m_maxFiles));
  m_zipf->SetAttribute ("Alpha", DoubleValue (m_alpha));
  if (0 == m_segCnt) {
     m_fileCnt = m_zipf->GetInteger();
  }

//uint32_t ivalue = m_zipf->GetInteger();

  m_segCnt++;
  m_segCnt %=  m_segSize;


  //add identifying file id and segment id, in name component
  IcnName<std::string> m_pktName = m_payload.GetName();
  m_pktName.SetUniqAttribute("segment", m_segCnt);
  NS_LOG_INFO(" (" << m_fileCnt << "," << m_segCnt << ") - ");
  m_pktName.SetUniqAttribute("fileNum", m_fileCnt+m_fileNumStart);
 
  std::string matchName=m_matchName; //"/test1/fileNum=";
  std::stringstream out;
  out << (m_fileCnt+m_fileNumStart);
  matchName.append(out.str());
//std::cout << matchName << " = " << "filenum=" << m_fileCnt+m_fileNumStart << "\n"; 
  if(m_timestamp) {
     m_payload.SetObjectCpyNamedAttribute("Timestamp", ns3::Simulator::Now());
  }
  m_payload.SetName(m_pktName);
  m_payload.SetPacketPurpose(PktType::INTERESTPKT);
  if (m_offPathCache)
  {
	  uint32_t hashValue = std::hash<std::string>{}(m_matchName);
	  std::stringstream dc;
	  dc << GetIpFromRange(hashValue);
	  dc << ";";
            ipPort_t info = GetProdNodeIpv4(m_matchName);
            Ipv4Address ipv4(info.first);
	    dc<<ipv4<<";";
          m_payload.SetPrintedNamedAttribute("DstChain", dc.str());
          m_payload.SetPrintedNamedAttribute("ReturnChain", "");
std::cout << "name:" << m_matchName << " hashes to hashValue(" << hashValue << ")  with a returnchain of " << dc.str() << "\n";
  }

 std::cout << "tx:" << m_payload << "\n"; 
   std::vector<uint8_t> fateData;
   m_payload.ClearTempData();
   m_payload.Serialize(fateData);
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
  NS_LOG_INFO("PKT TX:" << m_payload);
  if ((0 == m_count) || (m_sent < m_count)) 
    {
      ScheduleTransmit (m_interval);
    }
}


//should print out wrongly routed packets FIXME JLM
void
UdpFateZipfClient::HandleRead (Ptr<Socket> socket)
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
      Time timesent;
      uint64_t ttlrx=0;
      fatePkt.GetUnsignedNamedAttribute("TtlHop", ttlrx);
       m_totalHops += (m_startHops-ttlrx);

      if (m_timestamp) {
              m_payload.GetObjectCpyNamedAttribute("Timestamp", timesent);
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
        std::cout << "RX PKT:" << fatePkt << "\n";
    }
}
void
UdpFateZipfClient::PrintStats(std::ostream &os) const
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
}

} // Namespace ns3
