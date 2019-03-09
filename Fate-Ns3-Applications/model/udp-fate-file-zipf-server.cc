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
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/PacketTypeBase.h"
#include "ns3/fateIpv4-interface.h"
#include "udp-fate-file-zipf-server.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpFateFileZipfServerApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpFateFileZipfServer);

TypeId
UdpFateFileZipfServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpFateFileZipfServer")
                      .SetParent<Application> ()
                      .SetGroupName("Applications")
                      .AddConstructor<UdpFateFileZipfServer> ()
                      .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                                     UintegerValue (100),
                                     MakeUintegerAccessor (&UdpFateFileZipfServer::m_port),
                                     MakeUintegerChecker<uint16_t> ())
                      .AddAttribute ("ReturnSize", "Size of the data attribute in the packet, using <DATA> attribute tag.",
                                     UintegerValue (1),
                                     MakeUintegerAccessor (&UdpFateFileZipfServer::m_size),
                                     MakeUintegerChecker<uint16_t> ())
                      .AddAttribute ("MinMatchName",
                                     "Exact Packet Name with minimal matching attributes",
                                     StringValue (""),
                                     MakeStringAccessor (&UdpFateFileZipfServer::m_setNameMatch),
                                     MakeStringChecker ())
                      .AddAttribute ("FileSizes",
                                     "FileSize file name",
                                     StringValue (""),
                                     MakeStringAccessor (&UdpFateFileZipfServer::m_segSizeFile),
                                     MakeStringChecker ()) ;

  return tid;
}
void
UdpFateFileZipfServer::GetMatchNameAndPort(std::string &matchName, uint16_t &port)
{
  matchName = m_setNameMatch;
  port = m_port;
}
UdpFateFileZipfServer::UdpFateFileZipfServer ()
{
  NS_LOG_FUNCTION (this);
  m_statNumPktsTx =0;
  m_statNumIntPktRx=0;
  m_statNumDataPktTx=0;
  m_statNumNotMatchPktTx=0;
  m_statTotalDataTxSize=0;
  m_statTotalNotMatchTxSize=0;
  m_statNumErrorPkts=0;
  m_statTotalIntRxSize=0;
  m_segSizeBytes=0;
}

UdpFateFileZipfServer::~UdpFateFileZipfServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
  //m_partMatch.clear();
}

void
UdpFateFileZipfServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}


void
UdpFateFileZipfServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  if (m_segSizeFile.empty()) {
    m_segSize.push_back(1);
  } else {
    std::stringstream ss(m_segSizeFile);
    uint32_t val=0;
    ss >> m_segSizeBytes;
    while (ss >> val) {
      m_segSize.push_back(val);
    }
  }

  if (m_socket == 0)
  {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
    m_socket->Bind (local);
    if (addressUtils::IsMulticast (m_local))
    {
      Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
      if (udpSocket)
      {
        // equivalent to setsockopt (MCAST_JOIN_GROUP)
        udpSocket->MulticastJoinGroup (0, m_local);
      }
      else
      {
        NS_FATAL_ERROR ("Error: Failed to join multicast group");
      }
    }
  }

  if (m_socket6 == 0)
  {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket6 = Socket::CreateSocket (GetNode (), tid);
    Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
    m_socket6->Bind (local6);
    if (addressUtils::IsMulticast (local6))
    {
      Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
      if (udpSocket)
      {
        // equivalent to setsockopt (MCAST_JOIN_GROUP)
        udpSocket->MulticastJoinGroup (0, local6);
      }
      else
      {
        NS_FATAL_ERROR ("Error: Failed to join multicast group");
      }
    }
  }
  m_socket->SetRecvCallback (MakeCallback (&UdpFateFileZipfServer::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&UdpFateFileZipfServer::HandleRead, this));
}

void
UdpFateFileZipfServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
  {
    m_socket->Close ();
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  }
  if (m_socket6 != 0)
  {
    m_socket6->Close ();
    m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  }
}
void
UdpFateFileZipfServer::SetPartialMatch(const std::list< std::pair< std::string, std::string > > &match)
{
  //m_partMatch = match;
}

