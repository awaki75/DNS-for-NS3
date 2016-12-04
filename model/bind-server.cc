#include "bind-server.h"
#include "ns3/abort.h"
#include "ns3/address-utils.h"
#include "ns3/dns-header.h"
#include "ns3/dns.h"
#include "ns3/enum.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("BindServer");
NS_OBJECT_ENSURE_REGISTERED (BindServer);
TypeId
BindServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BindServer")
                        .SetParent<Application> ()
                        .AddConstructor<BindServer> ()
                        .AddAttribute ("SetServerAddress",
                                       "IP address of the server.",
                                       Ipv4AddressValue (),
                                       MakeIpv4AddressAccessor (&BindServer::m_localAddress),
                                       MakeIpv4AddressChecker ())
                        .AddAttribute ("RootServerAddress",
                                       "Ip address of the Root Server (This is only needed for Local NS).",
                                       Ipv4AddressValue (),
                                       MakeIpv4AddressAccessor (&BindServer::m_rootAddress),
                                       MakeIpv4AddressChecker ())
                        .AddAttribute ("SetNetMask",
                                       "Network Mask of the server.",
                                       Ipv4MaskValue (),
                                       MakeIpv4MaskAccessor (&BindServer::m_netMask),
                                       MakeIpv4MaskChecker ())
                        .AddAttribute ("NameServerType",
                                       " Type of the name server.",
                                       EnumValue (AUTH_SERVER),
                                       MakeEnumAccessor (&BindServer::m_serverType),
                                       MakeEnumChecker (LOCAL_SERVER, "LOCAL NAME SERVER",
                                                        ROOT_SERVER, "ROOT NAME SERVER",
                                                        TLD_SERVER, "TOP-LEVEL DOMAIN SERVER",
                                                        ISP_SERVER, "ISP'S NAME SERVER",
                                                        AUTH_SERVER, "AUTHORITATIVE NAME SERVER"))
                        .AddAttribute ("SetRecursiveSupport",
                                       "Set the name server support recursive IP resolution",
                                       EnumValue (BindServer::RA_UNAVAILABLE),
                                       MakeEnumAccessor (&BindServer::m_raType),
                                       MakeEnumChecker (BindServer::RA_UNAVAILABLE, "Does not support",
                                                        BindServer::RA_AVAILABLE, "Support"));
  return tid;
}

BindServer::BindServer (void)
{
  m_localAddress = Ipv4Address ();
  m_netMask = Ipv4Mask ();
  m_socket = 0;
  /* cstrctr */
}
BindServer::~BindServer ()
{
  /* dstrctr */
}

void
BindServer::AddZone (std::string zone_name, uint32_t TTL, uint16_t ns_class, uint16_t type, std::string rData)
{
  NS_LOG_FUNCTION (this << zone_name << TTL << ns_class << type << rData);
  m_nsCache.AddZone (zone_name, ns_class, type, TTL, rData);
}

void
BindServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  // Start expiration of the DNS records after TTL values.
  m_nsCache.SynchronizeTTL ();

  if (m_socket == 0)
  {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);
    InetSocketAddress local = InetSocketAddress (m_localAddress, DNS_PORT);
    m_socket->Bind (local);
  }
  m_socket->SetRecvCallback (MakeCallback (&BindServer::HandleQuery, this));
}

void
BindServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
  {
    m_socket->Close ();
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  }
  DoDispose ();
}

void
BindServer::HandleQuery (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> message;
  Address from;

  while ((message = socket->RecvFrom (from)))
  {
    if (InetSocketAddress::IsMatchingType (from))
    {
      // TODO
      // NS_LOG_INFO ()
    }

    message->RemoveAllPacketTags ();
    message->RemoveAllByteTags ();

    if (m_serverType == LOCAL_SERVER)
    {
      LocalServerService (message, from);
    }
    else if (m_serverType == ROOT_SERVER)
    {
      RootServerService (message, from);
    }
    else if (m_serverType == TLD_SERVER)
    {
      TLDServerService (message, from);
    }
    else if (m_serverType == ISP_SERVER)
    {
      ISPServerService (message, from);
    }
    else if (m_serverType == AUTH_SERVER)
    {
      AuthServerService (message, from);
    }
    else
    {
      NS_ABORT_MSG ("Name server should have a type. Hint: Set NameserverType. Aborting");
    }
  }
}

