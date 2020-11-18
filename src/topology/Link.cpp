#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <iostream>
#include <random>
#include <nckernel/timer.h>
#include <bits/types.h>
#include <nckernel/api.h>
#include <unistd.h>
#include <thread>
#include <bits/types/struct_timeval.h>

#include "Link.h"

using namespace std;

Link::Link(int id,
          int idNode_src,
          int idNode_dst,
          int layer_src,
          int layer_dst,
          uint64_t throughput,
          int latency, //RTT
          int jitter, //maybe double?
          float error_rate) {
  this->id = id;
  this->idNode_src = idNode_src;
  this->idNode_dst = idNode_dst;
  this->layer_src = layer_src;
  this->layer_dst = layer_dst;
  this->throughput = throughput;
  this->latency.tv_sec = latency / 1000;
  this->latency.tv_usec = (latency % 1000) * 1000;
  this->jitter = jitter;
  this->error_rate = error_rate;
  this->m_latency = new std::mutex;
  this->m_delay = new std::mutex;
  this->m_pkt = new std::mutex();
  this->isPacketNew = false;
  this->linkFree = true;
}
Link::Link() {
    this->throughput = 0;
    this->latency.tv_sec = 0;
    this->latency.tv_usec = 0;
    this->jitter = 0;
    this->error_rate = 0;
    this->m_latency = new std::mutex;
    this->m_delay = new std::mutex;
    this->m_pkt = new std::mutex;
    this->isPacketNew = false;
    this->linkFree = true;
}
Link::~Link() {
  //std::cout << "Link " << id << "is deleted " << '\n';
}
Link::Link(int id, int idNode_src, int idNode_dst, int layer_src, int layer_dst) {
    this->id = id;
    this->idNode_src = idNode_src;
    this->idNode_dst = idNode_dst;
    this->layer_src = layer_src;
    this->layer_dst = layer_dst;
    this->throughput = 0;
    this->latency.tv_sec = 0;
    this->latency.tv_usec = 0;
    this->jitter = 0;
    this->error_rate = 0;
    this->m_latency = new std::mutex;
    this->m_delay = new std::mutex;
    this->m_pkt = new std::mutex;
    this->isPacketNew = false;
    this->linkFree = true;
}



//***** SETS:
void Link::setAll(int id,
                  int idNode_src,
                  int idNode_dst,
                  int layer_src,
                  int layer_dst,
                  uint64_t throughput,
                  int latency,
                  int jitter,
                  float error_rate,
                  struct nck_timer *timer) {
    this->id = id;
    this->idNode_src = idNode_src;
    this->idNode_dst = idNode_dst;
    this->layer_src = layer_src;
    this->layer_dst = layer_dst;
    this->throughput = throughput;
    this->latency.tv_sec = latency / 1000;
    this->latency.tv_usec = (latency % 1000) * 1000;
    this->jitter = jitter;
    this->error_rate = error_rate;
    this->timer = timer;
}
void Link::setId(int id) {
  this->id = id;
}
void Link::setIdNode_src(int idNode_src) {
  this->idNode_src = idNode_src;
}
void Link::setIdNode_dst(int idNode_dst) {
  this->idNode_dst = idNode_dst;
}
void Link::setIdNodes(int idNode_src, int idNode_dst) {
  setIdNode_src(idNode_src);
  setIdNode_dst(idNode_dst);
}
void Link::setLayer_src(int layer_src) {
  this->layer_src = layer_src;
}
void Link::setLayer_dst(int layer_dst) {
  this->layer_dst = layer_dst;
}
void Link::setLayers(int layer_src, int layer_dst) {
  setLayer_src(layer_src);
  setLayer_dst(layer_dst);
}
void Link::setTroughput(uint64_t throughput) {
  this->throughput = throughput;
}
void Link::setLatency(int latency) {
  this->latency.tv_sec = latency / 1000;
  this->latency.tv_usec = (latency % 1000) * 1000;
}
void Link::setJitter(int jitter) {
  this->jitter = jitter;
}
void Link::setError_rate(float error_rate) {
  this->error_rate = error_rate;
}
void Link::setTimer(struct nck_timer *timer) {
  this->timer = timer;
}
void Link::setLinkCharacteristics(uint64_t throughput,
                                  int latency,
                                  int jitter,
                                  float error_rate) {
    this->throughput = throughput; //in bps
    this->latency.tv_sec = latency / 1000;
    this->latency.tv_usec = (latency % 1000) * 1000;
    this->jitter = jitter;
    this->error_rate = error_rate;
}

struct packet_struct * Link::createPacket() {
    std::lock_guard<std::mutex> lock_guard(*this->m_pkt);
    this->packetsInLink.push_back(new struct packet_struct());
    return packetsInLink[packetsInLink.size() - 1];
}

//TODO: DELETE
void Link::putPacketInLink() {
  std::lock_guard<std::mutex> lock_guard(*this->m_pkt);
  this->isPacketNew = true;
}

//**********GETS

