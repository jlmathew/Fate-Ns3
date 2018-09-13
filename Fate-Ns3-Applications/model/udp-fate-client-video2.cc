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
#include "udp-fate-client-video2.h"

#include "ns3/IcnName.h"
#include <iostream>
#include <ostream>
#include <string>
#include <stdlib.h>
#include "ns3/PacketTypeBase.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/fateIpv4-interface.h"
#include "ns3/string.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpFateVideoClient2Application");

NS_OBJECT_ENSURE_REGISTERED (UdpFateVideoClient2);

TypeId
UdpFateVideoClient2::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpFateVideoClient2")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpFateVideoClient2> ()
    .AddAttribute ("Jitter", 
                   "The time to wait between videos",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&UdpFateVideoClient2::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&UdpFateVideoClient2::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpFateVideoClient2::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
     .AddAttribute ("AddTimeStamp", "Add a timestamp to each packet",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateVideoClient2::m_timestamp),
                   MakeBooleanChecker())
     .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&UdpFateVideoClient2::m_txTrace),
                     "ns3::Packet::TracedCallback")
     .AddAttribute ("NStaticDestination", "Use a producer created destination table to route packets",
                    BooleanValue (true),
                    MakeBooleanAccessor (&UdpFateVideoClient2::m_nStaticDest),
                    MakeBooleanChecker())
     .AddAttribute ("VideoLocation", "Video parent directory.",
                   StringValue("/home/fallenangel/rtp"),
                   MakeStringAccessor(&UdpFateVideoClient2::m_location), 
                   MakeStringChecker())
     .AddAttribute ("VideoReqName", "Video directory",
                   StringValue("ps4"),
                   MakeStringAccessor(&UdpFateVideoClient2::m_filename), 
                   MakeStringChecker())
     .AddAttribute ("matchString", "Match string used to match to server",
                   StringValue("/ps4"),
                   MakeStringAccessor(&UdpFateVideoClient2::m_matchName), 
                   MakeStringChecker())
     .AddAttribute ("matchByType", "Match name by 'location | filenum | segnum'",
                   StringValue("filenum"),
                   MakeStringAccessor(&UdpFateVideoClient2::m_minMatchType), 
                   MakeStringChecker())
    .AddAttribute ("DecryptionType", "0=no encryption 1=decrypt I file(fake pub-priv), 2=decrypt audio, 4=decrypt video, 8=NetEnc I slices",
                   UintegerValue (5),
                   MakeUintegerAccessor (&UdpFateVideoClient2::m_frameDecrypt),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("PlayAudio", "Play audio with file",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateVideoClient2::m_playaudio),
                   MakeBooleanChecker())
    .AddAttribute ("PlayVideo", "Play video",
                   BooleanValue (true),
                   MakeBooleanAccessor (&UdpFateVideoClient2::m_playvideo),
                   MakeBooleanChecker())
    .AddAttribute ("UseTCP", "false=UDP, true=TCP",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpFateVideoClient2::m_tcp),
                   MakeBooleanChecker())
     .AddAttribute ("PktPayload", "Ascii XML representation of a packet, for ipv4 payload",
                   StringValue(""),
                   MakeStringAccessor(&UdpFateVideoClient2::m_xmlpayload), 
                   MakeStringChecker())
  ;
  return tid;
}

UdpFateVideoClient2::UdpFateVideoClient2 ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
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

UdpFateVideoClient2::~UdpFateVideoClient2()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void
UdpFateVideoClient2::decrypt(PktType &pkt, Address from) 
{

   static bool inIslice=false;
   static uint64_t newKey=0;
   static uint64_t oldKey=0;
    //if I frame, start new decrypt key
   uint64_t pktnum;
   bool isliceC = pkt.GetUnsignedNamedAttribute("IPkt", pktnum);
     uint8_t buff[255];
     uint8_t *buffer=nullptr;
    uint8_t *decryptData=nullptr;
    uint16_t length;
   //if we rx an I slice pkts, with an intermediate audio pkt, it kills the key generation
   //only mark the end if the packet is a video packet AND stops being I-slice

   //Only client has decrypt options, server always sends it encryted
//DANGER, if we rx an islice, we dont know it is the last pkt, so the audio could be
//incorrectly decrypted.   Should we tag last islice pkt?
   if (isliceC && (m_frameDecrypt & 1)) {
     //use sender IP address (from)
     uint32_t size = from.CopyAllTo(buff,255); 
     inIslice=true;
     uint32_t key2;
     uint64_t key;
     if (size>=4) {  std::memcpy(&key2, buff, sizeof(uint32_t)); key=key2; }
     else { assert(0); } 
//std::cout << "client i-frame key:" << key << "\n";
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
      pkt.SetRawCpyNamedAttribute("DATA", decryptData, length, false);
      delete[]buffer; delete[] decryptData; 
 
   } else if (m_frameDecrypt > 1) {
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
      pkt.SetRawCpyNamedAttribute("DATA", decryptData, length, false);
      delete[]buffer; delete[] decryptData;


   } else {
   }
   //

}

