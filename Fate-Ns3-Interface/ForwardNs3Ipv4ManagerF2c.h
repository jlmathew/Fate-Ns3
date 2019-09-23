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

#ifndef FORWARDNS3IPV4MANAGERBASEF2C_H_
#define FORWARDNS3IPV4MANAGERBASEF2C_H_

#include "../ModuleManager.h"
#include "../BaseStorage.h"
#include "../StoreManager.h"
//#include "UtilityFunctionGenerator.h"
#include "../RangeData.h"
#include "../PacketTypeBase.h"
#include "../NodeManager.h"
#include "ForwardNs3Ipv4Manager.h"
#include "ns3/node.h"
#include "ns3/ipv4-address.h"
//#include "ns3/global-fate.h"
//#include "../fate-global.h"

//extern std::map<ns3::Ipv4Address, std::string > dnsIpToName;


class ForwardNs3Ipv4ManagerF2C : public ForwardNs3Ipv4Manager 
{
  public:
  ForwardNs3Ipv4ManagerF2C();

  ForwardNs3Ipv4ManagerF2C(ConfigWrapper &config);
  virtual
  ~ForwardNs3Ipv4ManagerF2C();

  bool OnInit(UtilityExternalModule *);
  void OnPktIngress(PktType &pkt);
 // bool isThisMyIpAddr(const std::string &myName,  ns3::Ipv4Address addr) ;
  static const dataNameType_t &
   IdName(void) { static const dataNameType_t idName("ForwardNs3Ipv4ManagerF2c"); return idName; }
bool
isThisMyIpAddr(const std::string &myName,  ns3::Ipv4Address addr); 
private:
    std::map<ns3::Ipv4Address, std::string> * m_dns;
    double PopNextDst(PktType &pkt, std::string &nextAddr); 
};

#endif
