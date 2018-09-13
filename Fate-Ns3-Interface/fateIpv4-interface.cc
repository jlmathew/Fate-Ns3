/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 james mathewson 
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

#include "fateIpv4-interface.h"
//#include "fate-l3-protocol.h"
#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/pointer.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FateIpv4Interface");

NS_OBJECT_ENSURE_REGISTERED (FateIpv4Interface);

TypeId 
FateIpv4Interface::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FateIpv4Interface")
    .SetParent<Object> ()
    .SetGroupName ("Fate")
  ;
  return tid;
}

/** 
 * By default, Fate interface are created in the "down" state
 *  with no IP addresses.  Before becoming useable, the user must 
 * invoke SetUp on them once an Fate address and mask have been set.
 */
FateIpv4Interface::FateIpv4Interface () 
  : m_ifup (false),
    m_forwarding (true),
    m_node (0), 
    m_device (0)
{
  NS_LOG_FUNCTION (this);
}

FateIpv4Interface::~FateIpv4Interface ()
{
  NS_LOG_FUNCTION (this);
}

void
FateIpv4Interface::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_device = 0;
  Object::DoDispose ();
}

void 
FateIpv4Interface::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
  DoSetup ();
}

void 
FateIpv4Interface::SetDevice (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  m_device = device;
  DoSetup ();
}


void
FateIpv4Interface::DoSetup (void)
{
  NS_LOG_FUNCTION (this);
  if (m_node == 0 || m_device == 0)
    {
      return;
    }
}

Ptr<NetDevice>
FateIpv4Interface::GetDevice (void) const
{
  NS_LOG_FUNCTION (this);
  return m_device;
}

/*void
FateIpv4Interface::SetMetric (uint16_t metric)
{
  NS_LOG_FUNCTION (this << metric);
  m_metric = metric;
}

uint16_t
FateIpv4Interface::GetMetric (void) const
{
  NS_LOG_FUNCTION (this);
  return m_metric;
}*/


/**
 * These are IP interface states and may be distinct from 
 * NetDevice states, such as found in real implementations
 * (where the device may be down but IP interface state is still up).
 */
bool 
FateIpv4Interface::IsUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifup;
}

bool 
FateIpv4Interface::IsDown (void) const
{
  NS_LOG_FUNCTION (this);
  return !m_ifup;
}

void 
FateIpv4Interface::SetUp (void)
{
  NS_LOG_FUNCTION (this);
  m_ifup = true;
}

void 
FateIpv4Interface::SetDown (void)
{
  NS_LOG_FUNCTION (this);
  m_ifup = false;
}

bool 
FateIpv4Interface::IsForwarding (void) const
{
  NS_LOG_FUNCTION (this);
  return m_forwarding;
}

void 
FateIpv4Interface::SetForwarding (bool val)
{
  NS_LOG_FUNCTION (this << val);
  m_forwarding = val;
}


void
FateIpv4Interface::Send (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << *p );
std::cout << "PACKET SEEN!\n"; 
  // is this packet aimed at a local interface ?
  /* for (FateIpv4InterfaceAddressListCI i = m_ifaddrs.begin (); i != m_ifaddrs.end (); ++i)
    {
      if (dest == (*i).GetLocal ())
        {
          p->AddHeader (hdr);
          m_tc->Receive (m_device, p, FateL3Protocol::PROT_NUMBER,
                         m_device->GetBroadcast (),
                         m_device->GetBroadcast (),
                         NetDevice::PACKET_HOST);
          return;
        }
    } */
}

//static variable, identifying which port to reserve for FATE
uint16_t FateIpv4Interface::FatePort=65000;

uint32_t
FateIpv4Interface::GetNAddresses (void) const
{
  NS_LOG_FUNCTION (this);
  return 0; //m_ifaddrs.size ();
}

