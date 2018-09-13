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
#ifndef GLOBAL_FATE_H
#define GLOBAL_FATE_H

#include <map>
#include <vector>
#include <iostream>
#include <ostream>
#include <string>
#include <algorithm>
#include "ns3/udp-fate-server.h"
#include "ns3/udp-fate-video-server.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "ns3/IcnName.h"
#include "ns3/PacketTypeBase.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/node.h"
#include "ns3/ipv4-nix-vector-helper.h"
//#include  "ns3/fate-global.h"

namespace ns3 {

typedef std::pair<Ipv4Address, uint16_t> ipPort_t;

void CreateDestAssociation(const NodeContainer &producers);
//bool isThisMyIpAddr(const std::string &myName,  Ipv4Address addr) ;
void CreateDnsAssociation();
 std::map<Ipv4Address, std::string> * GetDns();

 std::string
GetProdNodeName(const std::string &bestMatchName);

ipPort_t 
GetProdNodeIpv4(const std::string &bestMatchName);

ipPort_t 
GeDnsNameFromIpv4(const std::string &dnsName);
 
#define ICN_STAT_ROUTES 1

} // namespace ns3

#endif /* UDP_ECHO_CLIENT_H */