void
UdpFateVideoClient2::playPkt( PktType &pkt)
{
   //send it to real ports on ffplay 
   //video is port 20002, audio is 30003

uint8_t *buf2;
bool isvideo=true;
std::string name=pkt.GetName().GetName();
auto found=name.find(".v.pk");
if (found > name.size())
{
   isvideo=false;
}

uint16_t size;
 pkt.GetRawCpyNamedAttribute("DATA", &buf2, size, false);

if (isvideo) {
   sendto(sockfdv, buf2, size, 0,   //this line forwards saved packets 
	       (struct sockaddr *) &clientaddr, sizeof(clientaddr));
} else {
   sendto(sockfda, buf2, size, 0,   //this line forwards saved packets 
	       (struct sockaddr *) &clientaddr2, sizeof(clientaddr));
}

delete[]buf2;
}

void
UdpFateVideoClient2::initPlayPkt()
{
    //open ffplay using sdp file
  //std::string sdpname = "vlc -vvv " ;
  //std::string sdpname = "xterm -hold -e /home/fallenangel/rtp/ffmpeg/ffmpeg/ffplay -protocol_whitelist file,crypto,udp,rtp -i " ;
  std::string sdpname = "xterm -e ffplay -protocol_whitelist file,crypto,udp,rtp -i " ;
  sdpname.append(getVideoSdp());
sdpname.append(" &");
std::cout << sdpname << "\n";
  system(sdpname.c_str());

  sockfdv = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
  sockfda = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
  if (sockfdv < 0) 
      NS_ASSERT(sockfdv > 0);
  if (sockfda < 0) 
      NS_ASSERT(sockfda > 0);

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
 int optval = 1;
  setsockopt(sockfdv, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));
  setsockopt(sockfda, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */

  bzero((char *) &clientaddr, sizeof(clientaddr));
  bzero((char *) &clientaddr2, sizeof(clientaddr));
  clientaddr.sin_family = AF_INET;
  clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  clientaddr.sin_port = htons((unsigned short) 20002);  //ffplay
  clientaddr2.sin_family = AF_INET;
  clientaddr2.sin_addr.s_addr = htonl(INADDR_ANY);
  clientaddr2.sin_port = htons((unsigned short) 30003);  //ffplay


}


void
UdpFateVideoClient2::HandleReadv (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = m_socket->RecvFrom (from)))
    {
      PktType fatePkt;
      bool valid = FateIpv4Interface::Ipv4ToFatePkt(packet, 0, fatePkt);
      NS_ASSERT(valid);

if(fatePkt.GetPacketPurpose() != PktType::DATAPKT)
{ std::cout << " Non data packet rx:" << fatePkt << "\n";
}
else {
//std::cout << "rx fate:" << fatePkt << "\n";
}

      if (fatePkt.GetPacketPurpose() == PktType::DATAPKT) {
        m_statNumDataPktRx++;
        m_statTotalDataRxSize += fatePkt.size();
          //decrypt?
          if (m_frameDecrypt != 0)
              decrypt(fatePkt, myTxAddress);
          
//Send to ffplay?
          if (m_playvideo) { //option to send it to ffplay
             playPkt(fatePkt);
          }
      } else {
        m_statNumIRPktRx++;
        m_statTotalIrRxSize += fatePkt.size();

      }

    }

}

void 
UdpFateVideoClient2::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
UdpFateVideoClient2::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}
void 
UdpFateVideoClient2::SetTimestamp(bool timestamp) {
  m_timestamp = timestamp;
}

void
UdpFateVideoClient2::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}


//JLM NOTE TO SELF: FIXME TODO only start applications are switched
void 
UdpFateVideoClient2::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      TypeId tid2 = TypeId::LookupByName ("ns3::TcpSocketFactory");
      //m_socketc = Socket::CreateSocket (GetNode (), tid2);
      if (m_tcp) {
      m_socket = Socket::CreateSocket (GetNode (), tid2);
      } else { 
      m_socket = Socket::CreateSocket (GetNode (), tid);
      }
               m_socket->Bind();

            ipPort_t info = GetProdNodeIpv4(m_matchName);
            Ipv4Address ipv4(info.first);
            uint16_t port= info.second;
            myTxAddress=InetSocketAddress(ipv4,port);
            InetSocketAddress dest(ipv4,port);
            m_socket->SetRecvPktInfo(true);
            m_socket->SetAllowBroadcast (true);
            //dest.SetTos(tos);  //if we need to set tos
            m_socket->Connect(dest);
            

          //m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
