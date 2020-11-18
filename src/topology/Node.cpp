#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <stdbool.h>
#include <cstring>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <assert.h>
#include <getopt.h>
#include <nckernel/nckernel.h>
#include <nckernel/api.h>
#include <nckernel/skb.h>
#include <nckernel/timer.h>
#include <thread>

#include "Node.h"
#include "Link.h"
#include "Network.h"

using namespace std;

Node::Node(){
    this->c = 1;
    //this->noMorePackets = 0;
    this->running = true;
    this->pkts_sent = 0;
    this->pkts_recvd = 0;
    this->m_running = new std::mutex;
    this->src_pkt = 0;
    this->decoded = 0;
    this->counter = 0;
    this->ready = true;
}

Node::Node(int id,
           int layer_id,
           vector<int> links)
 : id(id), layer_id(layer_id), links(links) {
    this->c = 1;
    //this->noMorePackets = 0;
    this->pkts_recvd = 0;
    this->pkts_sent = 0;
    this->running = true;
    this->m_running = new std::mutex;
    this->src_pkt = 0;
    this->decoded = 0;
    this->counter = 0;
    this->ready = true;
}

Node::Node(int id, int layer_id) {
    this->id = id;
    this->layer_id = layer_id;
    //this->noMorePackets = 0;
    this->pkts_sent = 0;
    this->pkts_recvd = 0;
    this->running = true;
    this->m_running = new std::mutex;
    this->src_pkt = 0;
    this->decoded = 0;
    this->counter = 0;
    this->c = 1;
    this->ready = true;
}

Node::Node(const Node &other) {
    this->id = other.id;
    this->layer_id = other.id;
    this->c = 1;
    //this->noMorePackets = 0;
    this->pkts_sent = 0;
    this->pkts_recvd = 0;
    this->running = true;
    this->m_running = new std::mutex;
    this->src_pkt = 0;
    this->decoded = 0;
    this->counter = 0;
    this->ready = true;
}





//SETS
void Node::setAll(int id, int layer_id, vector<int> links) {
  this->id = id;
  this->layer_id = layer_id;
  this->links = links;
}

void Node::setId(int id) {
  this->id = id;
}

void Node::setLayer_id(int layer_id) {
  this->layer_id = layer_id;
}

void Node::setLinks(vector<int> links) {
  this->links = std::move(links);
}

void Node::setLinksp(map<const int, Link *> linksp) {
  this->linksp = std::move(linksp);
}

void Node::setNodesToSend(vector<Node *> nodesToSend) {
  this->nodesToSend = std::move(nodesToSend);
}

void Node::setNetwork(Network *network) {
  this->network = network;
}

void Node::setSchedule(struct nck_schedule *schedule) {
    this->schedule = schedule;
}

void Node::setStep(struct timeval *step) {
    this->step = step;
}

void Node::setType(int type) {
  this->type = type;
}

void Node::setRecvFromLinksp(vector<Link *> recvLinkV) {
  this->recvFromLinksp = std::move(recvLinkV);
}

void Node::setNotReady() {
    std::lock_guard<std::mutex> lock_guard(this->m_ready);
    this->ready = false;
}
void Node::setReady() {
    std::lock_guard<std::mutex> lock_guard(this->m_ready);
    this->ready = true;
}


/**
 * createEncoderInNode() - Creates an encoder and initializes it.
 * @options_enc: Contextual object that will be passed to get_opt
 * @return: Returns 0 on success
 */
 int Node::createEncoderInNode(struct nck_option_value options_enc[], nck_timer *timer) {
    if (nck_create_coder(&coder, NCK_ENCODER, timer, options_enc, nck_option_from_array)) {
      network->writeLog( "ERROR >> NODE " + to_string(this->id) + " >> failed to create encoder");
      return -1;
    }
    network->writeLog( "NODE " + to_string(this->id) + " >> encoder created successfully ");
    cout << endl << "SRC - CODED SIZE = " << this->coder.coded_size << endl;
    setType(0);
    this->timer = timer;
    return 1;
 }