void
UdpFateFileZipfServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
  {
    if (InetSocketAddress::IsMatchingType (from))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                   InetSocketAddress::ConvertFrom (from).GetPort ());
    }
    else if (Inet6SocketAddress::IsMatchingType (from))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                   Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                   Inet6SocketAddress::ConvertFrom (from).GetPort ());
    }

    PktType fatePkt;
    FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
    //bool valid = FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
    std::cout << "server rx:" << fatePkt << "\n";

    fatePkt.ClearTempData();
    packet->RemoveAllPacketTags ();
    packet->RemoveAllByteTags ();

    //Check if correct matching name
    bool match = true;
    uint64_t fileNum=0;
    if (!m_setNameMatch.empty()) {
      IcnName < std::string> matchName, pktName;
      matchName.SetFullName(m_setNameMatch);
      pktName = fatePkt.GetName();
      match = pktName.PartialAttributeMatch(matchName);
      match |= pktName.GetUniqAttribute("FileNum", fileNum);
    }  //FIXME TODO above, set file num

    //if true, it is a data pkt
    //rx itnerest stats
    if (fatePkt.GetPacketPurpose() == PktType::INTERESTPKT) {
      m_statNumIntPktRx++;
      m_statTotalIntRxSize += packet->GetSize();
    }

    //reply and stats
    if (match && (fatePkt.GetPacketPurpose() == PktType::INTERESTPKT)) {
      fatePkt.SetPacketPurpose(PktType::DATAPKT);
      //single or multiple packets?
      uint64_t exist = 0;
      bool header = fatePkt.GetUnsignedNamedAttribute("Header", exist,false);
      uint32_t maxSegment = 1 ;
      if ( m_segSize.size() > fileNum) {
        maxSegment = m_segSize[fileNum];
      } else {
        maxSegment = m_segSize[m_segSize.size()-1];
      }

      if (header) {  //handle both segments and byte range requests
        fatePkt.SetUnsignedNamedAttribute("Segments", maxSegment);
        fatePkt.SetUnsignedNamedAttribute("TotalSize", (maxSegment)*m_segSizeBytes);
        fatePkt.SetUnsignedNamedAttribute("SegSize", m_segSizeBytes );
      } else {  //data segment
        uint64_t segment=0;
        uint64_t byteStart = 0;
        uint64_t byteEnd = 0;
        bool segExists=false;
        bool byteRngExists=false;
        segExists = fatePkt.GetUnsignedNamedAttribute("Segment", segment);
        byteRngExists = fatePkt.GetUnsignedNamedAttribute("ByteStart", byteStart);
        byteRngExists |= fatePkt.GetUnsignedNamedAttribute("ByteEnd", byteEnd);
        if (!segExists) {
          segment = ((byteEnd)/m_segSizeBytes);
        }
        if (segExists || byteRngExists) {
          if (segment > maxSegment ) {
            fatePkt.SetPacketPurpose(PktType::INTERESTRESPONSEPKT);
            fatePkt.SetNamedAttribute("ErrorCode", "Incomplete: Segment not requested");

          } else if ((byteEnd < byteStart) ) {
            fatePkt.SetPacketPurpose(PktType::INTERESTRESPONSEPKT);
            fatePkt.SetNamedAttribute("ErrorCode", "Incomplete: Byte Range out of range");

          } else { //all good
            //char value='A'+segment%26;
            /*if (byteRngExists) {
              std::string data(byteEnd-byteStart+1, 'A');
              fatePkt.SetNamedAttribute("DATA", data, false);
            } else { //segment
              std::string data(m_size, 'a');
              fatePkt.SetNamedAttribute("DATA", data, false);
            }*/
          }
        } else { //error, need segment
          fatePkt.SetPacketPurpose(PktType::INTERESTRESPONSEPKT);
          fatePkt.SetNamedAttribute("ErrorCode", "Incomplete: Segment not requested");
        }
      }
      m_statNumDataPktTx++;
      //m_statTotalDataTxSize += packet->GetSize();
    } else if (fatePkt.GetPacketPurpose() == PktType::INTERESTPKT) {
      //if false, it is a missed interestresponse packet
      //NS_LOG_INFO("BAD PKT:" << fatePkt);
      fatePkt.SetPacketPurpose(PktType::INTERESTRESPONSEPKT);
      fatePkt.SetNamedAttribute("ErrorCode", "Error: Correct server not found");
      //m_statTotalNotMatchTxSize += packet->GetSize();
      m_statNumNotMatchPktTx++;
      //assert(0);

    } else {  //do nothing in invalid packet
      std::cout << "invalid packet of type:" << fatePkt.GetPacketPurpose() << "\n";
      return;
    }

    //FIXME TODO should we only do this for a successful hit?
    //mark it as hitting the server/producer
    fatePkt.SetUnsignedNamedAttribute("ServerHitNodeName", GetNode()->GetId(), false);

    std::string retChain;
    bool chainRetResult = fatePkt.GetNamedAttribute("ReturnChain", retChain);
    std::string dstAddr;
    fatePkt.GetPrintedNamedAttribute("Ipv4Dst", dstAddr);
    if (chainRetResult)
    {
      //TODO FIXME add option to go by node name (via dns lookup) or IP
      std::size_t found = retChain.find(';');
      if (found != std::string::npos) {
        std::string ip=retChain.substr(found);
        std::string myName="Node"+std::to_string(GetNode()->GetId());
        if (1) { //isThisMyIpAddr(myName,ns3::Ipv4Address(dstAddr.c_str())) {
          retChain=retChain.substr(found+1);  //remove my ip address
          //next address is dest
          found = retChain.find(';');
          if (found != std::string::npos) {
            dstAddr = retChain.substr(0,found);
          }
        }
      }
      fatePkt.SetNamedAttribute("DstChain", retChain); //switch values
      fatePkt.SetNamedAttribute("ReturnChain","");

      //change 'from' to next IP
      //is this needed?
      Ipv4Address ipDest(dstAddr.c_str());
      uint16_t port =  InetSocketAddress::ConvertFrom (from).GetPort ();
      from = InetSocketAddress(ipDest,port);
//change packet dst IP
      fatePkt.SetNamedAttribute("Ipv4Dst",dstAddr  );

    }
    Ptr< Packet> returnPkt=0;
    std::cout << "server send:" << fatePkt << "\n";
    FateIpv4Interface::FatePktToIpv4(fatePkt, returnPkt);
    //returnPkt->Print(std::cout); //std::cout << "packet recived is:" << Packet::Ipv4ToFatePkt(returnPkt,0) << std::endl;
    if(match) {
      m_statTotalDataTxSize += packet->GetSize();
    } else {
      m_statTotalNotMatchTxSize += packet->GetSize();
    }

    NS_LOG_LOGIC ("Echoing packet");
    //socket->SendTo (packet, 0, from);
    socket->SendTo (returnPkt, 0, from);
    m_statNumPktsTx++;
    if (InetSocketAddress::IsMatchingType (from))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                   InetSocketAddress::ConvertFrom (from).GetPort ());
    }
    else if (Inet6SocketAddress::IsMatchingType (from))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                   Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                   Inet6SocketAddress::ConvertFrom (from).GetPort ());
    }
  }
}

void
UdpFateFileZipfServer::PrintStats(std::ostream &os) const
{
  os << "m_statNumPktsTx:" << m_statNumPktsTx << "\n";
  os << "m_statNumIntPktRx:" << m_statNumIntPktRx << "\n";
  os << "m_statNumDataPktTx:" << m_statNumDataPktTx << "\n";
  os << "m_statNumNotMatchPktTx:" << m_statNumNotMatchPktTx << "\n";
  os << "m_statTotalDataTxSize:" << m_statTotalDataTxSize << "\n";
  os << "m_statTotalNotMatchTxSize:" << m_statTotalNotMatchTxSize << "\n";
  os << "m_statTotalIntRxSize:" << m_statTotalIntRxSize << "\n";
  os << "m_statNumErrorPkts:" << m_statNumErrorPkts << "\n";
}

} // Namespace ns3
