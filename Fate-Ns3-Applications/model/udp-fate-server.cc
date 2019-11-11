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
#include "udp-fate-server.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpFateServerApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpFateServer);

TypeId
UdpFateServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpFateServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpFateServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpFateServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ReturnSize", "Size of the data attribute in the packet, using <DATA> attribute tag.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&UdpFateServer::m_size),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MinMatchName", 
                   "Exact Packet Name with minimal matching attributes",
                   StringValue (""),
                   MakeStringAccessor (&UdpFateServer::m_setNameMatch),
                   MakeStringChecker ())  
    .AddAttribute ("ReturnPath", 
                   "Exact Packet Name with minimal matching attributes",
                   StringValue (""),
                   MakeStringAccessor (&UdpFateServer::m_returnPath),
                   MakeStringChecker ())  ;
  return tid;
}
void
UdpFateServer::GetMatchNameAndPort(std::string &matchName, uint16_t &port)
{
   matchName = m_setNameMatch;
   port = m_port;
}
UdpFateServer::UdpFateServer ()
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
}

UdpFateServer::~UdpFateServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
  //m_partMatch.clear();
}

void
UdpFateServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}


void 
UdpFateServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

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

  m_socket->SetRecvCallback (MakeCallback (&UdpFateServer::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&UdpFateServer::HandleRead, this));
}

void 
UdpFateServer::StopApplication ()
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
UdpFateServer::SetPartialMatch(const std::list< std::pair< std::string, std::string > > &match)
{
   //m_partMatch = match;
}

void 
UdpFateServer::HandleRead (Ptr<Socket> socket)
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
   std::cout << "servera:" << fatePkt << "\n";
      
      fatePkt.ClearTempData();
      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();
     
      //Check if correct matching name
      bool match = true;
      if (!m_setNameMatch.empty()) {
         IcnName < std::string> matchName, pktName;
         matchName.SetFullName(m_setNameMatch);
         	pktName.SetFullName(fatePkt.GetAcclName());
         //pktName = fatePkt.GetName();
         match = pktName.PartialAttributeMatch(matchName);
      }
      //if true, it is a data pkt
      //rx itnerest stats
      if (fatePkt.GetPacketPurpose() == PktType::INTERESTPKT) {
         m_statNumIntPktRx++;
         m_statTotalIntRxSize += packet->GetSize();
      }

      //reply and stats
      if (match && (fatePkt.GetPacketPurpose() == PktType::INTERESTPKT)) {
        fatePkt.SetPacketPurpose(PktType::DATAPKT);
        std::string data(m_size, 'X');
        fatePkt.SetNamedAttribute("DATA", data, false);
        m_statNumDataPktTx++;
        //m_statTotalDataTxSize += packet->GetSize();
      } else if (fatePkt.GetPacketPurpose() == PktType::INTERESTPKT) {
         //if false, it is a missed interestresponse packet
        //NS_LOG_INFO("BAD PKT:" << fatePkt);
        fatePkt.SetPacketPurpose(PktType::INTERESTRESPONSEPKT);
        //m_statTotalNotMatchTxSize += packet->GetSize();
        m_statNumNotMatchPktTx++;
        //assert(0);
        
      } else {  //do nothing in invalid packet 
        std::cout << "invalid packet of type:" << fatePkt.GetPacketPurpose() << "\n";
        return;
      }

      //mark it as hitting the server/producer
      fatePkt.SetUnsignedNamedAttribute("ServerHitNodeName", GetNode()->GetId(), false);
      std::string retChain;
      bool chainRetResult = fatePkt.GetNamedAttribute("ReturnChain", retChain);

if (!m_returnPath.empty())
{
    //std::string srcAddr;
   //   fatePkt.GetPrintedNamedAttribute("Ipv4Src", srcAddr, true);
std::stringstream a;
a << InetSocketAddress::ConvertFrom (from).GetIpv4 ();
//this addr (to be removed), intermediate, actual dst addr
    retChain=a.str()+";"+m_returnPath+a.str()+";";
    //std::string ret=m_returnPath+a.str()+";";
}

std::string dstAddr;
      fatePkt.GetPrintedNamedAttribute("Ipv4Dst", dstAddr, true);
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
Ipv4Address ipDest(dstAddr.c_str());
uint16_t port =  InetSocketAddress::ConvertFrom (from).GetPort ();
from = InetSocketAddress(ipDest,port);
//change packet dst IP
fatePkt.SetNamedAttribute("Ipv4Dst",dstAddr, true  );
      }
      Ptr< Packet> returnPkt=0;
      FateIpv4Interface::FatePktToIpv4(fatePkt, returnPkt);
      //returnPkt->Print(std::cout); //std::cout << "packet recived is:" << Packet::Ipv4ToFatePkt(returnPkt,0) << std::endl;
      //std::cout << "server:" << fatePkt << "\n";
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
UdpFateServer::PrintStats(std::ostream &os) const
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