/**
* createDecoderInNode() - Creates an decoder and initializes it.
* @options_dec: Contextual object that will be passed to get_opt
* @return: Returns 0 on success
*/
int Node::createDecoderInNode(struct nck_option_value options_dec[], nck_timer *timer) {
  if (nck_create_coder(&coder, NCK_DECODER, timer, options_dec, nck_option_from_array)) {
      network->writeLog( "ERROR >> NODE " + to_string(this->id) + " >> failed to create decoder");
      return -1;
  }
  network->writeLog( "NODE " + to_string(this->id) + " >> decoder created successfully ");
  setType(2);
  this->timer = timer;
  return 1;
}

/**
 * createRecoderInNode() - Creates an recoder and initializes it.
 * @options_rec: Contextual object that will be passed to get_opt
 * @return: Returns 0 on success
 */
int Node::createRecoderInNode(struct nck_option_value options_rec[], nck_timer *timer) {
  if (nck_create_coder(&coder, NCK_RECODER, timer, options_rec, nck_option_from_array)) {
      network->writeLog( "ERROR >> NODE " + to_string(this->id) + " >> failed to create recoder");
      return -1;
  }
  network->writeLog( "NODE " + to_string(this->id) + " >> recoder created successfully ");
  cout << endl << "REC - CODED SIZE = " << this->coder.coded_size << endl;
  setType(1);
  this->timer = timer;
  return 1;
}



//GETS;
int Node::getLayer_id() {
  return layer_id;
}

int Node::getId() {
  return id;
}

vector<int> Node::getLinks() {
  return links;
}

struct nck_coder  Node::getCoder() {
  return coder;
}

vector<Node *> Node::getNodesToSend() {
  return nodesToSend;
}

int Node::getType() {
  return type;
}

vector<Link *> Node::getRecvFromLinksp() {
  return recvFromLinksp;
}

struct recv_struct * Node::createRcvStruct() {
    std::lock_guard<std::mutex> lock_guard(this->m_link);
    receives.push_back(new struct recv_struct());
    this->counter ++;
    receives[receives.size() - 1]->id = this->counter;
    return receives[receives.size() - 1];
}


void Node::deleteRcvStruct(int id) {
    std::lock_guard<std::mutex> lock_guard(this->m_link);
    for (int it = 0; it < receives.size(); it++) {
        if (receives[it]->id == id) {
            receives.erase(receives.begin() + it);
            break;
        }
    }
}

bool Node::isRunning() {
    m_running->lock();
    bool run = this->running;
    m_running->unlock();
    return run;
};

bool Node::isReady() {
    std::lock_guard<std::mutex> lock_guard(this->m_ready);
    return ready;
}

void Node::addLink(int idLink) {
  if(!checkIfLinkExists(idLink)){
    links.push_back(idLink);
  }
  else {
    std::cout << "Link with id = '" << idLink << "' can't be added to Node " << id << std::endl;
  }
}

void Node::addLinkp(const int idLink, Link *link) {
  linksp.insert(std::pair<const int, Link *>(idLink, link));
}

void Node::addLinkpToRecvLink(Link *link) {
  recvFromLinksp.push_back(link);
}

void Node::removeRecvLink(int idLink) {
  bool found = false;
  int i;
  for (i = 0; i < (int) recvFromLinksp.size() || !found; i++) {
    if (recvFromLinksp[i]->getId() == idLink) {
      found = true;
      break;
    }
  }
  if(found) {
    recvFromLinksp.erase(recvFromLinksp.begin() + i);
    network->writeLog("LINK " + to_string(idLink) + "has been deleted from Node " + to_string(id) );
    removeLinkId(idLink);
  } else {
    network->writeLog( "NODE >> removeRcvLink >> Link " + to_string(idLink) + " is not in the Node\n" );
  }
}

void Node::removeLinkId(int idLink) {
    auto it = std::find(links.begin(), links.end(), idLink);
    if (it != links.end()) {
        links.erase(it);
    } else {
        network->writeLog( "NODE >> removeLinkId >> Element Not Found - linkid = " + to_string(idLink) );
    }
}

