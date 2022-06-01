#ifndef CLIENT_H
#define CLIENT_H

#include <queue>
#include <set>
#include <vector>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/traced-value.h"

namespace ns3 {

    class Socket;
    class Packet;

    class StreamingClient : public Application
    {
    public:

        static TypeId GetTypeId(void);

        StreamingClient();
        virtual ~StreamingClient();

    protected:

        virtual void DoDispose(void);

    private:

        virtual void StartApplication(void);
        virtual void StopApplication(void);

        void ScheduleRequest(Time dt);
        void ScheduleGenerator(Time dt);
        void ScheduleConsumer(Time dt);

        void RequestStreaming(void);
        void GenerateFrame(void);
        void ConsumeFrame(void);

        void HandleRead(Ptr<Socket> socket);

        Ptr<Socket> m_socket;
        Address m_peerAddress;
        uint16_t m_peerPort;

        double m_lossRate;

        uint32_t m_size;
        uint32_t m_packetsPerFrame;
        uint32_t m_bufferingSize;
        uint32_t m_pauseSize;
        uint32_t m_resumeSize;

        Time m_intervalRequest;
        Time m_intervalGenerator;
        Time m_intervalConsumer;

        EventId m_requestEvent;
        EventId m_generatorEvent;
        EventId m_consumerEvent;

        std::set<uint32_t> m_packets;
        std::set<uint32_t> m_frames;
        std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t> > m_frameBuffer;

        uint32_t m_frameIdx;
        bool m_packetFlag;

        uint32_t m_nPackets;

        TracedCallback<uint32_t> m_nTrace;
        TracedCallback<Ptr<const Packet>, const Address&> m_rxTrace;
        TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;
    };
}

#endif