void
BindServer::LocalServerService (Ptr<Packet> nsQuery, Address toAddress)
{
  NS_LOG_FUNCTION (this);

  DNSHeader DnsHeader;
  uint16_t qType, qClass;
  std::string qName;
  bool foundInCache = false;
  bool nsQuestion = false;

  nsQuery->RemoveHeader (DnsHeader);

  if ((nsQuestion = DnsHeader.GetQRbit ()))  // if NS query
  {
    // retrieve the question list
    std::list<QuestionSectionHeader> questionList;
    questionList = DnsHeader.GetQuestionList ();

    // Although the header supports multiple questions at a time
    // the local DNS server is not yet implemented to resolve multiple questions at a time.
    // We assume that clients generate separate DNS messages for each
    // host that they wanted to resolve.

    // NOTE
    // We assumed that the local DNS does the recursive resolution process (i.e., in CDN networks)

    qName = questionList.begin ()->GetqName ();
    qType = questionList.begin ()->GetqType ();
    qClass = questionList.begin ()->GetqClass ();

    SRVTable::SRVRecordI cachedRecord = m_nsCache.FindARecordHas (qName, foundInCache);

    // // Find the query in the nameserver cache
    // SRVTable::SRVRecordInstance instance;
    // foundInCache = m_nsCache.FindRecordsFor (qName, instance);

    if (foundInCache)
    {
      NS_LOG_INFO ("Found a record in the local cache. Replying..");

      // Local Server always returns the server address according to the RR manner.
      Ptr<Packet> dnsResponse = Create<Packet> ();

      ResourceRecordHeader answer;
      answer.SetName (cachedRecord->first->GetRecordName ());
      answer.SetClass (cachedRecord->first->GetClass ());
      answer.SetType (cachedRecord->first->GetType ());
      answer.SetTimeToLive (25);  //cachedRecord->first->GetTTL ());
      answer.SetRData (cachedRecord->first->GetRData ());

      DnsHeader.AddAnswer (answer);

      DnsHeader.SetRAbit (1);

      dnsResponse->AddHeader (DnsHeader);

      // Add the DNS header to the response packet and send it to the client
      dnsResponse->AddHeader (DnsHeader);
      ReplyQuery (dnsResponse, toAddress);

      // Toggle servers
      m_nsCache.SwitchServersRoundRobin ();

      return;
    }  // end of query is found in cache
    else if (!foundInCache && (m_raType == RA_AVAILABLE))
    {
      NS_LOG_INFO ("Initiate recursive resolution.");

      Ptr<Packet> requestRR = Create<Packet> ();

      std::string tld;
      std::string::size_type found = 0;
      bool foundTLDinCache = false;

      // find the TLD of the query
      found = qName.find_last_of ('.');
      tld = qName.substr (found);

      // Find the TLD in the nameserver cache
      SRVTable::SRVRecordI cachedTLDRecord = m_nsCache.FindARecord (tld, foundTLDinCache);

      requestRR->AddHeader (DnsHeader);

      NS_LOG_INFO ("Add the recursive request in to the list");
      m_recursiveQueryList[qName] = toAddress;

      if (foundTLDinCache)
      {
        // Send to the TLD server
        SendQuery (requestRR, InetSocketAddress (Ipv4Address (cachedTLDRecord->first->GetRData ().c_str ()), DNS_PORT));
      }
      else
      {
        // Send to the reqiest to the Root server
        SendQuery (requestRR, InetSocketAddress (m_rootAddress, DNS_PORT));
      }
      return;
    }   // end of not found in cache and recursive resolution
  }     // end of the NS query
  else  // // NS response !rHeader.GetQRbit ()
  {
    NS_LOG_INFO ("Handle the NS responses from recursive name servers");

    // NOTE
    // In this implementation, the remaining bits of the OPCODE are used to specify the reply types.
    // 3 for the replies from a ROOT server
    // 4 for the replies from a TLD server
    // 5 for the replies from a ISP's name server
    // 6 fro the replies from a Authoritative name server (in this case the AA bit is also considered)

    // All answers are append to the DNS header.
    // However, only the relevant answer is taken according to the OPCODE value.
    // Furthermore, we implemented the servers to add the resource record to the top of the answer section.

    std::string forwardingAddress;

    // retrieve the Answer list
    std::list<ResourceRecordHeader> answerList;
    answerList = DnsHeader.GetAnswerList ();

    // Always use the most recent answer, so that the previous server is considered.
    // However, the answer list contains all answers recursive name servers added.
    qName = answerList.begin ()->GetName ();
    qType = answerList.begin ()->GetType ();
    qClass = answerList.begin ()->GetClass ();
    forwardingAddress = answerList.begin ()->GetRData ();

    if (DnsHeader.GetOpcode () == 3)  // reply from the root server about a TLD server
    {
      NS_LOG_INFO ("Add the TLD record in to the server cache");

      std::string tld;
      std::string::size_type foundAt = 0;
      foundAt = qName.find_last_of ('.');
      tld = qName.substr (foundAt);

      // add the record about TLD to the Local name server cache
      m_nsCache.AddRecord (tld,
                           answerList.begin ()->GetClass (),
                           answerList.begin ()->GetType (),
                           answerList.begin ()->GetTimeToLive (),
                           answerList.begin ()->GetRData ());

      // create a packet to send to TLD
      Ptr<Packet> sendToTLD = Create<Packet> ();

      DnsHeader.ResetOpcode ();
      DnsHeader.SetOpcode (0);
      DnsHeader.SetQRbit (1);
      sendToTLD->AddHeader (DnsHeader);

      SendQuery (sendToTLD, InetSocketAddress (Ipv4Address (forwardingAddress.c_str ()), DNS_PORT));
    }
    else if (DnsHeader.GetOpcode () == 4)
    {
      Ptr<Packet> sendToISP = Create<Packet> ();

      DnsHeader.ResetOpcode ();
      DnsHeader.SetOpcode (0);
      DnsHeader.SetQRbit (1);
      sendToISP->AddHeader (DnsHeader);

      SendQuery (sendToISP, InetSocketAddress (Ipv4Address (forwardingAddress.c_str ()), DNS_PORT));
      NS_LOG_INFO ("Contact ISP name server");
    }
    else if (DnsHeader.GetOpcode () == 5)
    {
      // If the ISP's name server says that it has the authoritative records,
      // cache it and pass it to the user.
      if (DnsHeader.GetAAbit ())
      {
        NS_LOG_INFO ("Add the Auth records in to the server cache");

        std::list<ResourceRecordHeader> answerList;
        answerList = DnsHeader.GetAnswerList ();

        // Store all answers, i.e., server records, to the Local DNS cache
        for (std::list<ResourceRecordHeader>::iterator iter = answerList.begin ();
             iter != answerList.end ();
             iter++)
        {
          m_nsCache.AddRecord (iter->GetName (),
                               iter->GetClass (),
                               iter->GetType (),
                               /*iter->GetTimeToLive ()*/ 40,
                               iter->GetRData ());
        }
        // Clear the existing answer list
        DnsHeader.ClearAnswers ();

        // Get the recent query from the cache.
        // TODO: This approach can be optimized
        bool foundInCache = false;
        SRVTable::SRVRecordI cachedRecord = m_nsCache.FindARecordHas (qName, foundInCache);

        // Create the Answer and reply it back to the client

        ResourceRecordHeader answer;

        answer.SetName (cachedRecord->first->GetRecordName ());
        answer.SetClass (cachedRecord->first->GetClass ());
        answer.SetType (cachedRecord->first->GetType ());
        answer.SetTimeToLive (25);  //(cachedRecord->first->GetTTL ());		// bypassed for testing purposes
        answer.SetRData (cachedRecord->first->GetRData ());

        DnsHeader.AddAnswer (answer);

        // create a packet to send to TLD
        Ptr<Packet> replyToClient = Create<Packet> ();

        DnsHeader.ResetOpcode ();
        DnsHeader.SetOpcode (0);
        DnsHeader.SetQRbit (0);
        DnsHeader.SetAAbit (1);
        replyToClient->AddHeader (DnsHeader);

        // Find the actual client query that stores in recursive list
        std::list<QuestionSectionHeader> questionList;
        questionList = DnsHeader.GetQuestionList ();
        qName = questionList.begin ()->GetqName ();

        ReplyQuery (replyToClient, m_recursiveQueryList.find (qName)->second);

        m_recursiveQueryList.erase (qName);
        m_nsCache.SwitchServersRoundRobin ();
      }
      else
      {
        Ptr<Packet> sendToAUTH = Create<Packet> ();

        DnsHeader.ResetOpcode ();
        DnsHeader.SetOpcode (0);
        DnsHeader.SetQRbit (1);
        sendToAUTH->AddHeader (DnsHeader);

        SendQuery (sendToAUTH, InetSocketAddress (Ipv4Address (forwardingAddress.c_str ()), DNS_PORT));
        NS_LOG_INFO ("Contact Authoritative name server");
      }
    }
    else if (DnsHeader.GetOpcode () == 6)
    {
      NS_LOG_INFO ("Add the Auth records in to the server cache");

      std::list<ResourceRecordHeader> answerList;
      answerList = DnsHeader.GetAnswerList ();

      // Store all answers, i.e., server records, to the Local DNS cache
      for (std::list<ResourceRecordHeader>::iterator iter = answerList.begin ();
           iter != answerList.end ();
           iter++)
      {
        m_nsCache.AddRecord (iter->GetName (),
                             iter->GetClass (),
                             iter->GetType (),
                             iter->GetTimeToLive () /* 40 */,
                             iter->GetRData ());
      }
      // Clear the existing answer list
      DnsHeader.ClearAnswers ();

      // Get the recent query from the cache.
      // TODO: This approach can be optimized
      bool foundInCache = false;
      SRVTable::SRVRecordI cachedRecord = m_nsCache.FindARecordHas (qName, foundInCache);

      // Create the Answer and reply it back to the client

      ResourceRecordHeader answer;

      answer.SetName (cachedRecord->first->GetRecordName ());
      answer.SetClass (cachedRecord->first->GetClass ());
      answer.SetType (cachedRecord->first->GetType ());
      answer.SetTimeToLive (25);  // (cachedRecord->first->GetTTL ());  // bypassed for testing purposes
      answer.SetRData (cachedRecord->first->GetRData ());

      DnsHeader.AddAnswer (answer);

      // create a packet to send to TLD
      Ptr<Packet> replyToClient = Create<Packet> ();

      DnsHeader.ResetOpcode ();
      DnsHeader.SetOpcode (0);
      DnsHeader.SetQRbit (0);
      DnsHeader.SetAAbit (1);
      replyToClient->AddHeader (DnsHeader);

      // Find the actual client query that stores in recursive list
      std::list<QuestionSectionHeader> questionList;
      questionList = DnsHeader.GetQuestionList ();
      qName = questionList.begin ()->GetqName ();

      ReplyQuery (replyToClient, m_recursiveQueryList.find (qName)->second);

      m_recursiveQueryList.erase (qName);
      m_nsCache.SwitchServersRoundRobin ();
    }
    else
    {
      // TODO
      // Abort with a error message
    }

  }  // end of ns response
}

