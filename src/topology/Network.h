#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdbool.h>
#include <fstream>
#include <memory>
#include "Link.h"
#include "Node.h"

using namespace std;

struct packet_struct {
    struct sk_buff packet;
    struct nck_schedule *schedule;
    timeval send_time;
    timeval receive_time;
    uint32_t pkt_num;
    bool lastPkt;
    int success;

    packet_struct(struct sk_buff packet, struct nck_schedule *schedule, timeval send_time,
            timeval receive_time, uint32_t pkt_num, int success)
            : packet(packet), schedule(schedule),
            send_time(send_time), receive_time(receive_time), pkt_num(pkt_num), lastPkt(lastPkt), success(success) {}
    packet_struct(){}
    packet_struct(const packet_struct& p) : packet(p.packet), schedule(p.schedule), send_time(p.send_time),
            receive_time(p.receive_time), pkt_num(p.pkt_num), success(p.success) {}
    packet_struct(packet_struct &&) {}
    packet_struct& operator = (packet_struct p) {
      struct sk_buff packet = p.packet;
      struct nck_schedule *schedule = p.schedule;
      timeval send_time = p.send_time;
      timeval receive_time = p.receive_time;
      uint32_t pkt_num = p.pkt_num;
      int success = p.success;
      return *this;
    }
};

struct recv_struct {
    Link *link;
    Node *node;
    int id;
    recv_struct(){}
    recv_struct(recv_struct &&) {}
    recv_struct& operator = (recv_struct r) {
      Link *link = r.link;
      Node *node = r.node;
      int id = r.id;
    }
    recv_struct(const recv_struct& r) : link(r.link), node(r.node), id(r.id){}
};

struct status {
    sk_buff pkt;
    struct timeval endTime;
    struct timeval startTime;
    int numPkt;
    uint32_t seq;
};

struct args_struct {
    int num_pkts;
    struct nck_schedule *schedule;
    struct nck_timer *timer;
};


class Network {
public:
  Network();
  Network(int num_layers,
          //vector<vector<shared_ptr<Node>>> layersp,
          vector<vector<Node *>> layersp,
          //vector<shared_ptr<Link>> linksp,
          vector<Link *> linksp,
          //vector<Node *> nodesp);
          vector<shared_ptr<Node>> nodesp);

private:
  vector<Link> links;           // All the links that are in the Network
  vector<vector<Node>> layers;  // Each position of the vector is one layer and contains the nodes in that layer
  vector<Node> nodes;           // All the nodes that are in the Network
  int num_layers;
  int num_links;
  int num_nodes;
  //vector<shared_ptr<Link>> linksp;         // Vector containing the pointers of the links
  vector<Link *> linksp;
  //vector<Node *> nodesp;
  vector<shared_ptr<Node>> nodesp;         // Vector containing the pointers of the nodes
  //vector<vector<shared_ptr<Node>>> layersp;// Vector containing the pointers of the nodes that are in each layer (layers)
  vector<vector<Node *>> layersp;
  vector<int> destReached;      // Vector containing the ids of the nodes that have received and decoded all the packets
                                //  in case there is only one destination, it will be a vector of one position.
  int transmissionFinished;
  int stoppedRunning;
  struct nck_timer *timer;


  std::fstream log_file, res_file;
  std::string logFileName, resFileName;

  mutex m_logFile;
  mutex m_resFile;
  mutex m_running;
  mutex m_transmission;
  mutex m_stopped;

  bool nodesRunning;


    /*  LAYER vector explanation:
        the int will be the id_layer of the Node class.
        this will be -> {{1, Node1}, {2, Node2}, {2, Node3}, {2, Node4}, {3, Node5}}
        For example:
              _  _
             /  0  \
            /       \
           O--- 0 ---0
            \       /
             \_ 0 _/

        This whould be layers{{Node}, {Node1, Node2, Node3}, {Node}}
        layer 1 -> 1 node
        layer 2 -> 3 nodes
        layer 3 -> 1 node
     */

public:


  //SETS
  void setNum_layers(int num_layers);
  //void setLayersp(vector<vector<shared_ptr<Node>>> layersp);
  void setLayersp(vector<vector<Node *>> layersp);
  //void setLinksp(vector<shared_ptr<Link>> linksp);
  void setLinksp(vector<Link *> linksp);
  void setNodesp(vector<shared_ptr<Node>> nodesp);
  //void setNodesp(vector<Node *> nodesp);
  void setLinkCharacteristics(int id,
                               uint64_t throughput,
                               float latency,
                               float jitter,
                               float error_rate);

  void setNodesRunning();
/**
 * putDestReached: adds the id of the node that has received all the packets and doesn't expect more packets.
 * @idNodeDest: the id of the node
 * @return: returns
 */
  int putDestReached(int idNodeDest);

  //****************GETS********************

  // getNum_layers: @return: returns the number of layers that are in the network
  int getNum_layers();
  // getNumNodes: @return: number of nodes in the network
  int getNumNodes();
  // getNumSources: @return number of sources in the network;
  int getNumSources();

  // getLayers: @return: vector containing the Nodes in each layer (each layer is the position in the vector)
  //vector<vector<Node>> getLayers();
  // getLinks: @return: vector containing the links of the network.
  //vector<Link> getLinks();
  // getNodes: @return: vector containing the nodes of the network.
  //vector<Node> getNodes();
  // getNodesFromLayer: @return: vector containing the nodes that are in layer id = "layer".
  //vector<Node> getNodesFromLayer(int layer);

