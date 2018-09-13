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

#ifndef FATE_HELPER_H
#define FATE_HELPER_H

#include "ns3/fate.h"
#include "ns3/GlobalModulesNs3.h"
#include "ns3/IcnName.h"
#include <iostream>
#include <ostream>
#include <string>
#include "ns3/PacketTypeBase.h"
#include "ns3/node.h"
#include "ns3/global-fate.h"
#include "ns3/NodeManager.h"
#include "ns3/UtilityExternalModule.h"
#include "ns3/UtilityConfigXml.h"

namespace ns3 {
class FateHelper //: public any tracetypes
{
FateHelper();
virtual ~FateHelper();
void Reset(const NodeContainer &nc) const;

void SetConfigFile(const std::string &config="fateXmlConfigFiles/.xml") ;

void Install(Ptr<Node> node) const;
void Install(const std::string &nodeName) const;
void Install(const NodeContainer &nc) const;
void InstallAll() const;
private:
FateHelper(const FateHelper &);
FateHelper &operator = (const FateHelper &op);

std::string m_configName;
/* ... */

};
}

#endif /* FATE_HELPER_H */