void
BindServer::RootServerService (Ptr<Packet> nsQuery, Address toAddress)
{
  // Assumptions made to Create the ROOT name server
  // Root name servers never create any NS requests.

  DNSHeader DnsHeader;
  uint16_t qType, qClass;
  std::string qName;
  bool foundInCache = false;

  nsQuery->RemoveHeader (DnsHeader);

  // Assume that only one question is attached to the DNS header
  std::list<QuestionSectionHeader> questionList;
  questionList = DnsHeader.GetQuestionList ();

  qName = questionList.begin ()->GetqName ();
  qType = questionList.begin ()->GetqType ();
  qClass = questionList.begin ()->GetqClass ();

  // Return the first record that matches the requested qName
  // This supports the RR method
  SRVTable::SRVRecordI cachedRecord = m_nsCache.FindARecordMatches (qName, foundInCache);

  if (foundInCache)
  {
    Ptr<Packet> rootResponse = Create<Packet> ();

    ResourceRecordHeader rrHeader;

    rrHeader.SetName (cachedRecord->first->GetRecordName ());
    rrHeader.SetClass (cachedRecord->first->GetClass ());
    rrHeader.SetType (cachedRecord->first->GetType ());
    rrHeader.SetTimeToLive (cachedRecord->first->GetTTL ());
    rrHeader.SetRData (cachedRecord->first->GetRData ());

    DnsHeader.SetQRbit (0);
    DnsHeader.ResetOpcode ();
    DnsHeader.SetOpcode (3);
    DnsHeader.AddAnswer (rrHeader);

    rootResponse->AddHeader (DnsHeader);

    ReplyQuery (rootResponse, toAddress);
  }
  else
  {
    // TODO
    // Send a reply contains RCODE = 3
  }
}

