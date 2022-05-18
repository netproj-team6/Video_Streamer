#ifndef CLIENT_H
#define CLIENT_H

#include <set>
#include <queue>
#include <vector>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/traced-value.h"

namespace ns3 {

class Socket;
class Packet;

class StreamingClient : public Application 
{
public:

  static TypeId GetTypeId (void);

  StreamingClient ();

  virtual ~StreamingClient ();

  void SetDataSize (uint32_t dataSize);

  uint32_t GetDataSize (void) const;

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleRequest(Time dt);
  void RequestStreaming(void);

  void ScheduleGenerator (Time dt);
  void GenerateFrame (void);

  void ScheduleConsumer (Time dt);
  void ConsumeFrame (void);

  EventId m_generatorEvent;
  EventId m_consumerEvent;

  void HandleRead (Ptr<Socket> socket);

  uint32_t m_count;
  Time m_intervalGenerator;
  Time m_intervalConsumer;
  uint32_t m_size;

  uint32_t m_dataSize;
  uint8_t *m_data;

  uint32_t m_sent;
  Ptr<Socket> m_socket;
  Address m_peerAddress;
  uint16_t m_peerPort;

  Ptr<Socket> m_socketRecv;
  Address m_local;
  uint16_t m_port;

  uint32_t packetsPerFrame;
  uint32_t frameIdx;
  uint32_t packetsSize;

  std::set<uint32_t> packets;
  std::set<uint32_t> frames;
  std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t> > frameBuffer;

  double lossRate;

  TracedCallback<Ptr<const Packet> > m_txTrace;
  TracedCallback<Ptr<const Packet> > m_rxTrace;
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_txTraceWithAddresses;
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;
};

}

#endif
