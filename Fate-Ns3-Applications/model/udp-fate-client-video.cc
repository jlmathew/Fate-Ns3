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
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "udp-fate-client-video.h"

#include "ns3/IcnName.h"
#include <iostream>
#include <ostream>
#include <string>
#include "ns3/nstime.h"
#include "ns3/PacketTypeBase.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/fateIpv4-interface.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpFateVideoClientApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpFateVideoClient);
//enable video and/or audio
//
TypeId
UdpFateVideoClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpFateVideoClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpFateVideoClient> ()
    .AddAttribute ("RemotePort", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&UdpFateVideoClient::m_peerPort),
                   MakeUintegerChecker<uint16_t>())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&UdpFateVideoClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("VideoLocation", 
                   "The requested video timer file (\%location/videoName.[av]-time.txt",
                   StringValue("/home/fallenangel/rtp"),
                   MakeStringAccessor(&UdpFateVideoClient::m_timerfilename), 
                   MakeStringChecker())
    .AddAttribute ("VideoReqName", 
                   "The requested video file",
                   StringValue(""),
                   MakeStringAccessor(&UdpFateVideoClient::m_filename), 
                   MakeStringChecker())
    .AddAttribute ("DecryptionType", "0=no encryption 1=decrypt I file(fake pub-priv), 2=decrypt audio, 4=decrypt video, 8=NetEnc I slices",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpFateVideoClient::m_frameDecrypt),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("PlayAudio", "Play audio with file",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateVideoClient::m_playaudio),
                   MakeBooleanChecker())
    .AddAttribute ("PlayVideo", "Play video",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateVideoClient::m_playvideo),
                   MakeBooleanChecker())
    .AddAttribute ("UseTCP", "false=UDP, true=TCP",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateVideoClient::m_tcp),
                   MakeBooleanChecker())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&UdpFateVideoClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
     .AddAttribute ("matchString", "Match string used to match to server",
                   StringValue("/test1/fileNum="),
                   MakeStringAccessor(&UdpFateVideoClient::m_matchName), 
                   MakeStringChecker())
     .AddAttribute ("matchByType", "Match name by 'location | filenum | segnum'",
                   StringValue("filenum"),
                   MakeStringAccessor(&UdpFateVideoClient::m_minMatchType), 
                   MakeStringChecker())
     .AddAttribute ("PktPayload", "Ascii XML representation of a packet, for ipv4 payload",
                   StringValue(""),
                   MakeStringAccessor(&UdpFateVideoClient::m_xmlpayload), 
                   MakeStringChecker())

  ;
  return tid;
}

UdpFateVideoClient::UdpFateVideoClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  //m_socketc = 0;
  m_socket = 0;
  //m_socketa = 0;
  m_sendEvent = EventId ();
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

UdpFateVideoClient::~UdpFateVideoClient()
{
  NS_LOG_FUNCTION (this);
  //m_socketc = 0;
  m_socket = 0;
  //m_socketa = 0;

  m_dataSize = 0;
}

void 
UdpFateVideoClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
UdpFateVideoClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
UdpFateVideoClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
UdpFateVideoClient::SetTimestamp ( bool timestamp)
{
  assert(0);
}

void
UdpFateVideoClient::decrypt(PktType &pkt, Address from, uint8_t *decryptData, uint16_t &length) 
{
   static bool inIslice=false;
   static uint64_t newKey=0;
   static uint64_t oldKey=0;
    //if I frame, start new decrypt key
   uint64_t pktnum;
   bool isliceC = pkt.GetUnsignedNamedAttribute("IPkt", pktnum);
     uint8_t buff[255];
     uint8_t *buffer=nullptr;
   //if we rx an I slice pkts, with an intermediate audio pkt, it kills the key generation
   //only mark the end if the packet is a video packet AND stops being I-slice

   //Only client has decrypt options, server always sends it encryted
//DANGER, if we rx an islice, we dont know it is the last pkt, so the audio could be
//incorrectly decrypted.   Should we tag last islice pkt?
   if (isliceC) {
     //use sender IP address (from)
     uint32_t size = from.CopyAllTo(buff,255); 
     inIslice=true;
     uint64_t key;
     if (size>=8) {  std::memcpy(&key, buff, sizeof(uint64_t)); }
     else { assert(0); } 
     if(!pkt.GetRawCpyNamedAttribute("DATA", &buffer, length, false)) { assert(0);}
     decryptData = new uint8_t[length]; 
     for(unsigned int i=0; i< (unsigned int) length; i += sizeof(key)) {
         uint64_t tempData =(*reinterpret_cast<uint64_t *>(&buffer[i])) ^ key;  
       
         memcpy(&decryptData[i], &tempData, sizeof(key));
     }
     //need to hash I-frames to make 'oldkey'
     oldKey=0; //read pkt and make hash key here for I-frames
     for(unsigned int i=0; i< (unsigned int) length; i += sizeof(key)) {
         oldKey=  *reinterpret_cast<uint64_t *>(&decryptData[i]) ^ oldKey;
     }
      delete[]buffer;
 
   } else {
      //use last decrypt key
      if (inIslice) {
        inIslice=false;
        newKey=oldKey;
        oldKey=0;
      }  
       //use newKey;  decode audio or any non-I slice video
      if(!pkt.GetRawCpyNamedAttribute("DATA", &buffer, length, false)) { assert(0);}
      decryptData = new uint8_t[length]; 
      for(unsigned int i=0; i< (unsigned int) length; i += sizeof(newKey)) {
         uint64_t tempData =(*reinterpret_cast<uint64_t *>(&buffer[i])) ^ newKey;  
         memcpy(&decryptData[i], &tempData, sizeof(newKey));
      }
      delete[]buffer;


   }
   //

}