    // getLinksp: @return: vector containing the pointers of the links of the network.
  //vector<shared_ptr<Link>> getLinksp();
  vector<Link *> getLinksp();
  // getNodesp: @return: vector containing the pointers of the nodes of the network.
  vector<shared_ptr<Node>> getNodesp();
  //vector<Node *> getNodesp();
  // getNodesFromLayer: @return: vector containing the pointers of the nodes that are in layer id = "layer".
  //vector<shared_ptr<Node>> getNodespFromLayer(int layer);
  vector<Node *> getNodespFromLayer(int layer);

  // getNode: @return: class Node with id = idNode
  //Node getNode(int idnode);

  // getNode: @return: pointer Node with id = idNode
  //shared_ptr<Node> getNodep(int idnode);
  Node * getNodep(int idnode);
  // getLinkWithId: @return: pointer Link with id = idLink
  //shared_ptr<Link> getLinkpWithId(int idLink);
  Link * getLinkpWithId(int idLink);
  // getSrcSize: @return: size of the coder
  size_t getSrcSize();


 /**
  * initializeNetwork: initializes the network by filling all nodes vector, network, links etc...
  * @nodesInEachLayer: the vector containing how many nodes will be in each layer (the layer is the position in the vector)
  * @options_enc: the configuration of the encoder
  * @options_dec: the configuration of the decoder
  * @options_rec: the configuration of the recoder
  * @*timer: timer that will be done to create the encoder/decoder/recoder
  * @return: void.
  */
  void initializeNetwork(vector<int> nodesInEachLayer,
                         struct nck_option_value options_enc[],
                         struct nck_option_value options_dec[],
                         struct nck_option_value options_rec[],
                         struct nck_schedule *schedule,
                         struct nck_timer *timer,
                         struct timeval *step);

  /**
   *  initialzeFiles: creates a new folder in DIRECTORY by the name of the topology used and the date and time of its creation
   *                  and creates to files: log_file and res_file, where the logs of the programm will be printed and the
   *                  results of the simmulation to do graphs will be stored.
   */
  void initializeFiles();

  /**
   * putInNodes_NodesToSend: in each node, it sets a vector containing the nodes in the next layer
   */
  void putInNodes_NodesToSend();

  /**
   * initializeRandomLinks: confguration of random values of the links (thoughput, error_rate ...). it is done to ALL links
   */
  void initializeRandomLinks();

  /**
   * setRadomInLinks: confguration of random values of the links (thoughput, error_rate ...).
   * @param idLinks: vector with ids of the links that need a random configuration
   * @param return: void.
   */
  void setRadomInLinks(vector<int> idLinks);

  /**
   * setRandomForLayers: config of random values of some links that are in a specificLayer (thoughput, error_rate ...).
   * @param numRandomLinks: number of links that need a random configuration
   * @param fromLayer: layer where the links that need a random configuration are.
   */
  void setRandomForLayers(int numRandomLinks, int fromLayer);

  /**
    * createLinksFromLayers: using the vector of nodes and layers it creates each LINK and stores it in links attribute
    *                        it also stores the input and out link of each node in the specific node.
    */
  void createLinksFromLayers();

  /**
   * createLinksFromNewNode: when a new Node is added in the network, it needs links to some nodes.
   * @param node: new node added that need Links
   */
  //void createLinksFromNewNode(shared_ptr<Node> node);
  void createLinksFromNewNose(Node * node);

  /**
   * createLayersVector: from the vector of the nodes it creates the layers vector of the network
   */
  void createLayersVector();

  /**
   * deleteNodeFromLayer: delete node in a layer;
   * @param idNode: identifier of the node to delete
   * @param idLayer: identifier of the layer where the node is
   */
  void deleteNodeFromLayer(int idNode, int idLayer);

  /**
   * deleteNode: deletes the node from the network.
   * @param idNode : identifier of the node to delete
   */
  void deleteNode(int idNode);

  /**
   * deleteLinks: deletes the links that are passed by paraemter from the network.
   * @param idLinks: vector containing the identifiers of the links to delete
   */
  void deleteLinks(vector<int> idLinks);

  /**
   * addNode: New node is added to the network
   * @param idNode: the id of the node to add (in case there is a node with the same id, another id will be created)
   * @param idLayer: the layer where the node must be added
   * @param options_enc: configuration of the encoder
   * @param options_dec: configuration of the decoder
   * @param options_rec: configuration of the recoder
   * @param timer: timer nck_timer
   */
  void addNode(int idNode,
               int idLayer,
               struct nck_option_value *options_enc,
               struct nck_option_value *options_dec,
               struct nck_option_value *options_rec,
               nck_timer *timer);


  void addLinkToNode(int idNode, int idLink);
  void addNodeInLayer(Node node);
  //void addNodepInLayer(shared_ptr<Node> node);
  void addNodepInLayer(Node *node);
  int generateIdNode();
  //int checkIfNodeExists(shared_ptr<Node> node);
  int checkIfNodeExists(Node *node);
  //bool checkIfNodeExistsInLayer(int layer, Node *node);
  int whereIsLink(int idLink);
  int whereIsLinkp(int idLink);
  int computeNumLinks();


  void finishTransmission();
  int isTransmissionFinished();

  void toStringNodes();
  void toStringLinks();
  void toStringIdLinks();
  void toStringNodesInLayer(int idLayer);
  void toStringIdNodes();
  //std::string toStringDestStatus();


  //int sendPkt(Node *src_node, uint32_t pkt, struct nck_schedule *schedule);
  void nodesStoppedRunning();
  int howManyNodesStopped();
  bool allNodesRunning();
  void notifyNodes();

  std::string dateTime();
  std::string timeHour();
  void writeLog(string text);
  void writeRes(string text);
  void closeFiles();

};

#endif