bool
FateIpv4Interface::FatePktToIpv4(const PktType &fatePkt2, Ptr<Packet> &p)
{

    bool ret=true;
    PktType fatePkt(fatePkt2);
    Ipv4Header ipHeader;
    UdpHeader udpHeader;
    TcpHeader tcpHeader;

uint64_t l4Proto=0;
uint64_t l3Proto=0;

    Ipv4Address toAddr, fromAddr;
uint64_t ttl=99;
uint64_t tos=0;
uint64_t id=0;
uint64_t ecn=0;
uint64_t dscp=0;
uint64_t htmlEncode=0;

std::cout << "fate to pkt:" << fatePkt2 << fatePkt2.GetName().GetAttributeCount() << "\n\n"; 

    bool isL3 = fatePkt.GetUnsignedNamedAttribute("L3Proto", l3Proto, true);
    if (isL3) {
     if (l3Proto != 0x800) { return false; }
    fatePkt.GetUnsignedNamedAttribute("Tos", tos, true);
    fatePkt.GetUnsignedNamedAttribute("TTL", ttl, true);
    fatePkt.GetUnsignedNamedAttribute("Id", id, true);
    fatePkt.GetUnsignedNamedAttribute("Ecn", ecn, true);
    fatePkt.GetUnsignedNamedAttribute("Dscp", dscp, true);
    fatePkt.GetPrintedNamedAttribute("Ipv4Src", toAddr, true); 
    fatePkt.GetPrintedNamedAttribute("Ipv4Dst", fromAddr, true); 

    ipHeader.SetSource(toAddr);
    ipHeader.SetDestination(fromAddr);
    ipHeader.SetTtl(ttl);
//bool crcOk = ipHeader.IsChecksumOk();
ipHeader.SetTos(tos);
ipHeader.SetIdentification(id);
ipHeader.SetEcn((Ipv4Header::EcnType) ecn);
ipHeader.SetDscp((Ipv4Header::DscpType) dscp);
    
    bool isL4 = fatePkt.GetUnsignedNamedAttribute("L4Proto", l4Proto, true); 
    if (!isL4) {
       l4Proto = 0;
    }
ipHeader.SetProtocol(l4Proto);
    } else { 
      l3Proto = 0;
    }

    if (l4Proto == 17) { //only work on UDP
      uint64_t srcPort=0;
      uint64_t dstPort=0;
      //uint16_t udpChecksum;

      fatePkt.GetUnsignedNamedAttribute("SrcPort", srcPort, true);
      fatePkt.GetUnsignedNamedAttribute("DstPort", dstPort, true);
      udpHeader.SetSourcePort(srcPort);
      udpHeader.SetDestinationPort(dstPort);
      } else if (l4Proto == 6) {  //FIXME add more tcp fields
      uint64_t srcPort=0;
      uint64_t dstPort=0;
      //uint16_t udpChecksum;

      fatePkt.GetUnsignedNamedAttribute("SrcPort", srcPort, true);
      fatePkt.GetUnsignedNamedAttribute("DstPort", dstPort, true);
      tcpHeader.SetSourcePort(srcPort);
      tcpHeader.SetDestinationPort(dstPort);
      //Need TCP sequence #, ack #, data offset, rsv, window, checksum, urg/ack/psh/rst/syn/fin, 
     // urgent pointer, options, padding.
      ret = false; //turn to true when the above options are implemented.

   }
   std::string html;
    fatePkt.GetUnsignedNamedAttribute("HTML_ENC", htmlEncode, true);

   if (!htmlEncode) {
      std::vector<uint8_t> fateData;
     fatePkt.ClearTempData();
     fatePkt.Serialize(fateData);
     uint8_t *data = new uint8_t[fateData.size()];
     for(unsigned i = 0; i < fateData.size(); i++) {
      data[i] = fateData[i];
     }
     p = Create<Packet>(data, fateData.size());
     delete[] data;
   } else { //add html
//get html message
    fatePkt.GetNamedAttribute("HTML", html, true);
//update html with FATE comment
//add to packet, after </html>  
    auto end=html.rfind("</html>"); 
    
      std::vector<uint8_t> fateData;
     fatePkt.ClearTempData();
     fatePkt.Serialize(fateData);
     std::stringstream out;
     out << fatePkt;
     std::string fateXml="<!--"+out.str()+"-->";
     //insert inside HTML info
     std::string newHtml=html.substr(0,end)+fateXml+html.substr(end,html.length()-end);
     p = Create<Packet>((uint8_t *) newHtml.c_str(), newHtml.size());
       

   }
   // std::string st ="this is a test to see if this works";/
   //Ptr<Packet> p = Create<Packet>((const uint8_t *) st.c_str(), st.length());
   //std::cout << "fateToPkt size is:" << p->GetSize() << std::endl;   
   if (l4Proto == 17) {
      p->AddHeader(udpHeader);
   } else if (l4Proto == 6) {
      p->AddHeader(tcpHeader);
   } else { ret = false; }

   ipHeader.SetPayloadSize(p->GetSize());
   if (l3Proto == 0x800) {
      p->AddHeader(ipHeader);
   } else {
      ret = false;
   }

   return ret; //->Copy();
}



