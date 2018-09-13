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

#include "udp-echo-helper.h"
#include "udp-fate-helper.h"
#include "ns3/udp-fate-server.h"
#include "ns3/udp-fate-client-cbr.h"
#include "ns3/udp-fate-client-video.h"
#include "ns3/udp-fate-client-video2.h"
#include "ns3/udp-fate-video-server.h"
#include "ns3/udp-fate-client-zipf.h"
#include "ns3/udp-fate-client-file.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

UdpFateVideoServerHelper::UdpFateVideoServerHelper (uint16_t port)
{
  m_factory.SetTypeId (UdpFateVideoServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void 
UdpFateVideoServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpFateVideoServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateVideoServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateVideoServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpFateVideoServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpFateVideoServer> ();
  node->AddApplication (app);

  return app;
}

void
UdpFateVideoServerHelper::SetPartialMatch(Ptr<Application> app, const std::list< std::pair< std::string, std::string> > &match)
{
  app->GetObject<UdpFateVideoServer>()->SetPartialMatch(match);
}

void
UdpFateVideoServerHelper::DumpStats(Ptr<Node> node, std::ostream &os )const 
{
       for (uint32_t j = 0; j < node->GetNApplications (); j++)
       {
          Ptr<UdpFateVideoServer> app = DynamicCast<UdpFateVideoServer> (node->GetApplication (j));
          if (app) {
            app->GetObject<UdpFateVideoServer>()->PrintStats(os);
          }
       }
       
}






UdpFateServerHelper::UdpFateServerHelper (uint16_t port)
{
  m_factory.SetTypeId (UdpFateServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void 
UdpFateServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpFateServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpFateServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpFateServer> ();
  node->AddApplication (app);

  return app;
}

void
UdpFateServerHelper::SetPartialMatch(Ptr<Application> app, const std::list< std::pair< std::string, std::string> > &match)
{
  app->GetObject<UdpFateServer>()->SetPartialMatch(match);
}

void
UdpFateServerHelper::DumpStats(Ptr<Node> node, std::ostream &os )const 
{
       for (uint32_t j = 0; j < node->GetNApplications (); j++)
       {
          Ptr<UdpFateServer> app = DynamicCast<UdpFateServer> (node->GetApplication (j));
          if (app) {
            app->GetObject<UdpFateServer>()->PrintStats(os);
          }
       }
       
}

UdpFateFileClientHelper::UdpFateFileClientHelper (ipPort(*getAddrPort)(const std::string &name))
{
  uninit=true;
}
UdpFateFileClientHelper::UdpFateFileClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (UdpFateFileClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  uninit=false;
}

UdpFateFileClientHelper::UdpFateFileClientHelper (Address address)
{
  m_factory.SetTypeId (UdpFateFileClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  uninit=false;
}

void 
UdpFateFileClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}
void
UdpFateFileClientHelper::DumpStats(Ptr<Node> node, std::ostream &os )const 
{
         for (uint32_t j = 0; j < node->GetNApplications (); j++)
       {
          Ptr<UdpFateFileClient> app = DynamicCast<UdpFateFileClient> (node->GetApplication (j));
          if (app) {
            app->GetObject<UdpFateFileClient>()->PrintStats(os);
          }

       }
}
void
UdpFateFileClientHelper::SetPktPayload (Ptr<Application> app, const std::string &xml)
{
  app->GetObject<UdpFateFileClient>()->SetPktPayload (xml);
}
void
UdpFateFileClientHelper::SetPktPayload (Ptr<Application> app,const PktType &payload )
{
  app->GetObject<UdpFateFileClient>()->SetPktPayload (payload);
}
void
UdpFateFileClientHelper::SetPktPayload (Ptr<Application> app,const IcnName<std::string> &name )
{
  app->GetObject<UdpFateFileClient>()->SetPktPayload (name);
}

void
UdpFateFileClientHelper::SetPktPayload (Ptr<Application> app, uint8_t *fill, uint32_t dataLength)
{
  app->GetObject<UdpFateFileClient>()->SetPktPayload (fill, dataLength);
}
void
UdpFateFileClientHelper::SetTimestamp (Ptr<Application> app, bool timestamp)
{
  app->GetObject<UdpFateFileClient>()->SetTimestamp(timestamp);
}

ApplicationContainer
UdpFateFileClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateFileClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateFileClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpFateFileClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpFateFileClient> ();
  node->AddApplication (app);

  return app;
}


UdpFateZipfClientHelper::UdpFateZipfClientHelper ()
{
  uninit=true;
}
UdpFateZipfClientHelper::UdpFateZipfClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (UdpFateZipfClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  uninit=false;
}

UdpFateZipfClientHelper::UdpFateZipfClientHelper (Address address)
{
  m_factory.SetTypeId (UdpFateZipfClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  uninit=false;
}

void 
UdpFateZipfClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}
void
UdpFateZipfClientHelper::DumpStats(Ptr<Node> node, std::ostream &os )const 
{
         for (uint32_t j = 0; j < node->GetNApplications (); j++)
       {
          Ptr<UdpFateZipfClient> app = DynamicCast<UdpFateZipfClient> (node->GetApplication (j));
          if (app) {
            app->GetObject<UdpFateZipfClient>()->PrintStats(os);
          }
       }
}
void
UdpFateZipfClientHelper::SetPktPayload (Ptr<Node> node, unsigned int appNum, const PktType &payload )
{
     if (appNum >= node->GetNApplications()) { assert(0);}
     Ptr<UdpFateZipfClient> app = DynamicCast<UdpFateZipfClient> (node->GetApplication (appNum));
     if (app) {
         app->GetObject<UdpFateZipfClient>()->SetPktPayload (payload);
     } else {
       assert(0);
     }
}

void
UdpFateZipfClientHelper::SetPktPayload (Ptr<Application> app, const std::string &xml)
{
  app->GetObject<UdpFateZipfClient>()->SetPktPayload (xml);
}
void
UdpFateZipfClientHelper::SetPktPayload (Ptr<Application> app,const PktType &payload )
{
  app->GetObject<UdpFateZipfClient>()->SetPktPayload (payload);
}
void
UdpFateZipfClientHelper::SetPktPayload (Ptr<Application> app,const IcnName<std::string> &name )
{
  app->GetObject<UdpFateZipfClient>()->SetPktPayload (name);
}
void
UdpFateZipfClientHelper::SetTimestamp (Ptr<Application> app, bool timestamp)
{
  app->GetObject<UdpFateZipfClient>()->SetTimestamp(timestamp);
}
void
UdpFateZipfClientHelper::SetPktPayload (Ptr<Application> app, uint8_t *fill, uint32_t dataLength)
{
  app->GetObject<UdpFateZipfClient>()->SetPktPayload (fill, dataLength);
}

ApplicationContainer
UdpFateZipfClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateZipfClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateZipfClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpFateZipfClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpFateZipfClient> ();
  node->AddApplication (app);

  return app;
}