void Node::removeNode(Node *nodeToSend) {
  int i;
  for (i = 0; i < (int) nodesToSend.size(); i ++) {
      if (nodesToSend[i]->getId() == nodeToSend->getId()) {
          nodesToSend.erase(nodesToSend.begin() + i);
          break;
      }
  }
  //node has been deleted, now need to delete also the link connecting to the node.
  for (std::map<int,Link *>::iterator it = linksp.begin(); it != linksp.end(); ++it) {
      //find the link that connects with this node
      if (it->second->getIdNode_src() == this->id && it->second->getIdNode_dst() == nodeToSend->getId()) {
          removeLinkId(it->second->getId());
          linksp.erase(it);
          break;
      }
  }
}

bool Node::checkEqualsP(Node *node) {
  return (node->getId() == id && node->getLayer_id() == layer_id && areEquals(node->getLinks(), links));
}

std::string Node::toString() {
    std::string s = "\n****\nNode: " + to_string(id) + " - Layer " + to_string(layer_id);
    if (!links.empty() ) {
        s += " - Links: ";
        for (int it = 0; it < (int)links.size(); it++) {
            s += to_string(links[it]) + "; ";
        }
        s += "\n";
    }
    else{
    s += "\n     - No links associated\n";
    }
    return s;
}

void Node::registerReceived(int idSrc, int idDst, float duration, int length) {
    network->writeRes( to_string(idSrc) + " - " + to_string(id)  + " "
    + " " + to_string(duration) + " " + to_string(length));
}

bool Node::checkIfLinkExists(int idLink) {
  vector<int>::iterator it = std::find(links.begin(), links.end(), idLink);
  return (!(it == links.end()));
}

bool Node::linksAreFree() {
    //int counter = 0;
    for (auto it = begin(nodesToSend); it != end(nodesToSend); it ++) {
        for ( auto j = linksp.begin(); j != linksp.end(); j++ ) {
            if ((*it)->getId() == (j->second)->getIdNode_dst() &&
                this->id == (j->second)->getIdNode_src()) {
                while ((j->second->isNewPkt())) {}
                //wait until the next nodes take the packets
            }
        }
    }
    return true;
}

bool Node::areEquals(vector<int> v1, vector<int> v2) {
  std::sort(v1.begin(), v1.end());
  std::sort(v2.begin(), v2.end());
  return v1 == v2;
}
/*
//Todo: do i need struct nck_timer_entry *handle??
 int Node::receivePkt(Link *link) {
    struct packet_struct *pkt = link->getPktFromLink();
    if( pkt->success != 0) {
        if (!pkt->lastPkt) {
            network->writeLog("NODE " + to_string(id) + " >> receivePkt >> PACKET SUCCESSFULLY RECEIVED FROM LINK " +
                              to_string(link->getId()));
            struct timeval receive_time;
            gettimeofday(&receive_time, NULL);

            float duration = (receive_time.tv_sec - pkt->send_time.tv_sec) * 1000000L +
                             (receive_time.tv_usec - pkt.send_time.tv_usec);

            network->writeRes(to_string(link->getIdNode_src()) + " - " + to_string(id) + " " + to_string(duration) + " " + to_string(pkt->packet.len));

            struct sk_buff *skb = new struct sk_buff;
            uint8_t buffer[coder.coded_size];
            memset(buffer, 0, coder.coded_size);
            skb_new(skb, buffer, coder.coded_size);
            skb_put(skb, coder.coded_size);
            memcpy(skb->data, pkt.packet.data, coder.coded_size);

            this->pkts_recvd ++;
            nck_put_coded(&coder, skb);
        }
        else { //is last packet
            link->consumedPkt();
            this->noMorePackets ++;
            network->writeLog( "NODE " + to_string(id) + " does NOT expect more packets");
        }
        link->consumedPkt();
        return 1;
    }
    else {
        network->writeLog(
                "NODE " + to_string(id) + " >> receivePkt >> LOST PACKET IN LINK " + to_string(link->getId()));
        return 0;
    }
}
 */

 static void receive(struct nck_timer_entry *h, void *context, int success) {
    struct recv_struct *rcv = (struct recv_struct *) context;
    Node *node = rcv->node;
    Link *link = rcv->link;
    while (!node->isReady()) {} //Wait
    node->setNotReady();

    if (link->isNewPkt()) {
        struct packet_struct *pkt = link->getPktFromLink();

        if (pkt->success != 0 && pkt != nullptr) {
            node->writeLog("NODE " + to_string(node->getId()) + " >> receivePkt >> PACKET SUCCESSFULLY RECEIVED FROM LINK " + to_string(link->getId()));
            node->writeRes(to_string(link->getIdNode_src()) + " - " + to_string(node->getId()) + " " + to_string(0) + " " + to_string(pkt->packet.len));
            node->receivedPkt();

            nck_put_coded(&node->coder, &(pkt->packet));
            //link->consumedPkt();    //free link

            if (h) {
                nck_timer_free(h);
            }

            if (node->getType() == 1) {
                node->sendCodedPkt(); //RECODER
            }
            else if (node->getType() == 2) {
                node->decodePkt();
            }
        }
        //link->consumedPkt();
    }
    node->deleteRcvStruct(rcv->id);
    node->setReady();
}

