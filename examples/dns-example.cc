/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/dns-module.h"

#include "ns3/applications-module.h"
#include "ns3/eslr-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/pyviz.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("DNSimpleExample");

int
main (int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd;

  {
    PyViz v;
  }

  cmd.AddValue ("verbose", "Tell application to log if true", verbose);

  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create Routers.");

  Ptr<Node> sendai = CreateObject<Node> ();  // 1
  Names::Add ("SendaiRouter", sendai);
  Ptr<Node> tsukuba = CreateObject<Node> ();  // 2
  Names::Add ("TsukubaRouter", tsukuba);
  Ptr<Node> nezu = CreateObject<Node> ();  // 3
  Names::Add ("NezuRouter", nezu);
  Ptr<Node> kddiOtemachi = CreateObject<Node> ();  // 4
  Names::Add ("KDDIOtemachiRouter", kddiOtemachi);
  Ptr<Node> nttOtemachi = CreateObject<Node> ();  // 5
  Names::Add ("NTTOtemachiRouter", nttOtemachi);
  Ptr<Node> shinkawasaki = CreateObject<Node> ();  // 6
  Names::Add ("ShinKawasakiRouter", shinkawasaki);
  Ptr<Node> yagami = CreateObject<Node> ();  // 7
  Names::Add ("YagamiRouter", yagami);
  Ptr<Node> fujisawa = CreateObject<Node> ();  // 8
  Names::Add ("FujisawaRouter", fujisawa);
  Ptr<Node> nara = CreateObject<Node> ();  // 9
  Names::Add ("NaraRouter", nara);
  Ptr<Node> dojima = CreateObject<Node> ();  // 10
  Names::Add ("DojimaRouter", dojima);
  Ptr<Node> komatsu = CreateObject<Node> ();  // 11
  Names::Add ("KomatsuRouter", komatsu);
  Ptr<Node> sakyo = CreateObject<Node> ();  // 12
  Names::Add ("SakyoRouter", sakyo);
  Ptr<Node> hiroshima = CreateObject<Node> ();  // 13
  Names::Add ("HiroshimaRouter", hiroshima);
  Ptr<Node> kurashiki = CreateObject<Node> ();  // 14
  Names::Add ("KurashikiRouter", kurashiki);
  Ptr<Node> fukuoka = CreateObject<Node> ();  // 15
  Names::Add ("FukuokaRouter", fukuoka);

  NS_LOG_INFO ("Create Client Nodes.");

  Ptr<Node> c1 = CreateObject<Node> ();
  Names::Add ("Client1", c1);
  c1->SetNodeType (Node::CLIENT_NODE);

  NS_LOG_INFO ("Create DNS server structure.");

  Ptr<Node> localDNS = CreateObject<Node> ();
  Names::Add ("LocalDNS", localDNS);
  Ptr<Node> rootDNS = CreateObject<Node> ();
  Names::Add ("RootDNS", rootDNS);
  Ptr<Node> tldDNS = CreateObject<Node> ();
  Names::Add ("TLDDNS", tldDNS);
  Ptr<Node> ispDNS = CreateObject<Node> ();
  Names::Add ("ISPDNS", ispDNS);
  Ptr<Node> authDNS = CreateObject<Node> ();
  Names::Add ("AuthDNS", authDNS);

  NS_LOG_INFO ("Create Server Nodes");

  Ptr<Node> sServer1 = CreateObject<Node> ();
  Names::Add ("ContentS1", sServer1);
  sServer1->SetNodeType (Node::SERVER_NODE);

  NS_LOG_INFO ("Create links.");
  //client<-->Router
  NodeContainer net1 (c1, sendai);  // Sendai --> i1

  // between routers
  NodeContainer net2 (sendai, nezu);               // sendai --> i2, nezu --> i1
  NodeContainer net3 (nezu, kddiOtemachi);         // nezu --> i2, kddiOtemachi --> i1
  NodeContainer net4 (nezu, yagami);               // nezu --> i3, yagami --> i1
  NodeContainer net5 (nezu, dojima);               // nezu --> i4, dojima --> i1
  NodeContainer net6 (kddiOtemachi, nttOtemachi);  // kddiOtemachi --> i2, nttOtemachi --> i1
  NodeContainer net7 (yagami, shinkawasaki);       // yagami --> i2, shinkawasaki --> i1
  NodeContainer net8 (yagami, fujisawa);           // yagami --> i3, fujisawa --> i1
  NodeContainer net9 (nttOtemachi, fujisawa);      // nttOtemachi --> i2, fujisawa --> i2
  NodeContainer net10 (nttOtemachi, tsukuba);      // nttOtemachi --> i3, tsukuba --> i1
  NodeContainer net11 (nttOtemachi, komatsu);      // nttOtemachi --> i4, komatsu --> i1
  NodeContainer net12 (nttOtemachi, dojima);       // nttOtemachi --> i5, dojima --> i2
  NodeContainer net13 (fujisawa, nara);            // fujisawa --> i3, nara --> i1
  NodeContainer net14 (nara, sakyo);               // nara --> i2, sakyo --> i1
  NodeContainer net15 (nara, dojima);              // nara --> i3, dojima --> i3
  NodeContainer net16 (dojima, sakyo);             // dojima --> i4, sakyo --> i2
  NodeContainer net17 (dojima, hiroshima);         // dojima --> i5, hiroshima --> i1
  NodeContainer net18 (dojima, kurashiki);         // dojima --> i6, kurashiki --> i1
  NodeContainer net19 (dojima, fukuoka);           // dojima --> i7, fukuoka --> i1
  NodeContainer net20 (kurashiki, fukuoka);        // kurashiki --> i2, fukuoka --> i2
  NodeContainer net21 (fukuoka, komatsu);          // fukuoka --> i3, komatsu --> i2

  //DNS Servers<-->Router
  NodeContainer net22 (localDNS, sendai);   // sendai  --> i3
  NodeContainer net23 (rootDNS, fukuoka);   // fukuoka --> i4
  NodeContainer net24 (tldDNS, hiroshima);  // hiroshima --> i2
  NodeContainer net25 (ispDNS, sakyo);      // sakyo --> i3
  NodeContainer net26 (authDNS, dojima);    // dojima --> i8

  // Contenet SErvers<-->Routers
  NodeContainer net27 (sServer1, hiroshima);  // hiroshima --> i3

  // routers
  NodeContainer routerSet1 (sendai, tsukuba, nezu, kddiOtemachi, nttOtemachi);
  NodeContainer routerSet2 (shinkawasaki, yagami, fujisawa, nara, dojima);
  NodeContainer routerSet3 (komatsu, sakyo, hiroshima, kurashiki, fukuoka);
  NodeContainer routers (routerSet1, routerSet2, routerSet3);

  // clients
  NodeContainer clientSet1 (c1);
  //DNS servers
  NodeContainer nameServers (localDNS, rootDNS, tldDNS, ispDNS, authDNS);
  // Contente servers
  NodeContainer contentSrevers (sServer1);
  NodeContainer nodes (clientSet1, nameServers, contentSrevers);

  NS_LOG_INFO ("Create channels.");

  NS_LOG_INFO ("Set 10Gbps Links.");
  PointToPointHelper p2p_10Gbps;
  p2p_10Gbps.SetDeviceAttribute ("DataRate", StringValue ("10Gbps"));
  p2p_10Gbps.SetChannelAttribute ("Delay", StringValue ("2ms"));  // Transmission Delay is a guess

  NS_LOG_INFO ("Set 1Gbps Links.");
  PointToPointHelper p2p_1Gbps;
  p2p_1Gbps.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  p2p_1Gbps.SetChannelAttribute ("Delay", StringValue ("2ms"));  // Transmission Delay is a guess

  NS_LOG_INFO ("Set 100Mbps Links.");
  PointToPointHelper p2p_100Mbps;
  p2p_100Mbps.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p_100Mbps.SetChannelAttribute ("Delay", StringValue ("2ms"));  // Transmission Delay is a guess

  // links between routers according to the WIDE topology infomration
  NetDeviceContainer ndc2 = p2p_1Gbps.Install (net2);      // net2 sendai --> i2, nezu --> i1
  NetDeviceContainer ndc3 = p2p_10Gbps.Install (net3);     //net3 nezu --> i2, kddiOtemachi --> i1
  NetDeviceContainer ndc4 = p2p_10Gbps.Install (net4);     //net4 nezu --> i3, yagami --> i1
  NetDeviceContainer ndc5 = p2p_10Gbps.Install (net5);     //net5 nezu --> i4, dojima --> i1
  NetDeviceContainer ndc6 = p2p_10Gbps.Install (net6);     //net6 kddiOtemachi --> i2, nttOtemachi --> i1
  NetDeviceContainer ndc7 = p2p_10Gbps.Install (net7);     //net7 yagami --> i2, shinkawasaki --> i1
  NetDeviceContainer ndc8 = p2p_10Gbps.Install (net8);     //net8 yagami --> i3, fujisawa --> i1
  NetDeviceContainer ndc9 = p2p_10Gbps.Install (net9);     //net9 nttOtemachi --> i2, fujisawa --> i2
  NetDeviceContainer ndc10 = p2p_100Mbps.Install (net10);  // net10 nttOtemachi --> i3, tsukuba --> i1
  NetDeviceContainer ndc11 = p2p_10Gbps.Install (net11);   //net11 nttOtemachi --> i4, komatsu --> i1
  NetDeviceContainer ndc12 = p2p_10Gbps.Install (net12);   //net12 nttOtemachi --> i5, dojima --> i2
  NetDeviceContainer ndc13 = p2p_100Mbps.Install (net13);  // net13 fujisawa --> i3, nara --> i1
  NetDeviceContainer ndc14 = p2p_1Gbps.Install (net14);    //net14 nara --> i2, sakyo --> i1
  NetDeviceContainer ndc15 = p2p_10Gbps.Install (net15);   //net15 nara --> i3, dojima --> i3
  NetDeviceContainer ndc16 = p2p_1Gbps.Install (net16);    // net16 dojima --> i4, sakyo --> i2
  NetDeviceContainer ndc17 = p2p_100Mbps.Install (net17);  //net17 dojima --> i5, hiroshima --> i1
  NetDeviceContainer ndc18 = p2p_10Gbps.Install (net18);   //net18 dojima --> i6, kurashiki --> i1
  NetDeviceContainer ndc19 = p2p_10Gbps.Install (net19);   //net19 dojima --> i7, fukuoka --> i1
  NetDeviceContainer ndc20 = p2p_10Gbps.Install (net20);   //net20 kurashiki --> i2, fukuoka --> i2
  NetDeviceContainer ndc21 = p2p_10Gbps.Install (net21);   //net21 fukuoka --> i3, komatsu --> i2

  // links between clients and their gateway routers
  NetDeviceContainer ndc1 = p2p_1Gbps.Install (net1);  // net1 Sendai --> i1

  // Links between Name servers and their gateway router
  NetDeviceContainer ndc22 = p2p_100Mbps.Install (net22);  //net22 sendai --> i3
  NetDeviceContainer ndc23 = p2p_100Mbps.Install (net23);  //net23 fukuoka --> i4
  NetDeviceContainer ndc24 = p2p_100Mbps.Install (net24);  //net24 hiroshima --> i2
  NetDeviceContainer ndc25 = p2p_100Mbps.Install (net25);  //net25 sakyo --> i3
  NetDeviceContainer ndc26 = p2p_100Mbps.Install (net26);  //net26 dojima --> i8

  // servers and their gateways
  NetDeviceContainer ndc27 = p2p_1Gbps.Install (net27);  // net1 Hiroshima --> i3

  NS_LOG_INFO ("Create IPv4 and routing");
  EslrHelper eslrRouting;

  // Rule of thumb:
  // Interfaces are added sequentially, starting from 0
  // However, interface 0 is always the loopback...

  // exclude the interfaces of name servers and clients been advertising the route advertisements
  eslrRouting.ExcludeInterface (sendai, 1);
  eslrRouting.ExcludeInterface (sendai, 3);
  eslrRouting.ExcludeInterface (fukuoka, 4);
  eslrRouting.ExcludeInterface (hiroshima, 2);
  eslrRouting.ExcludeInterface (hiroshima, 3);
  eslrRouting.ExcludeInterface (sakyo, 3);
  eslrRouting.ExcludeInterface (dojima, 8);

  eslrRouting.Set ("PrintingMethod", EnumValue (eslr::MAIN_R_TABLE));
  Ipv4ListRoutingHelper list;
  list.Add (eslrRouting, 0);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list);
  internet.Install (routers);

  InternetStackHelper internetNodes;
  internetNodes.Install (nodes);

  NS_LOG_INFO ("Assign IPv4 Addresses.");
  Ipv4AddressHelper ipv4;

  // For clients
  ipv4.SetBase ("192.168.16.0", "255.255.255.252");
  Ipv4InterfaceContainer iic1 = ipv4.Assign (ndc1);  // Sendai,i1 <--> client1,1

  // For internal Network
  ipv4.SetBase ("203.178.136.228", "255.255.255.252");
  Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);  // sendai,i2 <--> nezu,i1
  ipv4.SetBase ("203.178.136.220", "255.255.255.252");
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);  //nezu,i2 <--> kddiOtemachi,i1
  ipv4.SetBase ("203.178.136.92", "255.255.255.252");
  Ipv4InterfaceContainer iic4 = ipv4.Assign (ndc4);  //nezu,i3 <--> yagami,i1
  ipv4.SetBase ("203.178.136.72", "255.255.255.252");
  Ipv4InterfaceContainer iic5 = ipv4.Assign (ndc5);  //nezu,i4 <--> dojima,i1
  ipv4.SetBase ("203.178.138.0", "255.255.255.0");
  Ipv4InterfaceContainer iic6 = ipv4.Assign (ndc6);  //kddiOtemachi,i2 <--> nttOtemachi,i1
  ipv4.SetBase ("203.178.136.244", "255.255.255.252");
  Ipv4InterfaceContainer iic7 = ipv4.Assign (ndc7);  //yagami,i2 <--> shinkawasaki,i1
  ipv4.SetBase ("203.178.137.64", "255.255.255.224");
  Ipv4InterfaceContainer iic8 = ipv4.Assign (ndc8);  //yagami,i3 <--> fujisawa,i1
  ipv4.SetBase ("202.244.32.248", "255.255.255.252");
  Ipv4InterfaceContainer iic9 = ipv4.Assign (ndc9);  //nttOtemachi,i2 <--> fujisawa,i2
  ipv4.SetBase ("203.178.136.204", "255.255.255.252");
  Ipv4InterfaceContainer iic10 = ipv4.Assign (ndc10);  //nttOtemachi,i3 <--> tsukuba,i1
  ipv4.SetBase ("203.178.138.208", "255.255.255.248");
  Ipv4InterfaceContainer iic11 = ipv4.Assign (ndc11);  //nttOtemachi,i4 <--> komatsu,i1
  ipv4.SetBase ("203.178.141.224", "255.255.255.224");
  Ipv4InterfaceContainer iic12 = ipv4.Assign (ndc12);  //nttOtemachi,i5 <--> dojima,i2
  ipv4.SetBase ("203.178.136.184", "255.255.255.252");
  Ipv4InterfaceContainer iic13 = ipv4.Assign (ndc13);  //fujisawa,i3 <--> nara,i1
  ipv4.SetBase ("203.178.138.164", "255.255.255.252");
  Ipv4InterfaceContainer iic14 = ipv4.Assign (ndc14);  //nara,i2 <--> sakyo,i1
  ipv4.SetBase ("202.244.138.224", "255.255.255.224");
  Ipv4InterfaceContainer iic15 = ipv4.Assign (ndc15);  //nara,i3 <--> dojima,i3
  ipv4.SetBase ("203.178.138.96", "255.255.255.224");
  Ipv4InterfaceContainer iic16 = ipv4.Assign (ndc16);  //dojima,i4 <--> sakyo,i2
  ipv4.SetBase ("203.178.140.192", "255.255.255.224");
  Ipv4InterfaceContainer iic17 = ipv4.Assign (ndc17);  //dojima,i5 <--> hiroshima,i1
  ipv4.SetBase ("203.178.136.196", "255.255.255.252");
  Ipv4InterfaceContainer iic18 = ipv4.Assign (ndc18);  //dojima,i6 <--> kurashiki,i1
  ipv4.SetBase ("203.178.136.232", "255.255.255.252");
  Ipv4InterfaceContainer iic19 = ipv4.Assign (ndc19);  //dojima,i7 <--> fukuoka,i1
  ipv4.SetBase ("203.178.138.200", "255.255.255.252");
  Ipv4InterfaceContainer iic20 = ipv4.Assign (ndc20);  //kurashiki,i2 <--> fukuoka,i2
  ipv4.SetBase ("203.178.140.224", "255.255.255.224");
  Ipv4InterfaceContainer iic21 = ipv4.Assign (ndc21);  //fukuoka,i3 <--> komatsu,i2

  // For name servers
  ipv4.SetBase ("199.85.126.0", "255.255.255.252");
  Ipv4InterfaceContainer iic22 = ipv4.Assign (ndc22);  //sendai,i3 <-->	LocalDNS,i1
  ipv4.SetBase ("192.26.92.0", "255.255.255.252");
  Ipv4InterfaceContainer iic23 = ipv4.Assign (ndc23);  //fukuoka,i4 <-->	RootDNS,i1
  ipv4.SetBase ("98.139.183.0", "255.255.255.252");
  Ipv4InterfaceContainer iic24 = ipv4.Assign (ndc24);  //hiroshima,i2 <-->	TLDDNS,i1
  ipv4.SetBase ("192.54.93.0", "255.255.255.252");
  Ipv4InterfaceContainer iic25 = ipv4.Assign (ndc25);  //sakyo,i3 <--> ISPDNS,i1
  ipv4.SetBase ("206.190.36.0", "255.255.255.252");
  Ipv4InterfaceContainer iic26 = ipv4.Assign (ndc26);  //dojima,i8 <--> AuthDNS,i1

  // For servers
  ipv4.SetBase ("172.16.10.0", "255.255.255.252");
  Ipv4InterfaceContainer iic27 = ipv4.Assign (ndc27);  // Hiroshima,i3 <--> sServer1,1

  NS_LOG_INFO ("Setting the default gateways of the Source and Destination.");
  Ipv4StaticRoutingHelper statRouting;
  // setting up the 'Sendai' as the default gateway of the 'C1'
  Ptr<Ipv4StaticRouting> statC1 = statRouting.GetStaticRouting (c1->GetObject<Ipv4> ());
  statC1->SetDefaultRoute (sendai->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal (), 1, 1);

  // Setting up the Hiroshima as the default gateway of Surrogate server1
  Ptr<Ipv4StaticRouting> statS1 = statRouting.GetStaticRouting (sServer1->GetObject<Ipv4> ());
  statS1->SetDefaultRoute (hiroshima->GetObject<Ipv4> ()->GetAddress (3, 0).GetLocal (), 1, 1);

  // Setting up the 'Sendai' as the default gateway of the "Local DNS"
  Ptr<Ipv4StaticRouting> statLocalDns = statRouting.GetStaticRouting (localDNS->GetObject<Ipv4> ());
  statLocalDns->SetDefaultRoute (sendai->GetObject<Ipv4> ()->GetAddress (3, 0).GetLocal (), 1, 1);
  // Setting up the 'Fukuoka' as the default gateway of the "Root DNS"
  Ptr<Ipv4StaticRouting> statRootDns = statRouting.GetStaticRouting (rootDNS->GetObject<Ipv4> ());
  statRootDns->SetDefaultRoute (fukuoka->GetObject<Ipv4> ()->GetAddress (4, 0).GetLocal (), 1, 1);
  // Setting up the 'Hiroshima' as the default gateway of the "TLD DNS"
  Ptr<Ipv4StaticRouting> statTldDns = statRouting.GetStaticRouting (tldDNS->GetObject<Ipv4> ());
  statTldDns->SetDefaultRoute (hiroshima->GetObject<Ipv4> ()->GetAddress (2, 0).GetLocal (), 1, 1);
  // Setting up the 'Sakyo' as the default gateway of the "ISP DNS"
  Ptr<Ipv4StaticRouting> statIspDns = statRouting.GetStaticRouting (ispDNS->GetObject<Ipv4> ());
  statIspDns->SetDefaultRoute (sakyo->GetObject<Ipv4> ()->GetAddress (3, 0).GetLocal (), 1, 1);
  // Setting up the 'Dojima' as the default gateway of the "Local DNS"
  Ptr<Ipv4StaticRouting> statAuthDns = statRouting.GetStaticRouting (authDNS->GetObject<Ipv4> ());
  statAuthDns->SetDefaultRoute (dojima->GetObject<Ipv4> ()->GetAddress (8, 0).GetLocal (), 1, 1);

  NS_LOG_INFO ("Setting up Name servers");

  //  Ipv4Address add;
  //  add = Ipv4Address (serverAddress.c_str ());

  std::string serverAddress;
  //http://はじめよう.みんな.jp
  //Local DNS
  BindServerHelper localNs (BindServer::LOCAL_SERVER);
  localNs.SetAttribute ("SetRecursiveSupport", EnumValue (BindServer::RA_AVAILABLE));
  localNs.SetAttribute ("SetServerAddress",
                        Ipv4AddressValue (localDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()));
  localNs.SetAttribute ("RootServerAddress",
                        Ipv4AddressValue (rootDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()));
  ApplicationContainer appsLocal = localNs.Install (localDNS);

  ////  std::stringstream localDNSAddress;
  ////  localDNSAddress << sServer1->GetObject<Ipv4> ()->GetAddress (1,0).GetLocal ();
  ////  serverAddress = localDNSAddress.str ();
  ////  localNs.AddNSRecord (appsLocal.Get (0),
  ////                        "west.keio.ac.jp",
  ////                        3500,
  ////                        1,
  ////                        1,
  ////                        sServer1->GetObject<Ipv4> ()->GetAddress (1,0).GetLocal (), "");
  //  localNs.AddNSRecord (appsLocal.Get (0),
  //                        "server1.west.keio.ac.jp",
  //                        3500,
  //                        1,
  //                        1,
  //                        serverAddress);
  //  localNs.AddNSRecord (appsLocal.Get (0),
  //                        "server2.west.keio.ac.jp",
  //                        3500,
  //                        1,
  //                        1,
  //                        "this.is.for.testing");
  //  localNs.AddNSRecord (appsLocal.Get (0),
  //                        "server3.west.keio.ac.jp",
  //                        3500,
  //                        1,
  //                        1,
  //                        "this.is.testing");
  ////
  appsLocal.Start (Seconds (10.0));
  appsLocal.Stop (Seconds (2500.0));

  // Root DNS
  BindServerHelper rootNs (BindServer::ROOT_SERVER);
  rootNs.SetAttribute ("SetServerAddress",
                       Ipv4AddressValue (rootDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()));
  ApplicationContainer appsRoot = rootNs.Install (rootDNS);
  // Add TLD name server addresses

  serverAddress = "";
  std::stringstream rootDNSAddress;
  rootDNSAddress << tldDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
  serverAddress = rootDNSAddress.str ();
  rootNs.AddNSRecord (appsRoot.Get (0),
                      ".com", /*domain*/
                      86400,  /*TTL*/
                      1,      /*class*/
                      1,      /*type*/
                      serverAddress /*resource record*/);
  rootNs.AddNSRecord (appsRoot.Get (0),
                      ".net",
                      86400,
                      1,
                      1,
                      serverAddress);
  rootNs.AddNSRecord (appsRoot.Get (0),
                      ".jp",
                      86400,
                      1,
                      1,
                      serverAddress);

  appsRoot.Start (Seconds (9.0));
  appsRoot.Stop (Seconds (2500.0));

  // TLD DNS
  BindServerHelper tldNs (BindServer::TLD_SERVER);
  tldNs.SetAttribute ("SetServerAddress",
                      Ipv4AddressValue (tldDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()));
  ApplicationContainer appsTld = tldNs.Install (tldDNS);

  // Setting up Providers name server addresses
  serverAddress = "";
  std::stringstream tldDNSAddress;
  tldDNSAddress << ispDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
  serverAddress = tldDNSAddress.str ();
  tldNs.AddNSRecord (appsTld.Get (0),
                     ".ac.jp",
                     86400,
                     1,
                     1,
                     serverAddress);
  tldNs.AddNSRecord (appsTld.Get (0),
                     ".みんな.jp",
                     86400,
                     1,
                     1,
                     serverAddress);

  appsTld.Start (Seconds (9.0));
  appsTld.Stop (Seconds (2500.0));

  // Provider DNS
  BindServerHelper ispNs (BindServer::ISP_SERVER);
  ispNs.SetAttribute ("SetServerAddress",
                      Ipv4AddressValue (ispDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()));
  ApplicationContainer appsIsp = ispNs.Install (ispDNS);

  // Setting up Authoratative name server addresses
  serverAddress = "";
  std::stringstream ispDNSAddress;
  ispDNSAddress << authDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
  serverAddress = ispDNSAddress.str ();
  ispNs.AddNSRecord (appsIsp.Get (0),
                     ".keio.ac.jp",
                     86400,
                     1,
                     1,
                     serverAddress);
  ispNs.AddNSRecord (appsIsp.Get (0),
                     "はじめよう.みんな.jp",
                     86400,
                     1,
                     1,
                     serverAddress);
  appsIsp.Start (Seconds (9.0));
  appsIsp.Stop (Seconds (2500.0));

  // Authoratative DNS
  BindServerHelper authNs (BindServer::AUTH_SERVER);
  authNs.SetAttribute ("SetServerAddress",
                       Ipv4AddressValue (authDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()));
  ApplicationContainer appsAuth = authNs.Install (authDNS);

  // Setting up Authoratative name server addresses
  serverAddress = "";
  std::stringstream authDNSAddress;
  authDNSAddress << sServer1->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
  serverAddress = authDNSAddress.str ();
  authNs.AddNSRecord (appsAuth.Get (0),
                      "server2.west.keio.ac.jp",
                      400,
                      1,
                      1,
                      serverAddress);
  serverAddress = "";
  std::stringstream authDNSAddress1;
  authDNSAddress1 << sServer1->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
  serverAddress = authDNSAddress1.str ();
  authNs.AddNSRecord (appsAuth.Get (0),
                      "server1.west.keio.ac.jp",
                      400,
                      1,
                      1,
                      serverAddress);

  serverAddress = "";
  std::stringstream authDNSAddress3;
  authDNSAddress3 << sServer1->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
  serverAddress = authDNSAddress3.str ();
  authNs.AddNSRecord (appsAuth.Get (0),
                      "server3.west.keio.ac.jp",
                      400,
                      1,
                      1,
                      serverAddress);

  appsAuth.Start (Seconds (180.0));
  appsAuth.Stop (Seconds (2500.0));

  //create client1
  UdpEchoClientHelper client1 (localDNS->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal (), 9, true);
  client1.SetAttribute ("MaxPackets", UintegerValue (1000));
  client1.SetAttribute ("Id", UintegerValue (1));
  client1.SetAttribute ("PacketSize", UintegerValue (100));
  client1.SetAttribute ("EnableDNS", BooleanValue (true));
  ApplicationContainer appClient = client1.Install (c1);

  //client1.SetUrl (appClient.Get(0), "はじめよう.みんな.jp");
  client1.SetUrl (appClient.Get (0), "west.keio.ac.jp");
  appClient.Start (Seconds (200.0));
  appClient.Stop (Seconds (450.0));

  UdpEchoServerHelper server1 (9);
  server1.SetAttribute ("ServerAddress", Ipv4AddressValue (sServer1->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()));
  server1.SetAttribute ("NetMask", Ipv4MaskValue (sServer1->GetObject<Ipv4> ()->GetAddress (1, 0).GetMask ()));
  server1.SetAttribute ("ISPAddress", Ipv4AddressValue (hiroshima->GetObject<Ipv4> ()->GetAddress (3, 0).GetLocal ()));
  ApplicationContainer apps = server1.Install (sServer1);
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (550.0));

  // // For Priting the Routing tables
  // EslrHelper routingHelper;
  // Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
  //
  // routingHelper.PrintRoutingTableAt (Seconds (250), routers.Get (12), routingStream);

  Simulator::Stop (Seconds (455));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
