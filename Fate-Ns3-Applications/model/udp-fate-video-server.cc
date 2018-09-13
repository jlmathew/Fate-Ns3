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
#include "udp-fate-video-server.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include "ns3/boolean.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpFateVideoServerApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpFateVideoServer);

TypeId
UdpFateVideoServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpFateVideoServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpFateVideoServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpFateVideoServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MinMatchName", 
                   "Exact Packet Name with minimal matching attributes",
                   StringValue (""),
                   MakeStringAccessor (&UdpFateVideoServer::m_setNameMatch),
                   MakeStringChecker ())  
    .AddAttribute ("VideoReqName", 
                   "sub Directory of video file",
                   StringValue ("ps4"),
                   MakeStringAccessor (&UdpFateVideoServer::m_vidSubDir),
                   MakeStringChecker ()) 
    .AddAttribute ("EncryptISlice", "public-private I slice encrypt",
                   BooleanValue (true),
                   MakeBooleanAccessor (&UdpFateVideoServer::m_encISlice),
                   MakeBooleanChecker())
    .AddAttribute ("EncryptVideo", "encrypt video via I-slice hash",
                   BooleanValue (true),
                   MakeBooleanAccessor (&UdpFateVideoServer::m_encVideo),
                   MakeBooleanChecker())
    .AddAttribute ("EncryptAudio", "Encrypt audio",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateVideoServer::m_encAudio),
                   MakeBooleanChecker())
    .AddAttribute ("Use1stISliceHash", "Use the first I slice pkts, as opposed to the entire I slice",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateVideoServer::m_netEnc),
                   MakeBooleanChecker())
    .AddAttribute ("VideoLocation", 
                   "Location of video segments",
                   StringValue ("/home/fallenangel/rtp/"),
                   MakeStringAccessor (&UdpFateVideoServer::m_setLocation),
                   MakeStringChecker ())  
     ;
  return tid;
}
void
UdpFateVideoServer::GetMatchNameAndPort(std::string &matchName, uint16_t &port)
{
   matchName = m_setNameMatch;
   port = m_port;
}
UdpFateVideoServer::UdpFateVideoServer ()
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

UdpFateVideoServer::~UdpFateVideoServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
  //m_partMatch.clear();
}

void
UdpFateVideoServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}


void 
UdpFateVideoServer::StartApplication (void)
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
  std::string name = m_setLocation;
  name.append(m_vidSubDir);
  name.append("/");
  name.append(m_vidSubDir);
  parseHash(name);
  m_socket->SetRecvCallback (MakeCallback (&UdpFateVideoServer::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&UdpFateVideoServer::HandleRead, this));
}

void 
UdpFateVideoServer::StopApplication ()
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
UdpFateVideoServer::SetPartialMatch(const std::list< std::pair< std::string, std::string > > &match)
{
   //m_partMatch = match;
}

void 
UdpFateVideoServer::HandleRead (Ptr<Socket> socket)
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
      //FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
      bool valid = FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
      if(!valid) {
         std::cout << "server rx invalid pkt\n";
      }
      assert(valid);
      fatePkt.ClearTempData();
      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();
     
      //Check if correct matching name
      bool match = true;
      if (!m_setNameMatch.empty()) {
         IcnName < std::string> matchName, pktName;
         matchName.SetFullName(m_setNameMatch);
         pktName = fatePkt.GetName();
         match = pktName.PathMatch(matchName);
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
        //td::string data(m_size, 'X');
        //fatePkt.SetNamedAttribute("DATA", data, false);
        m_statNumDataPktTx++;

        bool found= getPktData(fatePkt, from);
        if(found) {
          //fatePkt.SetRawCpyNamedAttribute("DATA", &v[0], v.size(), false);
          m_statNumDataPktTx++;
        } else {
          fatePkt.SetPacketPurpose(PktType::INTERESTRESPONSEPKT);  //file doesnt exist 
        }


      } else if (fatePkt.GetPacketPurpose() == PktType::INTERESTPKT) {
         //if false, it is a missed interestresponse packet
        fatePkt.SetPacketPurpose(PktType::INTERESTRESPONSEPKT);
        m_statNumNotMatchPktTx++;
        
      } else {  //do nothing in invalid packet 
        std::cout << "invalid packet of type:" << fatePkt.GetPacketPurpose() << "\n";
        return;
      }

      //mark it as hitting the server/producer
      fatePkt.SetUnsignedNamedAttribute("ServerHitNodeName", GetNode()->GetId(), false);
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
UdpFateVideoServer::PrintStats(std::ostream &os) const
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
void
UdpFateVideoServer::parseHash(const std::string &filename)
{
 std::string filen=filename;
 filen.append(".v.encrypt");
 std::ifstream file (filen.c_str ());

uint32_t startFrame=1;
uint64_t lastHash=0;
//open both files, and hash on them
 while(!file.eof()) {
      std::string junk, stream;
      getline(file,stream);
      uint32_t iFrameStart,iFrameEnd;
      uint64_t hash;
      uint64_t hash1;
      std::stringstream input;
      input.str (stream);
      input >> junk;
      input >> junk;
      input >> iFrameStart;
      input >> junk;
      input >> iFrameEnd;
      input >> junk;
      input >> junk;
      input >> junk;
      input >> junk;
      input >> junk;
      input >> hash;
      input >> junk;
      input >> hash1;

      vidInfo info;
      for(auto i= startFrame; i<iFrameStart; i++) { 
        info.hash=lastHash;
        info.frameType='U';
        m_IsliceHash[i]=info;
      }
      for(auto i=iFrameStart; i<=iFrameEnd; i++) { 
        if(m_netEnc) {
          info.hash=hash1;
          if (i==iFrameStart)
             info.frameType='I';
             
        } else {
          info.hash=hash; //doesnt make a difference for I frames
        info.frameType='I';
        }
        info.frameType='I';
        m_IsliceHash[i]=info;
       
      }
        if(m_netEnc) {
          lastHash=hash1;
        } else {
          lastHash=hash; //doesnt make a difference for I frames
        }
      startFrame=iFrameEnd+1; 
  }   
  file.close();
  filen=filename;
        if(m_netEnc) {
  filen.append(".a2.encrypt");
} else {
  filen.append(".a.encrypt");
}
 std::ifstream filea (filen.c_str ());

uint32_t startAFrame=1;
uint64_t lastAHash=0;
//open both files, and hash on them
 while(!filea.eof()) {
      std::string junk, stream;
      getline(filea,stream);
      uint32_t aFrameStart;
      uint64_t hash;
      std::stringstream input;
      input.str (stream);
      input >> junk;
      input >> aFrameStart;
      input >> junk;
      input >> junk;
      input >> junk;
      input >> junk;
      input >> junk;
      input >> junk;
      input >> junk;
      input >> hash;

      for(auto i= startAFrame; i<aFrameStart; i++) { 
        m_audioHash[i]=lastAHash;
      }
        lastAHash = hash;
      startAFrame=aFrameStart; 
  }   
  filea.close();
 
}