void
BindServer::TLDServerService (Ptr<Packet> nsQuery, Address toAddress)
{
  DNSHeader DnsHeader;
  uint16_t qType, qClass;
  std::string qName;
  bool foundInCache = false;

  nsQuery->RemoveHeader (DnsHeader);

  // Assume that only one question is attached to the DNS header
  std::list<QuestionSectionHeader> questionList;
  questionList = DnsHeader.GetQuestionList ();

  qName = questionList.begin ()->GetqName ();
  qType = questionList.begin ()->GetqType ();
  qClass = questionList.begin ()->GetqClass ();

  // Return the first record that matches the requested qName
  // This supports the RR method
  SRVTable::SRVRecordI cachedRecord = m_nsCache.FindARecordMatches (qName, foundInCache);

  if (foundInCache)
  {
    Ptr<Packet> tldResponse = Create<Packet> ();

    ResourceRecordHeader rrHeader;

    rrHeader.SetName (cachedRecord->first->GetRecordName ());
    rrHeader.SetClass (cachedRecord->first->GetClass ());
    rrHeader.SetType (cachedRecord->first->GetType ());
    rrHeader.SetTimeToLive (cachedRecord->first->GetTTL ());
    rrHeader.SetRData (cachedRecord->first->GetRData ());

    DnsHeader.SetQRbit (0);
    DnsHeader.ResetOpcode ();
    DnsHeader.SetOpcode (4);
    DnsHeader.AddAnswer (rrHeader);

    tldResponse->AddHeader (DnsHeader);

    ReplyQuery (tldResponse, toAddress);
  }
  else
  {
    // TODO
    // Send a reply contains RCODE = 3
  }
}