void Node::decodePkt() {
    while(nck_has_source(&coder)) {
        struct sk_buff *packet_recv = new struct sk_buff();
        uint8_t buffer_recv[coder.source_size];
        memset(buffer_recv, 0, sizeof(uint8_t) * coder.source_size);
        skb_new(packet_recv, buffer_recv, coder.source_size);
        if (!nck_get_source(&coder, packet_recv)) {
            this->decoded ++;
            network->writeLog("Node " + to_string(id) + " >> Decoded pkts: " + to_string(decoded));
            uint32_t seq = skb_pull_u32(packet_recv);
            network->writeLog("DEST NODE " + to_string(id) + " decode pkt = " + to_string(this->decoded) + " with: \n seq: "
                              + to_string(seq));
        }
    }
}

int Node::decodedPkts() {
    return decoded;
}

void Node::receivedPkt() {
    this->pkts_recvd ++;
}

bool Node::encodeSrcPkt() {
    nck_schedule_run(this->schedule, this->step);
    uint8_t payload[coder.source_size - sizeof(uint32_t)];
    for (uint32_t j = 0; j < coder.source_size - sizeof(uint32_t); j++) {
        payload[j] = (uint8_t) rand();
    }
    struct sk_buff packet;
    uint8_t buffer[coder.source_size];
    skb_new(&packet, buffer, coder.source_size);
    skb_reserve(&packet, sizeof(uint32_t));
    //skb_put(packet, coder.source_size - sizeof(uint32_t));
    skb_put(&packet, skb_tailroom(&packet));
    memcpy(packet.data, payload, coder.source_size - sizeof(uint32_t));
    this->src_pkt ++;
    network->writeLog("NODE " + to_string(id) + " >> coded src = " + to_string (src_pkt));
    skb_push_u32(&packet, this->src_pkt);
    while (nck_full(&coder)) {
        sendCodedPkt();
    }
    if (nck_put_source(&coder, &packet)) {
        cout << "PACKETS COULD NOT BE CODED " << endl;
    };
    return sendCodedPkt();
}
/*
void Node::sendFinishPkts() {
    for (auto it = begin(nodesToSend); it != end(nodesToSend); it ++) {
        for ( auto j = linksp.begin(); j != linksp.end(); j++ ) {
            if ((*it)->getId() == (j->second)->getIdNode_dst() && id == (j->second)->getIdNode_src()) {
                struct packet_struct *pkt = j->second->createPacket();
                pkt->lastPkt = true;
                pkt->success = true;
                while ((j->second)->isNewPkt()) {}  //wait....
                (j->second)->putPacketInLink();
            }
        }
    }
}
 */