void
UdpFateVideoClient::playPkt(const PktType &pkt)
{
   //send it to real ports on ffplay 

}

void
UdpFateVideoClient::initPlayPkt()
{
    //open ffplay using sdp file
}
void
UdpFateVideoClient::parseTiming(const std::string &filename, bool isVideo)
{
 std::ifstream file (filename.c_str ());
 if (!file.is_open()) { assert(0);}
 std::string stream; 
 std::string junk;
 uint64_t var;
while (getline(file, stream)) {

   std::stringstream input;
  input.str (stream);
  input >> junk;
  input >> var;
  input >> junk;
  input >> junk;
  input >> junk;
  input >> junk;
  input >> junk;
  input >> junk;
   if(isVideo) 
     m_videoTime.push_back(NanoSeconds(var));
   else
     m_audioTime.push_back(NanoSeconds(var));
 }
 file.close();
}


void 
UdpFateVideoClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      TypeId tid2 = TypeId::LookupByName ("ns3::TcpSocketFactory");
      //m_socketc = Socket::CreateSocket (GetNode (), tid2);
      if (m_tcp) {
      //m_socketv = Socket::CreateSocket (GetNode (), tid2);
      m_socket = Socket::CreateSocket (GetNode (), tid2);
      } else { 
      m_socket = Socket::CreateSocket (GetNode (), tid);
      //m_socketa = Socket::CreateSocket (GetNode (), tid);
      }
               //m_socketc->Bind();
               m_socket->Bind();
               //m_socketa->Bind();
          //m_socketc->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPortc));
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
std::cout << "client connect:" << Ipv4Address::ConvertFrom(m_peerAddress) << " on port " << m_peerPort << "\n";
          //m_socketa->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPorta));
 
  //m_socketc->SetRecvCallback (MakeCallback (&UdpFateVideoClient::HandleReadc, this));
  m_socket->SetRecvCallback (MakeCallback (&UdpFateVideoClient::HandleReadv, this));
  //m_socket->SetRecvCallback (MakeCallback (&UdpFateVideoClient::HandleReada, this));
  //m_socket->SetAllowBroadcast (true);
  std::string parseName=m_timerfilename+"/"+m_filename+"/"+m_filename; //.append("/");
  //parseName.append("/");
  parseName.append(".v-time.txt");
  parseTiming(parseName,true);
  parseName=m_timerfilename+"/"+m_filename+"/"+m_filename; //.append("/");
  parseName.append(".a-time.txt");
  parseTiming(parseName,false);
  ScheduleTransmit (Seconds (0.));
}

void 
UdpFateVideoClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

      //m_socketc->Close ();
      //m_socketc->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      //m_socketc = 0;
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
      //m_socketa->Close ();
      //m_socketa->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      //m_socketa = 0;

  Simulator::Cancel (m_sendEvent);
}



void 
UdpFateVideoClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  //m_sendEvent = Simulator::Schedule (dt, &UdpFateVideoClient::Sendc, this);
  getVideoFile();
  getAudioFile();
}


std::string
UdpFateVideoClient::getVideoSdf()
{
std::string newName=m_timerfilename;
newName.append("/");
newName.append(m_filename);
newName.append("/");
newName.append(m_filename);
newName.append(".sdf");
 return newName;
}

