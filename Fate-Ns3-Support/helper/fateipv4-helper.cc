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


#include "fateipv4-helper.h"
#include "ns3/names.h"

namespace ns3 {

/* ... */
FateIpv4Helper::FateIpv4Helper()
{
   m_stats=nullptr;
   m_global=nullptr;
   m_log=nullptr;
   Initialize();
}

FateIpv4Helper::~FateIpv4Helper() 
{

}

FateIpv4Helper::FateIpv4Helper(const FateIpv4Helper &){
}

FateIpv4Helper &
FateIpv4Helper::operator = (const FateIpv4Helper &op)
{
  return *this;
}
void FateIpv4Helper::Reset(const NodeContainer &nc) const
{
    
}
void FateIpv4Helper::SetLog(GlobalModuleLog *log)
{
   m_log = log;
}
void FateIpv4Helper::SetConfigFile(const std::string &config) 
{
   m_configName = config;
}
void FateIpv4Helper::Initialize()
{
}
void FateIpv4Helper::SetStats(NodeStats *stats)
{
    m_stats = stats;
}
void FateIpv4Helper::SetGlobalModule(GlobalModule *global)
{
    m_global = global;
}
void FateIpv4Helper::Install(Ptr<Node> node) const
{
    bool init = m_global && m_stats && m_configName.size() && m_log && m_global->GetGlobalTimer() ;
    NS_ASSERT(init);

   //install fateipv4 stack pointer
    Ptr<FateIpv4L3> proto = CreateObject<FateIpv4L3>();
      UtilityConfigXml config;
      config.FirstNodeFileConfig(m_configName);
    NodeManager *nm = new NodeManager(config);
    proto->InsertFateNode(nm); 

         std::string name="Node";
         uint32_t nodeNum=node->GetId();
         std::ostringstream os;
         os << nodeNum;
         name.append(os.str()); 


      UtilityExternalModule *mod = new UtilityExternalModule;
         DeviceModule *dev = new DeviceModule;
         dev->SetNodeName(name);
         dev->SetNodeStats(m_stats);
         mod->SetDevice(dev);
         mod->SetGlobalModule(m_global);
    proto->InstantiateFate(mod);

    //attach to node
    node->AggregateObject(proto);
    //register protocol
    if (1 < node->GetNDevices()) {
        NS_ASSERT("node needs a device");
    }
    Ptr<NetDevice> device = node->GetDevice(0);
    node->RegisterFateProtocolHandler(MakeCallback(&FateIpv4L3::Receive, proto)); //register non promis?
    

}
void FateIpv4Helper::Install(const std::string &nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  Install (node);
}
void FateIpv4Helper::Install(const NodeContainer &nc) const
{
  for (NodeContainer::Iterator i = nc.Begin (); i != nc.End (); ++i)
    {
      Install (*i);
    }
}
void FateIpv4Helper::InstallAll() const {
  Install (NodeContainer::GetGlobal ());
}

}
