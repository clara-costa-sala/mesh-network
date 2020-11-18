#ifndef LINK_H
#define LINK_H

#include <vector>
#include <string>
#include <iostream>
#include <mutex>
#include <random>
#include "Network.h"


class Link {
  public:
    Link();
    ~Link();

    Link(int id,
         int idNode_src,
         int idNode_dst,
         int layer_src,
         int layer_dst,
         uint64_t throughput,
         int latency,
         int jitter,
         float error_rate);

    Link(int id, int idNode_src, int idNode_dst, int layer_src, int layer_dst);

  protected:
    int id;
    int idNode_src;
    int idNode_dst;
    int layer_src;
    int layer_dst;
    uint64_t throughput;    // bps
    timeval latency;        // ms
    int jitter;             // ms
    float error_rate;       // 0 - 1
    timeval delay;
    timeval real_latency;
    struct nck_timer_entry *freeLink_timer;
    struct nck_timer *timer;
    struct packet_struct *packet;
    std::vector<struct packet_struct *> packetsInLink;
    bool isPacketNew;
    bool linkFree;

    //to handle multiple access.
    std::mutex *m_delay, *m_latency, *m_pkt;

  public:
    //SETS
    void setAll(int id,
                int idNode_src,
                int idNode_dst,
                int layer_src,
                int layer_dst,
                uint64_t throughput,
                int latency,
                int jitter,
                float error_rate,
                struct nck_timer *timer);
    void setId(int id);
    void setIdNode_src(int idNode_src);
    void setIdNode_dst(int idNode_dst);
    void setIdNodes(int idNode_src, int idNode_dst);
    void setLayer_src(int layer_src);
    void setLayer_dst(int layer_dst);
    void setLayers(int layer_src, int layer_dst);
    void setTroughput(uint64_t throughput);
    void setLatency(int latency);
    void setJitter(int jitter);
    void setError_rate(float er);
    void setTimer(struct nck_timer *timer);
    void setLinkCharacteristics(uint64_t throughput,
                                int latency,
                                int jitter,
                                float error_rate);
    struct packet_struct * createPacket();
    void putPacketInLink();

    //GETS
    int getId();
    int getIdNode_src();
    int getIdNode_dst();
    int getLayer_src();
    int getLayer_dst();
    uint64_t getThroughput();
    int getLatency();
    float getJitter();
    float getError_rate();
    struct nck_timer* getTimer();
    struct nck_timer_entry* getFreeLink_timer();
    struct packet_struct * getPktFromLink();
    bool isNewPkt();
    void consumedPkt();
    //void linksReady(struct nck_timer_entry *entry, void *link, int success);
    bool receivedSuccessfully();
    void free();
    //free link timers
    void free_link_timers(Link &link);

    void toString();
    std::string toStringLinkCharacteristics();
    void randomCharacteristics();
    float setRandomValueFloat(float min, float max);
    uint64_t setRandomValueBig(uint64_t min, uint64_t max);

    timeval totalDelay(unsigned len);
    timeval computeDelay(unsigned len);
    timeval computeRealLatency();
};

#endif //LINK_H