UdpFateCbrClientHelper::UdpFateCbrClientHelper ()
{
  uninit=true;
}
UdpFateCbrClientHelper::UdpFateCbrClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (UdpFateCbrClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  uninit=false;
}

UdpFateCbrClientHelper::UdpFateCbrClientHelper (Address address)
{
  m_factory.SetTypeId (UdpFateVideoClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  uninit=false;
}

void 
UdpFateCbrClientHelper::SetAttribute ( std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
UdpFateCbrClientHelper::SetTimestamp (Ptr<Application> app, bool timestamp)
{
  app->GetObject<UdpFateVideoClient>()->SetTimestamp(timestamp);
}
ApplicationContainer
UdpFateCbrClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateCbrClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateCbrClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}
void
UdpFateCbrClientHelper::DumpStats(Ptr<Node> node, std::ostream &os )const 
{
  for (uint32_t j = 0; j < node->GetNApplications (); j++)
       {
          Ptr<UdpFateVideoClient> app = DynamicCast<UdpFateVideoClient> (node->GetApplication (j));
          if (app) {
            app->GetObject<UdpFateVideoClient>()->PrintStats(os);
          }
       }
}

Ptr<Application>
UdpFateCbrClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpFateVideoClient> ();
  node->AddApplication (app);

  return app;
}