int Link::getId() {
  return id;
}
int Link::getIdNode_src() {
  return idNode_src;
}
int Link::getIdNode_dst() {
  return idNode_dst;
}
int Link::getLayer_src() {
  return layer_src;
}
int Link::getLayer_dst() {
  return layer_dst;
}
uint64_t Link::getThroughput() {
  return throughput;
}
int Link::getLatency() {
  return (latency.tv_sec * 1000) + (latency.tv_usec / 1000);
}
float Link::getJitter() {
  return jitter;
}
float Link::getError_rate() {
  return error_rate;
}
struct nck_timer* Link::getTimer() {
    return timer;
}
struct nck_timer_entry* Link::getFreeLink_timer() {
  return freeLink_timer;
}

struct packet_struct * Link::getPktFromLink() {
    std::lock_guard<std::mutex> lock_guard(*this->m_pkt);
    if (packetsInLink.size() == 0) {
        return nullptr;
    }
    this->packet = packetsInLink[0];
    packetsInLink.erase(packetsInLink.begin());
    return packet;
}

bool Link::isNewPkt() {
    std::lock_guard<std::mutex> lock_guard(*this->m_pkt);
    //this->isPacketNew;
    if (packetsInLink.empty()) {
        return false;
    }
    else if (packetsInLink.size() == 1){
        return isPacketNew;
    }
    else {
        return true;
    }
}

void Link::consumedPkt() {
    std::lock_guard<std::mutex> lock_guard(*this->m_pkt);
    this->isPacketNew = false;
}

bool Link::receivedSuccessfully() {
  //std::default_random_engine generator;
  std::mt19937 generator(std::random_device{}());
  std::bernoulli_distribution distribution(1 - error_rate);
  return distribution(generator);
}

void Link::toString() {
  std::cout << "LINK " << id << std::endl;
  std::cout << "  Node" << idNode_src << " ------------------------> Node" << idNode_dst << std::endl;
  std::cout << "  Layer" << layer_src << " ------------------------> Layer" << layer_dst << endl;
  std::cout << "      * throughput: " << throughput << endl;
  std::cout << "      * latency: " << (latency.tv_usec / 1000 ) + (latency.tv_sec * 1000) << endl;
  std::cout << "      * jitter: " << jitter << endl;
  std::cout << "      * error_rate: " << error_rate << endl;
}

std::string Link::toStringLinkCharacteristics() {
    int latency_ms = (latency.tv_usec / 1000 ) + (latency.tv_sec * 1000);
  std::string str = "LINK " + to_string(id) +
          "\n                 * throughput: " + to_string(throughput) +
          "\n                 * latency: " + to_string(latency_ms) +
          "\n                 * jitter: " + to_string(jitter) +
          "\n                 * error_rate: " + to_string(error_rate);
  return str;
}

void Link::randomCharacteristics() {
  throughput =  setRandomValueBig(50e3, 100e6);
  setLatency(setRandomValueBig(0, 300));
  //latency =  setRandomValueFloat(0, 0.3);
  jitter =  setRandomValueBig(0, 100);
  error_rate = setRandomValueFloat (0, 0);
}

uint64_t Link::setRandomValueBig(uint64_t min, uint64_t max) {
  srand(time(NULL));
  return rand() % (max - min) + min;
}
float Link::setRandomValueFloat(float min, float max) {
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(min, max);
  return dis(gen);
}

void Link::free() {
    this->linkFree = true;
}

void Link::free_link_timers(Link &link) {
  nck_timer_cancel(link.freeLink_timer);
  nck_timer_free(link.freeLink_timer);
}
/*
static void freeLink(struct nck_timer_entry *entry, void *context, int success) {
    (void)entry; // unused parameter
    Link *l = (Link *) context;
    if (success) {
        l->free();
    }
}*/

timeval Link::totalDelay(unsigned len) {
    timeval res_delay;
    uint64_t bits = (uint64_t) len*8;
    res_delay.tv_sec = bits / throughput;
    res_delay.tv_usec = (bits*1000000) / throughput;

    double actual_jitter = jitter*((drand48()*2 - 1));
    if (actual_jitter == 0){
        res_delay.tv_sec += latency.tv_sec;
        res_delay.tv_usec += latency.tv_usec;
    } else {
        res_delay.tv_sec += latency.tv_sec + (jitter / 1000);
        res_delay.tv_usec += latency.tv_usec + ((jitter - (double) res_delay.tv_sec * 1000) * 1000);
    }
    res_delay.tv_usec += packetsInLink.size()*100;
    return res_delay;
}

timeval Link::computeDelay(unsigned len) {
  m_delay->lock();

  //len is in Bytes and throughput in bps
  uint64_t bits = (uint64_t) len*8;
  delay.tv_sec = bits / throughput;
  delay.tv_usec = (bits*1000000) / throughput;

  m_delay->unlock();
  return delay;
}

timeval Link::computeRealLatency() {
  m_latency->lock();

  double actual_jitter = jitter*((drand48()*2 - 1));
  if (actual_jitter == 0){
    real_latency = latency;
  } else {
      real_latency.tv_sec = latency.tv_sec + (jitter / 1000);
      real_latency.tv_usec = latency.tv_usec + ((jitter - (double) real_latency.tv_sec * 1000) * 1000);
  }

  m_latency->unlock();
  return real_latency;
}