void 
UdpFateVideoClient::getAudioFile (void)
{
std::string newName="/"+m_filename+"/"+m_filename; //.append("/");
newName.append(".a.pk");
	  std::stringstream input;
input << ++m_audioSeg;
newName.append(input.str());
IcnName<std::string> name;
name.SetFullName(newName);
       PktType fatePkt;
       fatePkt.SetPacketPurpose(PktType::INTERESTPKT);
      fatePkt.SetName(name);


   std::vector<uint8_t> fateData;
   fatePkt.Serialize(fateData);
   uint8_t *data = new uint8_t[fateData.size()];
   for(unsigned i = 0; i < fateData.size(); i++) {
      data[i] = fateData[i];
   }
   Ptr<Packet> p = Create<Packet>(data, fateData.size());
   delete data; 
   Senda(p);
   Simulator::Schedule (m_audioTime[m_audioSeg], &UdpFateVideoClient::getAudioFile, this);
   
 
}

void 
UdpFateVideoClient::getVideoFile (void)
{
  NS_LOG_FUNCTION (this);
//std::string newName=m_currentVidName;
std::string newName="/"+m_filename+"/"+m_filename; //.append("/");
newName.append(".v.pk");
	  std::stringstream input;
input << ++m_videoSeg;
newName.append(input.str());
IcnName<std::string> name;
name.SetFullName(newName);
       PktType fatePkt;
       fatePkt.SetPacketPurpose(PktType::INTERESTPKT);
      fatePkt.SetName(name);


   std::vector<uint8_t> fateData;
   fatePkt.Serialize(fateData);
   uint8_t *data = new uint8_t[fateData.size()];
   for(unsigned i = 0; i < fateData.size(); i++) {
      data[i] = fateData[i];
   }
   Ptr<Packet> p = Create<Packet>(data, fateData.size());
   delete data; 
   Sendv(p);
   Simulator::Schedule (m_videoTime[m_videoSeg], &UdpFateVideoClient::getVideoFile, this);

}


//
void
UdpFateVideoClient::Sendc(Ptr<Packet> p)
{
  m_txTrace (p);
  //m_socketc->Send (p);

  ++m_sent;
  ++m_statNumPktsTx;
  m_statTotalTxSize+=m_size;


 }
void
UdpFateVideoClient::Sendv(Ptr<Packet> p)
{
  m_txTrace (p);
  m_socket->Send (p);

  ++m_sent;
  ++m_statNumPktsTx;
  m_statTotalTxSize+=m_size;


 }
void
UdpFateVideoClient::Senda(Ptr<Packet> p)
{
  m_txTrace (p);
  m_socket->Send (p);

  ++m_sent;
  ++m_statNumPktsTx;
  m_statTotalTxSize+=m_size;


 }
/*
void
UdpFateVideoClient::HandleReadc (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = m_socketc->RecvFrom (from)))
    { 
      PktType fatePkt;
      bool valid = FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
      NS_ASSERT(valid);
      if (fatePkt.GetPacketPurpose() == PktType::DATAPKT) {
        m_statNumDataPktRx++;
        m_statTotalDataRxSize += fatePkt.size();
          //Send to ffplay?

      } else {
        m_statNumIRPktRx++;
        m_statTotalIrRxSize += fatePkt.size();

      }

    }
    

}
*/
void
UdpFateVideoClient::HandleReadv (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  uint8_t *buffer=nullptr;
  uint16_t length;
  while ((packet = m_socket->RecvFrom (from)))
    {
      PktType fatePkt;
      bool valid = FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
      NS_ASSERT(valid);
      if (fatePkt.GetPacketPurpose() == PktType::DATAPKT) {
        m_statNumDataPktRx++;
        m_statTotalDataRxSize += fatePkt.size();
          //decrypt?
          decrypt(fatePkt, from, buffer, length);
          //Send to ffplay?
          if (false) { //option to send it to ffplay
             playPkt(fatePkt);
             delete[] buffer;
          }
      } else {
        m_statNumIRPktRx++;
        m_statTotalIrRxSize += fatePkt.size();

      }

    }

}
/*
void
UdpFateVideoClient::HandleReada (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = m_socket->RecvFrom (from)))
    {
      PktType fatePkt;
      bool valid = FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
      NS_ASSERT(valid);
      if (fatePkt.GetPacketPurpose() == PktType::DATAPKT) {
        m_statNumDataPktRx++;
        m_statTotalDataRxSize += fatePkt.size();
          //Send to ffplay?
        std::vector<uint8_t> pktData;
       //fatePkt.GetNamedAttribute("Data", pktData); 
      } else {
        m_statNumIRPktRx++;
        m_statTotalIrRxSize += fatePkt.size();

      }

    }

}
*/

void
UdpFateVideoClient::PrintStats(std::ostream &os) const
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
