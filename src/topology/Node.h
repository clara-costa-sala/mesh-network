#ifndef NODE_H
#define NODE_H

#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <algorithm>
#include <iostream>
//#include <thread>
#include <stdbool.h>
#include <nckernel/nckernel.h>
#include <nckernel/skb.h>
#include <nckernel/timer.h>
#include <map>
#include <mutex>



using namespace std;

class Network;
class Link;

class Node{
public:
  Node();
  Node(int id,
       int layer_id,
       vector<int> links);
  Node(int id, int layer_id);

  Node (const Node &other);


  struct nck_coder coder;

private:
  int id;
  int layer_id;
  vector<int> links;
  map<const int, Link *> linksp;
  vector<Link *> recvFromLinksp;
  vector<Node *> nodesToSend;
  struct nck_schedule *schedule;
  struct timeval *step;
  struct nck_timer *timer;
  Network *network;
  int type; //0: src, 1: node, 2: dst
  int c;
  //int noMorePackets;
  int pkts_sent;
  int pkts_recvd;
  int counter;
  bool ready;
  uint32_t src_pkt;
  uint32_t decoded;
  bool running;
  std::mutex *m_running;
  std::mutex m_link;
    std::mutex m_ready;
  std::vector<struct recv_struct *> receives;


public:
  //SETS
  void setAll(int id,
              int layer_id,
              vector<int> links);
  void setLayer_id(int layer_id);
  void setId(int id);
  void setLinks(vector<int> links);
  void setLinksp(map<const int, Link *> linksp);
  //void setReady(int ready);
  void setNodesToSend(vector<Node *> nodesToSend);
  void setNetwork(Network *network);
  void setSchedule(struct nck_schedule *schedule);
  void setStep(struct timeval *step);
  void setType(int type);
  void setRecvFromLinksp(vector<Link *> recvLinkV);
  void setNotReady();
  void setReady();

  /**
   * createEncoderInNode() - Creates an encoder and initializes it.
   * @options_enc: Contextual object that will be passed to get_opt
   * @return: Returns 0 on success
   */
  int createEncoderInNode(struct nck_option_value options_enc[], nck_timer *timer);
  /**
   * createDecoderInNode() - Creates an decoder and initializes it.
   * @options_dec: Contextual object that will be passed to get_opt
   * @return: Returns 0 on success
   */
  int createDecoderInNode(struct nck_option_value options_dec[], nck_timer *timer);
  /**
   * createRecoderInNode() - Creates an recoder and initializes it.
   * @options_rec: Contextual object that will be passed to get_opt
   * @return: Returns 0 on success
   */
  int createRecoderInNode(struct nck_option_value options_rec[], nck_timer *timer);

  //GETS
  int getLayer_id();
  int getId();
  vector<int> getLinks();
  vector<Node *> getNodesToSend();
  struct nck_coder getCoder();
  int getType();
  vector<Link *> getRecvFromLinksp();
  struct recv_struct * createRcvStruct();
  void deleteRcvStruct(int id);
  bool isRunning();
  bool isReady();

  //static void receive(struct nck_timer_entry *handle, void *contect, int success);
  /**
   * removeRcvLink: remove the link from the receiving links list
   * @param idLink : identifier of the link to delete
   */
  void removeRecvLink(int idLink);
  /**
   * removeLinkId: removes the link from the list of the link's identifier.
   * @param idLink : identifier of the link to delete.
   */
  void removeLinkId(int idLink);

  /**
   * removeNode: remove nodeToSend from the list NodesToSend
   * @param nodeToSend : the Node that needs to be removed
   */
  //void removeNode(shared_ptr<Node> nodeToSend);
  void removeNode(Node * nodeToSend);

  /**
   * addLink: adds the identifier of a link in the links list
   * @param idLink : the ifentifier of the link to add
   */
  void addLink(int idLink);

  /**
   * addLinkp: adds a pointer of a link in the map linksp
   * @param idLink : the ifentifier of the link
   * @param link : the pointer of the link
   */
  void addLinkp(const int idLink, Link * link);

  /**
   * addLinkpToRecvLink: adds the pointer of a link in the receiveLink vector
   * @param link: the pointer of the link to add
   */
  void addLinkpToRecvLink(Link *link);

  /**
   * checkEquals: check if the node that is passing as a parameter is equal to "this" node
   * @param node: node pointer that needs to be checked
   * @return : true if they are equal or false if they are not.
   */
  bool checkEqualsP(Node *node);

  /**
   * toString: strings the Node characterisitics in order to print it
   */
  std::string toString();

  void registerReceived(int idSrc, int idDst, float duration, int length);

  /**
   * checkIdLinksExists: when a new link is added, we need to know if this link already exists
   * @param idLink: idetifier of the new link that needs to be checked
   * @return: true if it exists, false in the other case
   */
  bool checkIfLinkExists(int idLink);

  bool linksAreFree();

  bool areEquals(vector<int> v1, vector<int> v2);

  /**
   * receivePkt: gets the packet saved in the link and decodes it
   * @param link : pointer of the link where the packet is
   * @return : number of the source node that sends it / if the receiving has not ben successfulluy
   *            done, then returns 0.
   */
  //  int receivePkt(Link *link);

    void decodePkt();
    int decodedPkts();
    void receivedPkt();


  /**
   * encodeStcPackets: encodes the source packets that is in the payload
   * @pkt: packet number we are encoding
   * @payload*: pointer of the source packet
   */
    bool encodeSrcPkt();

    //void sendFinishPkts();
    bool sendCodedPkt();

    void writeRes(string txt);
    void writeLog(string txt);

    void * run(int num_pkts, struct nck_schedule *schedule, struct nck_timer *timer);

    void start();
};
#endif