bool
UdpFateVideoServer::getPktData(PktType &pkt, Address from)
{
  std::string filen=m_setLocation;
  auto icnname=pkt.GetName();
  filen.append(m_vidSubDir); 
  filen.append("/");
  filen.append(icnname.GetName());  //should be name.v.pkxx or name.a.pktyyy
    //filen.append(".pk");
  uint32_t ithpkt = 0;


    //filen.append(std::to_string(segment));
  std::ifstream file (filen.c_str (),  std::ifstream::binary);
  if (!file) { return false; } //file not found
  file.seekg(0, file.end);
  int length = file.tellg();
  file.seekg(0, file.beg);
   uint8_t *buffer = new uint8_t[length];
   uint8_t *bufferEnc = new uint8_t[length];
   file.read(reinterpret_cast<char *>(buffer), length);


//debug
//length=1556;
//uint8_t bu[1556];
//for(unsigned int i=0; i< (unsigned) length; i++)
//    bu[i]=i % 256;
 //pkt.SetRawCpyNamedAttribute("DATA", bu, length, false);

//check if we send unencoded
if (!(m_encISlice) || m_encVideo || m_encAudio || m_netEnc) {
   pkt.SetRawCpyNamedAttribute("DATA", buffer, length, false);
  return true;
}

   //pkt.Set("Slice", "I");
   //encrypt I slice, do ximple xor
   uint32_t key2;
   uint64_t key;
   //if key is not an iframe, get key
   ithpkt=filen.find(".v.pk"); //=0 is audio, else vido
   if(ithpkt > filen.size()) {
       ithpkt=0;
    }
   char vidtype='U';
   if (ithpkt) {  //its video, check for i frame
       ithpkt=std::stoul(filen.substr(ithpkt+5));
       auto it = m_IsliceHash.find(ithpkt);
       if (it == m_IsliceHash.end()
) { assert(0); }
       key = it->second.hash;  //get i frame hash
       vidtype=it->second.frameType;
   } else {  //audio
     ithpkt=filen.find(".a.pk"); //=0 is audio, else vido
     ithpkt=std::stoul(filen.substr(ithpkt+5));
     auto it = m_audioHash.find(ithpkt);
       if (it == m_audioHash.end()
)         { assert(0); }
       key = it->second;  //get i frame hash
   }


//ithpkt =0 means no encryption
static bool firstI=true;
   if (vidtype == 'I') {
      pkt.SetUnsignedNamedAttribute("IPkt", ithpkt);
      if(firstI) {
        pkt.SetUnsignedNamedAttribute("1stI", firstI);
        firstI=false;
        if (m_frameDecrypt & 8) 
           pkt.SetUnsignedNamedAttribute("CacheTime", 0);
      }
      if (!(m_frameDecrypt & 8)) 
          pkt.SetUnsignedNamedAttribute("CacheTime", 0);
     //encrypt I slice, do ximple xor with sender IP
//mark end of I slice ...
      uint8_t buff[255];
     uint32_t size = from.CopyAllTo(buff,255); 
     if (size>=4) {  std::memcpy(&key2, buff, sizeof(uint32_t)); key = key2;}
     else { assert(0); }    
//std::cout << "server i-frame key:" << key << "\n";
     for(unsigned int i=0; i< (unsigned int) length; i += sizeof(key)) {
         uint64_t newval = (*(reinterpret_cast<uint64_t *>(&buffer[i]))) ^ key;
         std::memcpy(&bufferEnc[i], &newval, sizeof(key));
     }
   } else if (m_encVideo) {  //audio or NOT an I slice
     firstI=true;
     for(unsigned int i=0; i< (unsigned int) length; i += sizeof(key)) {
         uint64_t newval = (*(reinterpret_cast<uint64_t *>(&buffer[i]))) ^ key;
         std::memcpy(&bufferEnc[i], &newval, sizeof(key));
         //std::memset(&bufferEnc[i], (*reinterpret_cast<uint64_t *>(&buffer[i])) ^ key, sizeof(key));
     }
     
   } else {
     std::memcpy(bufferEnc, buffer, length);
   }
   pkt.SetRawCpyNamedAttribute("DATA", bufferEnc, length, false);
   delete[] buffer;
   delete[] bufferEnc;
   return true;

}

} // Namespace ns3