bool Node::sendCodedPkt() {
    nck_schedule_run(this->schedule, this->step);
    uint32_t  pkt_num = this->src_pkt;
    bool isLink = false;
    int num_sent = 0;
    //CODE PACKET
    while(nck_has_coded(&coder)) {
        if (!nodesToSend.empty()){
            int dest_node = (this->c) % nodesToSend.size();

            //SEND PKT
            Link *l;
            auto it = linksp.begin();
            bool isLink = false;
            while(it != linksp.end() && !isLink) {       // Iterate till the proper link is found.
                l = it->second;
                if (l->getIdNode_dst() == nodesToSend[dest_node]->getId() && l->getIdNode_src() == this->id) {
                   //while (l->isNewPkt()) {
                   //     nck_schedule_run(this->schedule, this->step);
                   // } //wait till the link is available
                    //struct timeval start_sending_time;
                    //gettimeofday(&start_sending_time, NULL);
                    //pkt->send_time = start_sending_time;

                    //GET CODED PACKET
                    uint8_t buffer[coder.coded_size];
                    memset(buffer, 0, coder.coded_size);
                    struct packet_struct *pkt = l->createPacket();
                    skb_new(&(pkt->packet), buffer, coder.coded_size);
                    nck_get_coded(&coder, &(pkt->packet));

                    pkt->success = l->receivedSuccessfully();
                    pkt->pkt_num = (uint32_t) pkt_num;
                    struct timeval delay = l->totalDelay(pkt->packet.len); // compute delay + jitter


                    l->putPacketInLink();
                    //now there is a packet in the link and the next node can receive it when delay time passes.
                    timeradd(&(this->schedule->time), &delay, &(pkt->receive_time));
                    struct recv_struct *rcv = nodesToSend[dest_node]->createRcvStruct();
                    rcv->node = nodesToSend[dest_node];
                    rcv->link = l;

                    nck_timer_add(this->timer, &delay, rcv, &receive);

                    this->c ++;
                    this->pkts_sent ++;
                    isLink = true;
                    num_sent ++;

                    network->writeLog( "NODE " + to_string(id) + " >> sendCodedPkt >> sending to node " + to_string(nodesToSend[dest_node]->getId()) + " packet " + to_string(this->pkts_sent));
                }
            it ++;
            }
            if (!isLink) network->writeLog("NODE " + to_string(id) + " >> sendCodedPkt >> could not find the link ");
            nck_schedule_run(this->schedule, this->step);
        }
    }
    if (num_sent == 0) return num_sent;
    else return true;
}

void Node::writeRes(string txt) {
    network->writeRes(txt);
}

void Node::writeLog(string txt) {
    network->writeLog(txt);
}


