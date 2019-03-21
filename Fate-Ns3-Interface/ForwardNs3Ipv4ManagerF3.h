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
//support file transfers
#ifndef FORWARDNS3IPV4MANAGERBASEF3_H_
#define FORWARDNS3IPV4MANAGERBASEF3_H_

#include "../ModuleManager.h"
#include "../BaseStorage.h"
#include "../StoreManager.h"
//#include "UtilityFunctionGenerator.h"
#include "../RangeData.h"
#include "../PacketTypeBase.h"
#include "../NodeManager.h"
#include "ns3/node.h"
#include "ns3/ipv4-address.h"


class ForwardNs3Ipv4ManagerF3 : public ModuleManager
{
  public:
  ForwardNs3Ipv4ManagerF3();

  ForwardNs3Ipv4ManagerF3(ConfigWrapper &config);
  virtual
  ~ForwardNs3Ipv4ManagerF3();

  
  virtual bool OnInit (UtilityExternalModule *);

  //virtual const std::string &
  //Name() const;
   virtual void
  Config(ConfigWrapper &config); 
  //delete Configs by name or number
virtual void
  OnPktIngress(PktType &pkt);
  static const dataNameType_t &
   IdName(void) { static const dataNameType_t idName("ForwardNs3Ipv4ManagerF3"); return idName; }

  //store connectivity
 void SetStore(StoreManager *);
  
 bool
isThisMyIpAddr(const std::string &myName,  ns3::Ipv4Address addr); 


  //default actions
  private:
 void
IpUdpTcpReturn(PktType &pkt);
bool ForwardChainPkt(PktType &pkt);

  TypicalCacheStore *m_cacheStore;
  std::string m_cacheStoreName;
  uint64_t m_statsMiss;
  uint64_t m_statsHit;
  uint64_t m_statsHitExpired;

  double m_dropValue;
      std::map<ns3::Ipv4Address, std::string> * m_dns;
    double PopNextDst(PktType &pkt, std::string &nextAddr); 

};

#endif