std::cout << "client connect:" << ipv4 << " on port " << port << "\n";
//std::cout << "client connect:" << Ipv4Address::ConvertFrom(m_peerAddress) << " on port " << m_peerPort << "\n";
          //m_socketa->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPorta));
 
  m_socket->SetRecvCallback (MakeCallback (&UdpFateVideoClient2::HandleReadv, this));
   m_socket->SetAllowBroadcast (true);

  if (m_xmlpayload.size()) {
    std::stringstream(m_xmlpayload) >> m_payload; 
  }
  m_payload.GetUnsignedNamedAttribute("TtlHop", m_startHops);

  std::string parseName=m_location+"/"+m_filename+"/"+m_filename; //.append("/");
  //parseName.append("/");
  parseName.append(".v-time.txt");
  parseTiming(parseName,true);
  parseName=m_location+"/"+m_filename+"/"+m_filename; //.append("/");
  parseName.append(".a-time.txt");
  parseTiming(parseName,false);
  ScheduleTransmit (Seconds (0.));

//only if we play video

initPlayPkt();
}
void
UdpFateVideoClient2::parseTiming(const std::string &filename, bool isVideo)
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
UdpFateVideoClient2::StopApplication ()
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
UdpFateVideoClient2::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
UdpFateVideoClient2::SetPktPayload (const std::string &xml)
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
UdpFateVideoClient2::SetPktPayload (const PktType &payload)
{
  m_payload = payload;
     std::vector<uint8_t> fateData;
   m_payload.Serialize(fateData);
   m_size = fateData.size();
   m_payload.GetUnsignedNamedAttribute("TtlHop", m_startHops);


}

void 
UdpFateVideoClient2::SetPktPayload (const IcnName<std::string> &name)
{
  m_payload.ClearData();
  m_payload.SetName(name);
}

void 
UdpFateVideoClient2::SetPktPayload (uint8_t *fill, uint32_t dataSize)
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
UdpFateVideoClient2::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  //m_sendEvent = Simulator::Schedule (dt, &UdpFateVideoClient2::Send, this);
  //m_sendEvent = Simulator::Schedule (dt, &UdpFateVideoClient2::Senda, this);
  getVideoFile();
  getAudioFile();
}



void
UdpFateVideoClient2::PrintStats(std::ostream &os) const
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

std::string
UdpFateVideoClient2::getVideoSdp()
{
std::string newName=m_location;
//newName.append("/");
newName.append(m_filename);
newName.append("/");
newName.append(m_filename);
newName.append(".sdp");
 return newName;
}

void 
UdpFateVideoClient2::getAudioFile (void)
{
std::string newName="/"+m_filename+"/"+m_filename; //.append("/");
newName.append(".a.pk");
	  std::stringstream input;
input << ++m_audioSeg;
if (m_audioSeg > m_audioTime.size())
{  return;}  //done with audio
newName.append(input.str());
IcnName<std::string> name;
name.SetFullName(newName);
       PktType fatePkt;
       fatePkt.SetPacketPurpose(PktType::INTERESTPKT);
      fatePkt.SetName(name);

//std::cout << name << "\n";

   std::vector<uint8_t> fateData;
   fatePkt.Serialize(fateData);
   uint8_t *data = new uint8_t[fateData.size()];
   for(unsigned i = 0; i < fateData.size(); i++) {
      data[i] = fateData[i];
   }
   Ptr<Packet> p = Create<Packet>(data, fateData.size());
   delete data; 
   Senda(p);
   Simulator::Schedule (m_audioTime[m_audioSeg], &UdpFateVideoClient2::getAudioFile, this);
   
 
}
void
UdpFateVideoClient2::Sendv(Ptr<Packet> p)
{
  m_txTrace (p);
  m_socket->Send (p->Copy());

  ++m_sent;
  ++m_statNumPktsTx;
  m_statTotalTxSize+=m_size;


 }
void
UdpFateVideoClient2::Senda(Ptr<Packet> p)
{
  m_txTrace (p);
  m_socket->Send (p->Copy());

  ++m_sent;
  ++m_statNumPktsTx;
  m_statTotalTxSize+=m_size;


 }
void 
UdpFateVideoClient2::getVideoFile (void)
{
  NS_LOG_FUNCTION (this);
//std::string newName=m_currentVidName;
std::string newName="/"+m_filename+"/"+m_filename; //.append("/");
newName.append(".v.pk");
	  std::stringstream input;
input << ++m_videoSeg;
if (m_videoSeg > m_videoTime.size())
{  return;}  //done with video 
newName.append(input.str());
IcnName<std::string> name;
name.SetFullName(newName);
       PktType fatePkt;
       fatePkt.SetPacketPurpose(PktType::INTERESTPKT);
      fatePkt.SetName(name);
//std::cout << name << "\n";

   std::vector<uint8_t> fateData;
   fatePkt.Serialize(fateData);
   uint8_t *data = new uint8_t[fateData.size()];
   for(unsigned i = 0; i < fateData.size(); i++) {
      data[i] = fateData[i];
   }
//std::cout << "clientsend:" << fatePkt << "\n";
   Ptr<Packet> p = Create<Packet>(data, fateData.size());
   delete data; 
   Sendv(p);
   Simulator::Schedule (m_videoTime[m_videoSeg], &UdpFateVideoClient2::getVideoFile, this);

}

} // Namespace ns3