UdpFateVideoClientHelper::UdpFateVideoClientHelper ()
{
  uninit=true;
}
UdpFateVideoClientHelper::UdpFateVideoClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (UdpFateVideoClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  uninit=false;
}

UdpFateVideoClientHelper::UdpFateVideoClientHelper (Address address)
{
  m_factory.SetTypeId (UdpFateVideoClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  uninit=false;
}

void 
UdpFateVideoClientHelper::SetAttribute ( std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
UdpFateVideoClientHelper::SetTimestamp (Ptr<Application> app, bool timestamp)
{
  app->GetObject<UdpFateVideoClient>()->SetTimestamp(timestamp);
}
ApplicationContainer
UdpFateVideoClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateVideoClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateVideoClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}
void
UdpFateVideoClientHelper::DumpStats(Ptr<Node> node, std::ostream &os )const 
{
  for (uint32_t j = 0; j < node->GetNApplications (); j++)
       {
          Ptr<UdpFateVideoClient> app = DynamicCast<UdpFateVideoClient> (node->GetApplication (j));
          if (app) {
            app->GetObject<UdpFateVideoClient>()->PrintStats(os);
          }
       }
}

Ptr<Application>
UdpFateVideoClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpFateVideoClient> ();
  node->AddApplication (app);

  return app;
}




//////////////////////////
UdpFateVideoClientHelper2::UdpFateVideoClientHelper2 ()
{
  uninit=true;
}
UdpFateVideoClientHelper2::UdpFateVideoClientHelper2 (Address address, uint16_t port)
{
  m_factory.SetTypeId (UdpFateVideoClient2::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  uninit=false;
}

UdpFateVideoClientHelper2::UdpFateVideoClientHelper2 (Address address)
{
  m_factory.SetTypeId (UdpFateVideoClient2::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  uninit=false;
}

void 
UdpFateVideoClientHelper2::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}
void
UdpFateVideoClientHelper2::DumpStats(Ptr<Node> node, std::ostream &os )const 
{
         for (uint32_t j = 0; j < node->GetNApplications (); j++)
       {
          Ptr<UdpFateVideoClient2> app = DynamicCast<UdpFateVideoClient2> (node->GetApplication (j));
          if (app) {
            app->GetObject<UdpFateVideoClient2>()->PrintStats(os);
          }
       }
}
void
UdpFateVideoClientHelper2::SetPktPayload (Ptr<Node> node, unsigned int appNum, const PktType &payload )
{
     if (appNum >= node->GetNApplications()) { assert(0);}
     Ptr<UdpFateVideoClient2> app = DynamicCast<UdpFateVideoClient2> (node->GetApplication (appNum));
     if (app) {
         app->GetObject<UdpFateVideoClient2>()->SetPktPayload (payload);
     } else {
       assert(0);
     }
}

void
UdpFateVideoClientHelper2::SetPktPayload (Ptr<Application> app, const std::string &xml)
{
  app->GetObject<UdpFateVideoClient2>()->SetPktPayload (xml);
}
void
UdpFateVideoClientHelper2::SetPktPayload (Ptr<Application> app,const PktType &payload )
{
  app->GetObject<UdpFateVideoClient2>()->SetPktPayload (payload);
}
void
UdpFateVideoClientHelper2::SetPktPayload (Ptr<Application> app,const IcnName<std::string> &name )
{
  app->GetObject<UdpFateVideoClient2>()->SetPktPayload (name);
}
void
UdpFateVideoClientHelper2::SetTimestamp (Ptr<Application> app, bool timestamp)
{
  app->GetObject<UdpFateVideoClient2>()->SetTimestamp(timestamp);
}
void
UdpFateVideoClientHelper2::SetPktPayload (Ptr<Application> app, uint8_t *fill, uint32_t dataLength)
{
  app->GetObject<UdpFateVideoClient2>()->SetPktPayload (fill, dataLength);
}

ApplicationContainer
UdpFateVideoClientHelper2::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateVideoClientHelper2::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpFateVideoClientHelper2::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpFateVideoClientHelper2::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpFateVideoClient2> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