/*

void * Node::run(int num_pkts, struct nck_schedule *schedule, struct nck_timer *timer) {
    m_running->lock();
    this->running = true;
    m_running->unlock();

    network->writeLog("NODE >> NODE " + to_string(id) + " is RUNNING ...");
    while (!network->allNodesRunning()) {} // wait

    if (getType() == 0) { //is source -> encode
        cout << endl << "NODE SRC SOURCE SIZE " << coder.source_size  << " CODE : " << coder.coded_size << endl;
        network->writeLog("NODE >> NODE " + to_string(id) + " is SOURCE ...");

        srand48(time(NULL));
        struct nck_schedule sched;
        struct nck_timer t;
        //struct timeval step, timeout_bf;
        //struct sk_buff *packet_recv = new struct sk_buff;

        nck_schedule_init(&sched);
        nck_schedule_timer(&sched, &t);

        //TODO: i don't knwo if i should use the schedule of the main or create a new one

        uint32_t pkt_num = 0;
        size_t src_size = coder.source_size;
        bool keepCoding = true;
        while( pkt_num < (uint32_t) num_pkts && keepCoding) {

             if(nck_schedule_run(&sched, &step)) {
                //STOP
            }
            //source node encodes the packet
            network->writeLog( "NODE " + to_string(id) + " >> source packet " + to_string(pkt_num));

            encodeSrcPkt();
            sendCodedPkt();

            pkt_num++;
            if (pkt_num == num_pkts ) {
                //nck_flush_source(&coder);
                //sendCodedPkt(pkt_num);
                sendFinishPkts();
                if (linksAreFree()) keepCoding = false;
            }
            //}
            //TODO :
            //timeradd(&sched.time, &step, &sched.time);

        }
        //nck_flush_coded(&coder);
        //sendCodedPkt(num);
        network->writeLog( "NODE " + to_string(id) + " >> HAS SENT ALL SOURCE PACKETS = " + to_string(this->pkts_sent));
        sendFinishPkts();
        //while (!network->isTransmissionFinished()){}
    }
    else if (getType() == 1) {
        network->writeLog("NODE >> NODE " + to_string(id) + " is RECODER ...");
        cout << endl << "SOURCE SIZE " << coder.source_size  << " CODE : " << coder.coded_size << endl;

        int i = 0;
        int num = 0;
        noMorePackets = 0;
        bool keepRunning = true;
        while (keepRunning) {
            if (noMorePackets < recvFromLinksp.size()) {
               //take all packets from the links
               //iterate throughout each link to see if there are pkt to receive:
                while (!recvFromLinksp[i]->isNewPkt()){
                    i = (i + 1) % (int) recvFromLinksp.size();
                }
                // This node is the only one who can receive from this link -> no mutex here
                //shared_ptr<Link> link = recvFromLinksp[i];
                Link *link = recvFromLinksp[i];
                if (receivePkt(link)) {
                    if (sendCodedPkt()) this->src_pkt ++;
                }
                if (network->isTransmissionFinished()) {
                    keepRunning = false;
                }
                i = (i + 1) % (int)recvFromLinksp.size(); // increment so we check another link.
            }
            else {
                for (auto it = begin(recvFromLinksp); it != end(recvFromLinksp); it ++) {
                    if ((*it)->isNewPkt()) {
                        if (receivePkt(*it)) {
                            sendCodedPkt();
                        }
                    }
                }
                keepRunning = false;
            }
        }
        //nck_flush_coded(&coder);
        network->writeLog( "NODE " + to_string(id) + " >> sendCodedPkt >> sending last packet -> no more packets are expected");
        sendFinishPkts();
    }
    else if (getType() == 2){ //WE NEED TO receive and decode
        network->writeLog("NODE >> NODE " + to_string(id) + " is DECODER ...");
        int i = 0;
        int num_sources = network->getNumSources();

        //todo: nck_schedule_run(&sched, &step);
        int j = 0;
        //TODO: also while (I won't receive more packets.)
        while (i < num_pkts*num_sources && noMorePackets < recvFromLinksp.size()) {
            while (!recvFromLinksp[j]->isNewPkt()) {
                j = (j + 1) % recvFromLinksp.size();
            }
            //shared_ptr<Link> link = recvFromLinksp[j];
            Link *link = recvFromLinksp[j];

            //TODO: Decode pkt.
            if (receivePkt(link) ) {
                while(nck_has_source(&coder)) {
                    //todo ----
                    struct sk_buff *packet_recv = new struct sk_buff();
                    uint8_t buffer_recv[coder.source_size];
                    memset(buffer_recv, 0, sizeof(uint8_t) * coder.source_size);
                    skb_new(packet_recv, buffer_recv, coder.source_size);
                    nck_get_source(&coder, packet_recv);

                    //TODO: GRT PKT NUMBER
                    uint32_t seq = skb_pull_u32(packet_recv);
                    //uint32_t gen = skb_pull_u32(pakt);
                    //uint16_t rank = skb_pull_u16(pakt);
                    network->writeLog( "DEST NODE " + to_string(id) + " decode pkt = " + to_string(i) + " with: \n seq: "
                                       + to_string(seq)); // + "\n gen : " + to_string(gen) + "\n rank : " + to_string(rank));

                    i ++;
                }
            }
            j = (j + 1) % recvFromLinksp.size();
        }
        //nck_flush_source(&coder);
        //network->writeLog( "DEST NODE " + to_string(id) + " >> FLUSHING ...");

        while (nck_has_source(&coder)) {
            struct sk_buff *pakt = new struct sk_buff();
            uint8_t buffer_recv[coder.source_size];
            memset(buffer_recv, 0, coder.source_size);
            skb_new(pakt, buffer_recv, coder.source_size);
            nck_get_source(&coder, pakt);
            uint32_t seq = skb_pull_u32(pakt);
            network->writeLog( "DEST NODE " + to_string(id) + " decode pkt = " + to_string(i) + " with: \n seq: "
                               + to_string(seq));

        }
        network->writeLog( "NODE DEST " + to_string(id) + " RECEIVED " + to_string(pkts_recvd) + " PACKETS" );
        network->finishTransmission();
    }
    //NODE SOURCES SHOULD WAIT TILL THE PACKET FROM THE LINKS ARE OUT
    if (!network->isTransmissionFinished() && type != 2) {
        linksAreFree();
    }
    nck_free(&coder);
    network->writeLog( "NODE " + to_string(id) + " >> STOPPED RUNNING ...");
    m_running->lock();
    this->running = false;
    m_running->unlock();
    network->nodesStoppedRunning(); //increment nodes that sopped runnning:
}
*/