bool
FateIpv4Interface::Ipv4ToFatePkt(Ptr<const Packet> p, uint16_t protocol, PktType &newPkt)  
{
  uint64_t htmlEncode=false; 
  //NOTE: while it would be more flexible to do this in stages, Fate packets are only valid TCP/UDP packets,
  //not ICMP or other (yet). 
  newPkt.ClearTempData();
  newPkt.ClearData();

  Ptr<Packet> packet = p->Copy ();
  bool valid = false;
//std::cout << " original pkt:";
//packet->Print(std::cout);
//std::cout << "\n";

  Ipv4Header ipHeader;
  UdpHeader udpHeader;
  TcpHeader tcpHeader;
  Ipv4Address toAddr, fromAddr;
uint8_t ttl=0;
bool crcOk=false;
uint8_t tos=0;
uint16_t id=0;
Ipv4Header::EcnType ecn=Ipv4Header::ECN_NotECT;
Ipv4Header::DscpType dscp=Ipv4Header::DscpDefault;
uint16_t ipProto = 0;

     uint64_t srcPort=0;
     uint64_t dstPort=0;

 //need to make this automated and recursive, in case of enscapusulation
 if (protocol==0x800) {
     packet->RemoveHeader(ipHeader);
     ipProto = ipHeader.GetProtocol();
   fromAddr = ipHeader.GetSource();
   toAddr = ipHeader.GetDestination();
 ttl = ipHeader.GetTtl();
 crcOk = ipHeader.IsChecksumOk();
 tos = ipHeader.GetTos();
 id = ipHeader.GetIdentification();
 ecn = ipHeader.GetEcn();
 dscp = ipHeader.GetDscp(); 

if (ipProto == 17) { //only work on UDP
      packet->RemoveHeader(udpHeader);

      srcPort = udpHeader.GetSourcePort();
      dstPort = udpHeader.GetDestinationPort();
      //std::cout << "udp ports:" << srcPort << "->" << dstPort << std::endl;
      valid = true;
     }
   else if (ipProto == 6 ) {
      packet->RemoveHeader(tcpHeader);
      srcPort = tcpHeader.GetSourcePort();
      dstPort = tcpHeader.GetDestinationPort();
      valid = true;
      //Need TCP sequence #, ack #, data offset, rsv, window, checksum, urg/ack/psh/rst/syn/fin, 
     // urgent pointer, options, padding.
   } else { 
     return false;
   }
    //must match the FATE port to differentiate it between regular packets and fate packets
    //also, check if its embedded HTTP/HTML fate (assume port 80 for now)
    if ((srcPort == GetFateUniqPort() ) && (dstPort == GetFateUniqPort())) {
       //if not a FATE supplemental packet, check if its HTTP
       if ((srcPort != 80) && (dstPort !=80)) {
         return false;
       }
       //it uses an HTTP port, check if there is an embedded fate packet
         uint32_t size = packet->GetSize();
         std::string xmlFate;
         xmlFate.reserve(size+1);
         uint8_t *data = new uint8_t[size];
        packet->CopyData(data, size);
        for(unsigned int i=0; i<size; i++) {
            xmlFate[i]=data[i];
         }
         xmlFate[size]='\0';
         delete[] data;
         //look for <!--<FATEPKT  to </FATEPKT>-->
         std::size_t first=xmlFate.find("<!--<FATEPKT");
         std::size_t last;
         if (first != std::string::npos) {
           last=xmlFate.rfind("</FATEPKT>-->");
           if (last == std::string::npos) {
              return false; //no end syntax
           }
         } else { return false;}   //no fate xml in HTML
         //embedded FATE inside HTML!   Now extract it
         std::stringstream(xmlFate.substr(first+4,last-3-first+4)) >> newPkt;
         htmlEncode=true;
         //remove html-fate comment
         std::string html=xmlFate.substr(0,first)+xmlFate.substr(last,xmlFate.length()-last);
         //add html to packet
         newPkt.SetNamedAttribute("HTML", html, true);
         //so we can recreate the packet as IP-extended (w/fate) or HTML enc with FATE
         newPkt.SetUnsignedNamedAttribute("HTML_ENC", htmlEncode, true);
         //we should extract packet type (data/interest) and NAME from html file TODO jlm
         //DATA pkt unless POST/GET inquiry TODO
         //use filename as pkt name
    }
  } else if (protocol == 0) {
    uint32_t size = packet->GetSize();
    uint8_t *data = new uint8_t[size];
    packet->CopyData(data, size);
    std::vector<uint8_t> dataVect;
    dataVect.reserve(size);
    for(unsigned int i=0; i<size; i++) {
      dataVect.push_back(data[i]);
    }
    delete[] data;
    newPkt.Deserialize(dataVect);
    valid = true;
  } else {
std::cout << "invalid non 0x800 proto :" << newPkt << "\n";

return false;
 } 

  if(!htmlEncode) {
    uint32_t size = packet->GetSize();
    uint8_t *data = new uint8_t[size];
    packet->CopyData(data, size);
    std::vector<uint8_t> dataVect;
    dataVect.reserve(size);
    for(unsigned int i=0; i<size; i++) {
      dataVect.push_back(data[i]);
    }
    delete[] data;
    newPkt.Deserialize(dataVect);
  }
  if (protocol == 0x800) {
    newPkt.SetPrintedNamedAttribute("Ipv4Src", fromAddr, true); 
    newPkt.SetPrintedNamedAttribute("Ipv4Dst", toAddr, true); 
    newPkt.SetUnsignedNamedAttribute("Tos", tos, true);
    newPkt.SetUnsignedNamedAttribute("TTL", ttl, true);
    newPkt.SetUnsignedNamedAttribute("Id", id, true);
    newPkt.SetUnsignedNamedAttribute("L3Proto", protocol, true);
    newPkt.SetUnsignedNamedAttribute("Ecn", ecn, true);
    newPkt.SetUnsignedNamedAttribute("Dscp", dscp, true);
    newPkt.SetUnsignedNamedAttribute("CRC_OK", crcOk, true);
    newPkt.SetUnsignedNamedAttribute("L4Proto", ipProto, true); 
  }
  if ((ipProto == 6) || (ipProto == 17)) { 
    newPkt.SetUnsignedNamedAttribute("SrcPort", srcPort, true);
    newPkt.SetUnsignedNamedAttribute("DstPort", dstPort, true);
  } 
std::cout << "PktToFate:" << newPkt << newPkt.GetName().GetAttributeCount() << std::endl;
  return valid;
}