void
BindServer::ISPServerService (Ptr<Packet> nsQuery, Address toAddress)
{
  DNSHeader DnsHeader;
  uint16_t qType, qClass;
  std::string qName;
  bool foundInCache = false, foundAuthRecordinCache = false;

  nsQuery->RemoveHeader (DnsHeader);

  // Assume that only one question is attached to the DNS header
  std::list<QuestionSectionHeader> questionList;
  questionList = DnsHeader.GetQuestionList ();

  qName = questionList.begin ()->GetqName ();
  qType = questionList.begin ()->GetqType ();
  qClass = questionList.begin ()->GetqClass ();

  // Find for a record that exactly matches the query name.
  // if the query is exactly matches for a record in the ISP cache,
  // the record is treated as a Authoritative record for the client.
  // In case the ISP name server could not find a auth. record for the query,
  // the ISP name server returns IP addresses of the Authoritative name servers
  // for the requested name.
  // To make the load distribution, we assumed the RR implementation for authoritative records.
  SRVTable::SRVRecordI foundAuthRecord = m_nsCache.FindARecord (qName, foundAuthRecordinCache);

  // Return the first record that matches the requested qName
  // This supports the RR method
  SRVTable::SRVRecordI cachedRecord = m_nsCache.FindARecordMatches (qName, foundInCache);

  Ptr<Packet> ispResponse = Create<Packet> ();

  if (foundAuthRecordinCache)
  {
    // Move the existing answer list to the Additional section.
    // 	This feature is implemented to track the recursive operation and
    // 	thus for debugging purposes.
    // Assume that only one question is attached to the DNS header
    NS_LOG_INFO ("Move the Existing recursive answer list in to additional section.");
    std::list<ResourceRecordHeader> answerList;
    answerList = DnsHeader.GetAnswerList ();

    for (std::list<ResourceRecordHeader>::iterator iter = answerList.begin ();
         iter != answerList.end ();
         iter++)
    {
      ResourceRecordHeader additionalRecord;

      additionalRecord.SetName (iter->GetName ());
      additionalRecord.SetClass (iter->GetClass ());
      additionalRecord.SetType (iter->GetType ());
      additionalRecord.SetTimeToLive (iter->GetTimeToLive ());
      additionalRecord.SetRData (iter->GetRData ());

      DnsHeader.AddARecord (additionalRecord);
    }
    // Clear the existing answer list
    DnsHeader.ClearAnswers ();

    ResourceRecordHeader rrHeader;

    rrHeader.SetName (foundAuthRecord->first->GetRecordName ());
    rrHeader.SetClass (foundAuthRecord->first->GetClass ());
    rrHeader.SetType (foundAuthRecord->first->GetType ());
    rrHeader.SetTimeToLive (foundAuthRecord->first->GetTTL ());
    rrHeader.SetRData (foundAuthRecord->first->GetRData ());

    DnsHeader.SetQRbit (0);
    DnsHeader.ResetOpcode ();
    DnsHeader.SetOpcode (5);
    DnsHeader.SetAAbit (1);
    DnsHeader.AddAnswer (rrHeader);

    ispResponse->AddHeader (DnsHeader);

    ReplyQuery (ispResponse, toAddress);

    // Change the order of server according to the round robin algorithm
    m_nsCache.SwitchServersRoundRobin ();
  }
  else if (foundInCache)
  {
    ResourceRecordHeader rrHeader;

    rrHeader.SetName (cachedRecord->first->GetRecordName ());
    rrHeader.SetClass (cachedRecord->first->GetClass ());
    rrHeader.SetType (cachedRecord->first->GetType ());
    rrHeader.SetTimeToLive (cachedRecord->first->GetTTL ());
    rrHeader.SetRData (cachedRecord->first->GetRData ());

    DnsHeader.SetQRbit (0);
    DnsHeader.ResetOpcode ();
    DnsHeader.SetOpcode (5);
    DnsHeader.AddAnswer (rrHeader);

    ispResponse->AddHeader (DnsHeader);

    ReplyQuery (ispResponse, toAddress);
  }
  else
  {
    // TODO
    // Send a reply contains RCODE = 3
  }
}

