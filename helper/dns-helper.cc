/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/dns.h"
#include "ns3/names.h"
#include "dns-helper.h"
#include "ns3/bind-server.h"
namespace ns3 {

	BindServerHelper::BindServerHelper (BindServer::ServerType serverType)
	{
		m_factory.SetTypeId (BindServer::GetTypeId ());
  	SetAttribute ("NameServerType", EnumValue (serverType));
	}
	
	void
	BindServerHelper::SetAttribute (std::string name, const AttributeValue &value)
	{
 		m_factory.Set (name, value);
	}

	void 
	BindServerHelper::AddNSRecord (Ptr<Application> app, std::string name, uint32_t TTL, uint16_t nsClass, uint16_t type, std::string rData)
	{
		app->GetObject<BindServer> ()->AddZone (name, TTL, nsClass, type, rData);	
	}

	ApplicationContainer
	BindServerHelper::Install (Ptr<Node> node) const
	{
  	return ApplicationContainer (InstallPriv (node));
	}

	ApplicationContainer
	BindServerHelper::Install (std::string nodeName) const
	{
  	Ptr<Node> node = Names::Find<Node> (nodeName);
  	return ApplicationContainer (InstallPriv (node));
	}

	ApplicationContainer
	BindServerHelper::Install (NodeContainer c) const
	{
  	ApplicationContainer apps;
  	for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  	return apps;
	}

	Ptr<Application>
	BindServerHelper::InstallPriv (Ptr<Node> node) const
	{
  	Ptr<Application> app = m_factory.Create<BindServer> ();
  	node->AddApplication (app);

  	return app;
	}
}