/*bool
FateIpv4Interface::AddAddress (FateIpv4InterfaceAddress addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_ifaddrs.push_back (addr);
  return true;
}

FateIpv4InterfaceAddress
FateIpv4Interface::GetAddress (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  if (index < m_ifaddrs.size ())
    {
      uint32_t tmp = 0;
      for (FateIpv4InterfaceAddressListCI i = m_ifaddrs.begin (); i!= m_ifaddrs.end (); i++)
        {
          if (tmp  == index)
            {
              return *i;
            }
          ++tmp;
        }
    }
  else
    {
      NS_FATAL_ERROR ("index " << index << " out of bounds");  
    }
  FateIpv4InterfaceAddress addr;
  return (addr);  // quiet compiler
}

FateIpv4InterfaceAddress
FateIpv4Interface::RemoveAddress (uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  if (index >= m_ifaddrs.size ())
    {
      NS_FATAL_ERROR ("Bug in FateIpv4Interface::RemoveAddress");
    }
  FateIpv4InterfaceAddressListI i = m_ifaddrs.begin ();
  uint32_t tmp = 0;
  while (i != m_ifaddrs.end ())
    {
      if (tmp  == index)
        {
          FateIpv4InterfaceAddress addr = *i;
          m_ifaddrs.erase (i);
          return addr;
        }
      ++tmp;
      ++i;
    }
  NS_FATAL_ERROR ("Address " << index << " not found");
  FateIpv4InterfaceAddress addr;
  return (addr);  // quiet compiler
}

FateIpv4InterfaceAddress
FateIpv4Interface::RemoveAddress(FateAddress address)
{
  NS_LOG_FUNCTION(this << address);

  if (address == address.GetLoopback())
    {
      NS_LOG_WARN ("Cannot remove loopback address.");
      return FateIpv4InterfaceAddress();
    }

  for(FateIpv4InterfaceAddressListI it = m_ifaddrs.begin(); it != m_ifaddrs.end(); it++)
    {
      if((*it).GetLocal() == address)
        {
          FateIpv4InterfaceAddress ifAddr = *it;
          m_ifaddrs.erase(it);
          return ifAddr;
        }
    }
  return FateIpv4InterfaceAddress();
}*/

} // namespace ns3