void
BindServer::AuthServerService (Ptr<Packet> nsQuery, Address toAddress)
{
  DNSHeader DnsHeader;
  uint16_t qType, qClass;
  std::string qName;
  bool foundInCache = false;  //, foundAARecords = false;

  nsQuery->RemoveHeader (DnsHeader);

  // Assume that only one question is attached to the DNS header
  std::list<QuestionSectionHeader> questionList;
  questionList = DnsHeader.GetQuestionList ();

  qName = questionList.begin ()->GetqName ();
  qType = questionList.begin ()->GetqType ();
  qClass = questionList.begin ()->GetqClass ();

  NS_UNUSED (foundInCache);

  // Find the query in the nameserver cache
  SRVTable::SRVRecordInstance instance;
  // foundInCache = m_nsCache.FindRecordsFor (qName, instance);
  foundInCache = m_nsCache.FindAllRecordsHas (qName, instance);

  if (foundInCache)
  {
    // Move the existing answer list to the Additional section.
    // This feature is implemented to track the recursive operation and
    // thus for debugging purposes.
    // Assume that only one question is attached to the DNS header
    NS_LOG_INFO ("Move the Existing recursive answer list in to additional section.");
    std::list<ResourceRecordHeader> answerList;
    answerList = DnsHeader.GetAnswerList ();

    for (std::list<ResourceRecordHeader>::iterator iter = answerList.begin ();
         iter != answerList.end ();
         iter++)
    {
      ResourceRecordHeader additionalRecord;

      additionalRecord.SetName (iter->GetName ());
      additionalRecord.SetClass (iter->GetClass ());
      additionalRecord.SetType (iter->GetType ());
      additionalRecord.SetTimeToLive (iter->GetTimeToLive ());
      additionalRecord.SetRData (iter->GetRData ());

      DnsHeader.AddARecord (additionalRecord);
    }
    // Clear the existing answer list
    DnsHeader.ClearAnswers ();

    // Now, add the server list as the new answer list
    NS_LOG_INFO ("Add the content server list as the new answer list of the DNS header.");
    // Create the response
    Ptr<Packet> authResponse = Create<Packet> ();

    // Get the found record list and add the records to the DNS header according to the Type
    for (SRVTable::SRVRecordI it = instance.begin (); it != instance.end (); it++)
    {
      // Assume that Number of DNS records will note results packet segmentation
      if (it->first->GetType () == 1)  // A host record or a CNAME record
      {
        ResourceRecordHeader rrHeader;

        rrHeader.SetName (it->first->GetRecordName ());
        rrHeader.SetClass (it->first->GetClass ());
        rrHeader.SetType (it->first->GetType ());
        rrHeader.SetTimeToLive (it->first->GetTTL ());
        rrHeader.SetRData (it->first->GetRData ());

        DnsHeader.AddAnswer (rrHeader);
      }
      if (it->first->GetType () == 2)  // A Authoritative Name server record
      {
        ResourceRecordHeader nsRecord;

        nsRecord.SetName (it->first->GetRecordName ());
        nsRecord.SetClass (it->first->GetClass ());
        nsRecord.SetType (it->first->GetType ());
        nsRecord.SetTimeToLive (it->first->GetTTL ());
        nsRecord.SetRData (it->first->GetRData ());

        DnsHeader.AddNsRecord (nsRecord);
      }
      if (it->first->GetType () == 5)  // A Authoritative Name server record
      {
        ResourceRecordHeader rrRecord;

        rrRecord.SetName (it->first->GetRecordName ());
        rrRecord.SetClass (it->first->GetClass ());
        rrRecord.SetType (it->first->GetType ());
        rrRecord.SetTimeToLive (it->first->GetTTL ());
        rrRecord.SetRData (it->first->GetRData ());

        DnsHeader.AddNsRecord (rrRecord);
      }
    }

    DnsHeader.SetQRbit (0);
    DnsHeader.SetAAbit (1);
    DnsHeader.ResetOpcode ();
    DnsHeader.SetOpcode (6);

    authResponse->AddHeader (DnsHeader);
    ReplyQuery (authResponse, toAddress);

    // Change the order of server according to the round robin algorithm
    m_nsCache.SwitchServersRoundRobin ();
  }
  else
  {
    // TODO
    // Send a reply contains RCODE = 3
  }
}

void
BindServer::SendQuery (Ptr<Packet> requestRecord, Address toAddress)
{
  NS_LOG_FUNCTION (this << requestRecord << InetSocketAddress::ConvertFrom (toAddress).GetIpv4 () << InetSocketAddress::ConvertFrom (toAddress).GetPort ());

  NS_LOG_INFO ("Server " << m_localAddress << " send a reply to " << InetSocketAddress::ConvertFrom (toAddress).GetIpv4 ());
  m_socket->SendTo (requestRecord, 0, toAddress);
}

void
BindServer::ReplyQuery (Ptr<Packet> nsQuery, Address toAddress)
{
  NS_LOG_FUNCTION (this << nsQuery << InetSocketAddress::ConvertFrom (toAddress).GetIpv4 () << InetSocketAddress::ConvertFrom (toAddress).GetPort ());

  NS_LOG_INFO ("Server " << m_localAddress << " send a reply to " << InetSocketAddress::ConvertFrom (toAddress).GetIpv4 ());
  m_socket->SendTo (nsQuery, 0, toAddress);
}
}
