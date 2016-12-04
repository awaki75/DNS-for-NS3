/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/dns-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("DnsExample");

// DNSクエリを送信する
void
SendDnsQuery (Ptr<Socket> socket)
{
  std::string qName = "www.example.co.jp";

  QuestionSectionHeader question;
  question.SetqName (qName);

  DNSHeader header;
  header.SetQRbit (1);
  header.AddQuestion (question);

  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (header);

  socket->Send (packet);

  NS_LOG_INFO ("SendDnsQuery:" << qName);
}

// DNSクエリを受信する
void
RecvDnsQuery (Ptr<Socket> socket)
{
  Ptr<Packet> packet = socket->Recv ();

  DNSHeader header;
  packet->RemoveHeader (header);

  std::list<ResourceRecordHeader> answers = header.GetAnswerList ();

  std::string qName = answers.begin ()->GetName ();
  std::string rData = answers.begin ()->GetRData ();

  NS_LOG_INFO ("RecvDnsQuery:" << qName << ":" << rData);
}

// ノードからIPアドレスを取得する
Ipv4Address
getAddress (Ptr<Node> node)
{
  return node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
}

// ノードからIPアドレスを取得する
std::string
getAddressString (Ptr<Node> node)
{
  std::stringstream stream;
  stream << getAddress (node);
  return stream.str ();
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // LogComponentEnable ("BindServer", LOG_ALL);
  LogComponentEnable ("DnsExample", LOG_ALL);

  // ノードをつくる
  Ptr<Node> client = CreateObject<Node> ();
  Ptr<Node> server = CreateObject<Node> ();
  Ptr<Node> router = CreateObject<Node> ();
  Ptr<Node> localServer = CreateObject<Node> ();
  Ptr<Node> rootServer = CreateObject<Node> ();
  Ptr<Node> tldServer = CreateObject<Node> ();
  Ptr<Node> ispServer = CreateObject<Node> ();
  Ptr<Node> authServer = CreateObject<Node> ();

  // ノードに名前をつける (PyVizで表示される)
  Names::Add ("Client", client);
  Names::Add ("Server", server);
  Names::Add ("Router", router);
  Names::Add ("Local Name Server", localServer);
  Names::Add ("Root Name Server", rootServer);
  Names::Add ("Top Level Domain Server", tldServer);
  Names::Add ("ISP's DNS Server", ispServer);
  Names::Add ("Authoritative Name Server", authServer);

  // プロトコルスタックを割り当てる
  {
    NodeContainer nodes;
    nodes.Add (client);
    nodes.Add (server);
    nodes.Add (router);
    nodes.Add (localServer);
    nodes.Add (rootServer);
    nodes.Add (tldServer);
    nodes.Add (ispServer);
    nodes.Add (authServer);
    InternetStackHelper stack;
    stack.Install (nodes);
  }

  // ノード同士を繋いでIPアドレスを割り当てる
  {
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

    {
      NodeContainer nodes (client, router);
      Ipv4AddressHelper address;
      address.SetBase ("10.1.1.0", "255.255.255.0");
      address.Assign (p2p.Install (nodes));
    }

    {
      NodeContainer nodes (server, router);
      Ipv4AddressHelper address;
      address.SetBase ("10.1.2.0", "255.255.255.0");
      address.Assign (p2p.Install (nodes));
    }

    {
      NodeContainer nodes (localServer, router);
      Ipv4AddressHelper address;
      address.SetBase ("10.1.3.0", "255.255.255.0");
      address.Assign (p2p.Install (nodes));
    }

    {
      NodeContainer nodes (rootServer, router);
      Ipv4AddressHelper address;
      address.SetBase ("10.1.4.0", "255.255.255.0");
      address.Assign (p2p.Install (nodes));
    }

    {
      NodeContainer nodes (tldServer, router);
      Ipv4AddressHelper address;
      address.SetBase ("10.1.5.0", "255.255.255.0");
      address.Assign (p2p.Install (nodes));
    }

    {
      NodeContainer nodes (ispServer, router);
      Ipv4AddressHelper address;
      address.SetBase ("10.1.6.0", "255.255.255.0");
      address.Assign (p2p.Install (nodes));
    }

    {
      NodeContainer nodes (authServer, router);
      Ipv4AddressHelper address;
      address.SetBase ("10.1.7.0", "255.255.255.0");
      address.Assign (p2p.Install (nodes));
    }
  }

  // ルーティングテーブルをつくる
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Local Name Serverを設定する
  {
    BindServerHelper bindServer (BindServer::LOCAL_SERVER);
    bindServer.SetAttribute ("SetRecursiveSupport", EnumValue (BindServer::RA_AVAILABLE));
    bindServer.SetAttribute ("SetServerAddress", Ipv4AddressValue (getAddress (localServer)));
    bindServer.SetAttribute ("RootServerAddress", Ipv4AddressValue (getAddress (rootServer)));
    ApplicationContainer apps = bindServer.Install (localServer);
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (60.0));
  }

  // Root Name Serverを設定する
  {
    BindServerHelper bindServer (BindServer::ROOT_SERVER);
    bindServer.SetAttribute ("SetServerAddress", Ipv4AddressValue (getAddress (rootServer)));
    ApplicationContainer apps = bindServer.Install (rootServer);
    bindServer.AddNSRecord (apps.Get (0),
                            ".jp",
                            86400,
                            1,
                            1,
                            getAddressString (tldServer));
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (60.0));
  }

  // Top Level Domain Serverを設定する
  {
    BindServerHelper bindServer (BindServer::TLD_SERVER);
    bindServer.SetAttribute ("SetServerAddress", Ipv4AddressValue (getAddress (tldServer)));
    ApplicationContainer apps = bindServer.Install (tldServer);
    bindServer.AddNSRecord (apps.Get (0),
                            ".co.jp",
                            86400,
                            1,
                            1,
                            getAddressString (ispServer));
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (60.0));
  }

  // ISP's DNS Serverを設定する
  {
    BindServerHelper bindServer (BindServer::ISP_SERVER);
    bindServer.SetAttribute ("SetServerAddress", Ipv4AddressValue (getAddress (ispServer)));
    ApplicationContainer apps = bindServer.Install (ispServer);
    bindServer.AddNSRecord (apps.Get (0),
                            ".example.co.jp",
                            86400,
                            1,
                            1,
                            getAddressString (authServer));
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (60.0));
  }

  // Authoritative Name Serverを設定する
  {
    BindServerHelper bindServer (BindServer::AUTH_SERVER);
    bindServer.SetAttribute ("SetServerAddress", Ipv4AddressValue (getAddress (authServer)));
    ApplicationContainer apps = bindServer.Install (authServer);
    bindServer.AddNSRecord (apps.Get (0),
                            "www.example.co.jp",
                            86400,
                            1,
                            1,
                            getAddressString (server));
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (60.0));
  }

  // クライアントからLocal Name Serverへのソケットをつくる
  // ソケットで何か受信したらRecvDnsQueryを実行する
  // シミュレート開始から5秒後にSendDnsQueryを実行する
  {
    Ptr<Socket> socket = Socket::CreateSocket (client, TypeId::LookupByName ("ns3::UdpSocketFactory"));
    socket->SetAllowBroadcast (true);
    socket->Bind (InetSocketAddress (getAddress (client), DNS_PORT));
    socket->Connect (InetSocketAddress (getAddress (localServer), DNS_PORT));
    socket->SetRecvCallback (MakeCallback (RecvDnsQuery));
    Simulator::Schedule (Seconds (5.0), &SendDnsQuery, socket);
  }

  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
