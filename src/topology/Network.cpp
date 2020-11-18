using namespace std;

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>

#include <nckernel/nckernel.h>
#include <nckernel/skb.h>
#include <nckernel/timer.h>
#include <math.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <bits/shared_ptr.h>

#include "Network.h"
#include "../private.h"


#define DIRNAME "/home/clara/Documentos/TFG/sim_results"


//CONSTRUCTORS ************

Network::Network() {
  num_links = 0;
  initializeFiles();
  this->nodesRunning = false;
  this->transmissionFinished = 0;
  this->stoppedRunning = 0;
}


Network::Network(int num_layers,
                 //vector<vector<shared_ptr<Node>>> layersp,
                 vector<vector<Node *>> layersp,
                 //vector<shared_ptr<Link>> linksp,
                 vector<Link *> linksp,
                 //vector<Node *> nodesp) {
                 vector<shared_ptr<Node>> nodesp) {
  this->num_layers = num_layers;
  this->layersp = (layersp);
  this->nodesp = nodesp;
  this->linksp = (linksp);
  this->transmissionFinished = 0;
  this->stoppedRunning = 0;
}





//SETS *******************
void Network::setNum_layers(int num_layers) {
  this->num_layers = num_layers;
}
//void Network::setLayersp(vector<vector<shared_ptr<Node>>> layersp) {
void Network::setLayersp(vector<vector<Node *>> layersp) {
  this->layersp = (layersp);
}
//void Network::setLinksp(vector<shared_ptr<Link>> linksp) {
void Network::setLinksp(vector<Link *> linksp) {
  this->linksp = linksp;
}
/*
void Network::setNodesp(vector<Node *> nodesp) {
  this->nodesp = nodesp;
  this->num_nodes = nodesp.size();
}
*/
void Network::setNodesp(vector<shared_ptr<Node>> nodesp) {
  this->nodesp = nodesp;
  this->num_nodes = nodesp.size();
}

void Network::setLinkCharacteristics(int id,
                                     uint64_t throughput,
                                     float latency,
                                     float jitter,
                                     float error_rate) {
  std::size_t it = 0;
  bool found = false;
  while (it < linksp.size() && found == false) {
    if (linksp[it]->getId() == id) {
      linksp[it]->setLinkCharacteristics(throughput, latency, jitter, error_rate);
      found = true;
    }
    it ++;
  }
}

void Network::setNodesRunning() {
    std::lock_guard<std::mutex> lock_guard(this->m_running);
    this->nodesRunning = true;
}




//GETS ********************
int Network::getNum_layers() {
  return num_layers;
}
int Network::getNumNodes() {
  return (int) nodesp.size();
}
int Network::getNumSources() {
  return (int) layersp[0].size();
}

/*
vector<vector<Node>> Network::getLayers() {
  return layers;
}

vector<Link> Network::getLinks() {
  return links;
}
*/
//vector<shared_ptr<Link>> Network::getLinksp() {
vector<Link *> Network::getLinksp() {
  return this->linksp;
}
/*
vector<Node> Network::getNodes() {
  return nodes;
}*/
vector<shared_ptr<Node>> Network::getNodesp() {
//vector<Node *> Network::getNodesp() {
  return nodesp;
}
/*
vector<Node> Network::getNodesFromLayer(int layer) {
  vector<Node> nodesInLink;
  if (!nodes.empty()) {
    for (int i = 0; i < nodes.size(); i++ ) {
      Node *n = nodes[i];
      if (n.getLayer_id() == layer) {
        nodesInLink.push_back(n);
      }
    }
  }
  else {
    writeLog("ERROR - NETWORK >> Trying to get Nodes from layer - There are NO nodes in layer: " + to_string(layer) + "\n");
    //std::cout << "Network >> There are no nodes in this layer: " << layer << std::endl;
  }
  //return layers[layer];
  return nodesInLink;
}
*/
//vector<shared_ptr<Node>> Network::getNodespFromLayer(int layer) {
vector<Node *> Network::getNodespFromLayer(int layer) {
  //vector<shared_ptr<Node>> nodesInLink;
  vector<Node *> nsInLayer;
  if (!nodesp.empty() && layer < (int) layersp.size()) {
    /*
    for (size_t i = 0; i < nodesp.size(); i++) {
      shared_ptr<Node> node_p = nodesp[i];
      if (node_p->getLayer_id() == layer) {
          nodesInLink.push_back(node_p);
      }
    }
    // check that it has the same as in vector layers
    if (nodesInLink.size() != layersp[layer].size()) {
        writeLog("\nERROR - NETWORK >> getNodespFromLayer (not matching) Updating vector 'layer': " + to_string(layer) );
        layersp[layer].clear();     //delete nodes from layersp vector
        layersp[layer].insert( layersp[layer].end(), nodesInLink.begin(), nodesInLink.end() );
    }
     */
    if (!layersp[layer].empty()) nsInLayer = layersp[layer];
  }
  else {
      writeLog("ERROR - NETWORK >> Trying to get Nodes from layer - There are NO nodes in layer: " + to_string(layer) + "\n");
      //std::cout << "Network >> There are no nodes in this layer: " << layer << std::endl;
  }
  //return layersp[layer];
  return nsInLayer;
}
/*
Node Network::getNode(int idNode) {
  Node node();
  for (int it = 0; it < nodes.size(); it++) {
    if (nodes[it].getId() == idNode) {
      node = nodes[it];
    }
  }
  return node;
}
 */

//shared_ptr<Node> Network::getNodep(int idNode) {
Node * Network::getNodep(int idNode) {
  //shared_ptr<Node> node;
  Node *node = nullptr ;
  for (auto it = begin(nodesp); it != end(nodesp); it++) {
    if ((*it)->getId() == idNode) {
      node = (*it).get();
    }
  }
  return node;
}

//shared_ptr<Link> Network::getLinkpWithId(int idLink) {
Link * Network::getLinkpWithId(int idLink) {
  //shared_ptr<Link> link;
  Link * link = nullptr;
  for (auto it = begin(linksp); it != end(linksp); it++) {
    if ((*it)->getId() == idLink) {
      link =  (*it);
    }
  }
  return link;
}

size_t Network::getSrcSize() {
  return (nodes[0]).getCoder().source_size;
}


//OTHERS
void Network::initializeNetwork(vector<int> nodesInEachLayer,
                                struct nck_option_value options_enc[],
                                struct nck_option_value options_dec[],
                                struct nck_option_value options_rec[],
                                struct nck_schedule *schedule,
                                struct nck_timer *timer,
                                struct timeval *step) {
  this->transmissionFinished = 0;
  this->num_layers = (int) nodesInEachLayer.size();
  this->num_nodes = 0;
  this->timer = timer;
  int id_nodes = 0;
  writeLog( "NETWORK >> initializeNetwork >> CREATING NODES ... " );
  //We put nodes in each layer
  for (int i = 0; i < num_layers; i ++) {
    for (int j = 0; j < nodesInEachLayer[i]; j ++) {
      Node *newNodep = new Node(id_nodes, i);
      newNodep->setNetwork(this);
      newNodep->setSchedule(schedule);
      newNodep->setStep(step);
      //writeLog( "NETWORK >> initializeNetwork >> ENCODER - layer " + to_string(newNodep->getLayer_id()));
      if(i == 0) {
        writeLog( "NETWORK >> initializeNetwork >> ENCODER - node " + to_string(id_nodes));
        newNodep->createEncoderInNode(options_enc, timer);
      }
      else if(i == num_layers - 1) { //we are in the last node -> decoder {
        writeLog( "NETWORK >> initializeNetwork >> DECODER - node " + to_string(id_nodes));
        newNodep->createDecoderInNode(options_dec, timer);
      }
      else {
        writeLog( "NETWORK >> initializeNetwork >> RECODER - node " + to_string(id_nodes));
        newNodep->createRecoderInNode(options_rec, timer);
      }
      //add new node to nodes vector in the network
      nodesp.push_back(std::shared_ptr<Node>(newNodep)); //Shared pointer -> we share it with all the threads.
      //nodes.push_back(std::move(newNode));
      id_nodes ++;
    }
    num_nodes += nodesInEachLayer[i];
  }
  writeLog( "NETWORK >> initalizeNetwork >> number of nodes = " + to_string(num_nodes));
  writeLog( "NETWORK >> initializeNetwork >> CREATING LAYERS VECTOR ... " );
  createLayersVector();
  writeLog( "NETWORK >> initializeNetwork >> CREATING LINKS VECTOR ... " );
  createLinksFromLayers();
  putInNodes_NodesToSend();
}

void Network::initializeFiles() {
 std::string dirName = DIRNAME;
 std::string dir = dirName + "/top1_" + dateTime();
 mkdir(dir.c_str(), S_IRWXU);
 this->logFileName = dir + "/" + "log.txt";
 this->resFileName = dir + "/" + "res.txt";
 this->log_file = std::fstream(logFileName);
 this->res_file = std::fstream(resFileName);
 log_file.open(logFileName, fstream::app);
 res_file.open(resFileName, fstream::app);
}

void Network::putInNodes_NodesToSend() {
  for (int i = 0; i < num_nodes; i++) {
    //shared_ptr<Node> n = nodesp[i];
    Node * n = nodesp[i].get();
    int layer = n->getLayer_id();
    if(layer < num_layers - 1) {
      n->setNodesToSend(getNodespFromLayer(layer + 1));
    }
  }
}

void Network::initializeRandomLinks() {
  writeLog( "NETWORK >> initializeRandomLinks >> setting random values for links");
  for (auto it = begin(linksp); it != end(linksp); it ++) {
    (*it)->randomCharacteristics();
    writeLog((*it)->toStringLinkCharacteristics());
  }
}

void Network::setRadomInLinks(vector<int> idLinks) {
  for (auto i = begin (idLinks); i != end (idLinks); i++) {
    for (auto j = begin(linksp); j != end (linksp); j++) {
      if ((*j)->getId() == (*i)) {
        (*j)->randomCharacteristics();
      }
    }
  }
}

void Network::setRandomForLayers(int numRandomLinks, int fromLayer) {
  if (fromLayer < num_layers) {
    int count = 0;
    vector<int> v(links.size(), 0);

    std::cout << "Random Links = ";
    while (count < numRandomLinks) {
      int randomIndex = (int) (rand()%(links.size()) + 1);
      if (linksp[randomIndex]->getLayer_src() == fromLayer && v[randomIndex] == 0) {
        linksp[randomIndex]->randomCharacteristics();
        std::cout << randomIndex << " , ";
        v[randomIndex] = 1;
        count++;
      }
    }
    std::cout << std::endl;
  }
  else {
    std::cout << "Network >> setRandomForLayers: the layer number is wrong -> should be less than num_layers" << '\n';
  }
}

void Network::createLinksFromLayers() {
  num_links = computeNumLinks();
  writeLog( "NETWORK >> createLinksFromLayers >> Number of links: " + to_string(num_links) );

  for (int i = 0; i < num_layers - 1; i++) {
    writeLog( "NETWORK >> createLinksFromLayers -> From Layer: " + to_string(i) + " To Layer " + to_string(i + 1));
    //vector<shared_ptr<Node>> actual_nodesp = getNodespFromLayer(i);
    //vector<shared_ptr<Node>> future_nodesp = getNodespFromLayer(i);
    vector<Node *> actual_nodesp = getNodespFromLayer(i);
    vector<Node *> future_nodesp = getNodespFromLayer(i + 1);
    //writeLog( "NETWORK >> createLinksFromLayers -> actual_nodes: " + to_string(actual_nodesp.size()) + "\n" );
    //writeLog( "NETWORK >> createLinksFromLayers -> future_nodes: " + to_string(future_nodesp.size()) + "\n" );

    for (auto j = begin (actual_nodesp); j != end (actual_nodesp); ++j ) {
      //shared_ptr<Node> n = (*j);
      Node *n = (*j);

      for (auto k = begin (future_nodesp); k != end (future_nodesp); ++k ) {
        //shared_ptr<Node> v = (*k);
        Node *v = (*k);

        //we create a link:
        Link *linkp = new Link((int) linksp.size(), n->getId(), v->getId(), n->getLayer_id(), v->getLayer_id());
        writeLog( "NETWORK >> createLinksFromLayers >> link " + to_string(linkp->getId()) + " created");
        //link.toString();
        //links.push_back(link);
        //shared_ptr<Link> linkp(&link);
        //Link * linkp = &link;
        linksp.push_back(linkp);
        linkp->setTimer(timer);

        //add Link to nodes src and dst:
        addLinkToNode(n->getId(), linkp->getId());
        addLinkToNode(v->getId(), linkp->getId());

        //TODO: should I change addLinkp (int id, shared_ptr<Link> linkp) or with Link* link is fine?
        n->addLinkp(linkp->getId(), linkp);
        v->addLinkp(linkp->getId(), linkp);
        v->addLinkpToRecvLink(linkp);
      }
    }
  }
}

//void Network::createLinksFromNewNode(shared_ptr<Node> node) {
void Network::createLinksFromNewNose(Node *node) {
  int layer = node->getLayer_id();
  if (layer != 0) {
    //vector<shared_ptr<Node>> previous_nodes = getNodespFromLayer(layer - 1);
    vector<Node *> previous_nodes = getNodespFromLayer(layer - 1);

    for (auto it = begin (previous_nodes); it != end (previous_nodes); ++it ) {
      //shared_ptr<Node> p = (*it);
      Node * p = (*it);

      //we create a link:
      int id_link = linksp[num_links - 1]->getId() + 1;
      Link link(id_link, p->getId(), node->getId(), layer - 1, layer);
      link.randomCharacteristics();
      writeLog( "NETWORK >> createLinksFromNewNode >> link " + to_string(id_link) + " created" );
      //link.toString();
      links.push_back(link);
      //linksp.push_back(shared_ptr<Link>(&link));
      linksp.push_back(&link);
      num_links ++;
      writeLog( "NETWORK >> Num_links = " + to_string(num_links) + "\n" );

        //add Link to nodes:
      addLinkToNode(node->getId(), id_link);
      addLinkToNode(p->getId(), id_link);
    }
  }
  if (layer == num_layers - 1) {
    //vector<shared_ptr<Node>> future_nodes = getNodespFromLayer(layer + 1);
    vector<Node *> future_nodes = getNodespFromLayer(layer + 1);

    for (auto it = begin (future_nodes); it != end (future_nodes); ++it ) {
      //shared_ptr<Node> f = (*it);
      Node *f = (*it);

      //we create a link:
      int id_link = links[num_links - 1].getId() + 1;
      Link link(id_link, node->getId(), f->getId(), layer, layer + 1);
      writeLog( "NETWORK >> createLinksFromNewNode >> link " + to_string(id_link) + " created" );
      //link.toString();
      links.push_back(link);
      //linksp.push_back(shared_ptr<Link>(&link));
      linksp.push_back(&link);

        //add Link to nodes:
      addLinkToNode(node->getId(), (int) links.size() - 1);
      addLinkToNode(f->getId(), (int) links.size() - 1);
    }
  }
}

void Network::createLayersVector() {
  layersp.clear();
  layersp.resize(num_layers);
  if(!nodesp.empty()) {
    for (int it = 0; it < (int)nodesp.size(); it++) {
      int layer = nodesp[it]->getLayer_id();
      writeLog( "NETWORK >> createLayersVector >> putting Node " + to_string(nodesp[it]->getId()) + " in Layer " + to_string(layer));
      layersp[layer].push_back(nodesp[it].get());
    }
  }
  else {
    writeLog( "ERROR: NETWORK >> createLayersVector >> Layers vector could not be created because nodes vector is NULL" );
  }
  if (layersp.empty()) {
    writeLog( "ERROR: NETWORK >> createLayersVector >> Layers vector could not be created" );
  }
}

void Network::deleteNodeFromLayer(int idNode, int idLayer) {
  bool deleted = false;
  size_t it = 0;
  //vector<Node> ns = layers[idLayer];
  //vector<shared_ptr<Node>> nsp = layersp[idLayer];
  vector<Node *> nsp = layersp[idLayer];

  while (it < nsp.size() && !deleted) {
    //shared_ptr<Node> node = nsp[it];
    Node *node = nsp[it];
    if (node->getId() == idNode) {
        //we found the node in the vector -> we can delete them now
      layersp[idLayer].erase(layersp[idLayer].begin() + it);
      writeLog( "NETWORK >> deleteNodeFromLayer >> Node " + to_string(idNode) + " has been deleted from layer " + to_string(idLayer) + "\n" );
      deleted = true;
      break;
    }
    it ++;
  }
  if (!deleted) {
      writeLog( "NETWORK >> deleteNodeFromLayer >> There isn't a Node with id :  " + to_string(idNode) + " in layer " + to_string(idLayer) + "\n" );
  }
}

void Network::deleteNode(int idNode) {
  bool deleted = false;
  size_t it = 0;
  while(it < nodesp.size() && !deleted) {
    if (nodesp[it]->getId() == idNode) {
      writeLog( "NETWORK >> deleteNode " + to_string(idNode) + " is going to be deleted\n" );
      //nodes[it].toString();
      vector<int> linksFromNode = nodesp[it]->getLinks();   //we need to delete the links that go to node "nodes[it]"
      deleteLinks(linksFromNode);
      //toStringIdLinks();
      int layer = nodesp[it]->getLayer_id();
      //nodes.erase(nodes.begin() + it);
      nodesp.erase(nodesp.begin() + it);
      toStringIdNodes();
      deleteNodeFromLayer(idNode, layer);
      //toStringNodesInLayer(layer);
      //TODO: stop THREAD!!!
      deleted = true;
      num_nodes --;
    }
    it ++;
  }
  if (!deleted) {
      std::cout << "Network >> There isn't a Node in list of nodes with id : " << idNode << std::endl;
  }
}

void Network::deleteLinks(vector<int> idLinks) {
  for (size_t i = 0; i < idLinks.size(); i++) {
    int pos = whereIsLinkp(idLinks[i]);
    if (pos != -1) {
      // remove link from list of links
      int idLink = linksp[pos]->getId();
      int idNodeSrc = linksp[pos]->getIdNode_src();
      int idNodeDst = linksp[pos]->getIdNode_dst();
      links.erase(links.begin() + pos);
      linksp.erase(linksp.begin() + pos);
      //link has been deleted -> we need to remove also the links that are in the Node list
      getNodep(idNodeDst)->removeRecvLink(idLink);
      getNodep(idNodeSrc)->removeNode(getNodep(idNodeDst)); //delete the node to send
      writeLog("NETWORK >> Link " + to_string(idLinks[i]) + " has been deleted\n");
      num_links --;
    }
    else {
      writeLog("ERROR - NETWORK >> deleteLinks >> Link " + to_string(idLinks[i]) + " could not be deleted -> it doesn't exists\n");
    }
  }
  writeLog("NETWORK >> num_links has been modified = " + to_string(num_links) + "\n");
}

void Network::addNode(int idNode,
                      int idLayer,
                      struct nck_option_value *options_enc,
                      struct nck_option_value *options_dec,
                      struct nck_option_value *options_rec,
                      nck_timer *timer ) {
  Node newNode(idNode, idLayer);
  newNode.setNetwork(this);
  //int exists = checkIfNodeExists(shared_ptr<Node>(&newNode));
  int exists = checkIfNodeExists(&newNode);
  if (exists == 0) {
    if (idLayer == 0) {
      newNode.createEncoderInNode(options_enc, timer);
    }
    else if (idLayer == num_layers - 1) { //we are in the last node -> decoder {
      newNode.createDecoderInNode(options_dec, timer);
    } else {
      newNode.createRecoderInNode(options_rec, timer);
    }
    //nodes.push_back(newNode);
    nodesp.push_back(shared_ptr<Node>(&newNode));
    //nodesp.push_back(&newNode);
    writeLog( "NETWORK >> Node " + to_string(idNode) +  " succesfully added\n");
    //addNodepInLayer(shared_ptr<Node>(&newNode));
    addNodepInLayer(&newNode);
    //createLinksFromNewNode(shared_ptr<Node>(&newNode));
    createLinksFromNewNose(&newNode);
    num_nodes ++;
  }
  else if (exists == 1) {
    writeLog( "NETWORK >> Creating new Id for new node " + to_string(idNode) + "\n" );
    int newId = generateIdNode();
    newNode.setId(newId);
    writeLog( "NETWORK >> New ID created " + to_string(newId) + "\n" );
    if (idLayer == 0) {
      newNode.createEncoderInNode(options_enc, timer);
    }
    else if (idLayer == num_layers - 1) { //we are in the last node -> decoder {
      newNode.createDecoderInNode(options_dec, timer);
    } else {
      newNode.createRecoderInNode(options_rec, timer);
    }
    //nodes.push_back(newNode);
    nodesp.push_back(shared_ptr<Node>(&newNode));
    //nodesp.push_back(&newNode);
    //node->toString();
    writeLog( "NETWORK >> Node " + to_string(newId) + " succesfully added\n" );
    //addNodepInLayer(shared_ptr<Node>(&newNode));
    addNodepInLayer(&newNode);

    //createLinksFromNewNode(shared_ptr<Node>(&newNode));
    createLinksFromNewNose(&newNode);
    num_nodes ++;
  }
  else {
    writeLog( "NETWORK >> Cannot add Node " + to_string(idNode) + ", it already exists\n" );
  }
}


void Network::addLinkToNode(int idNode, int idLink) {
  //writeLog( "NETWORK >> addLinkToNode >> adding Link " + to_string(idLink) + " to Node " + to_string(idNode));
  int it = 0;
  bool exists = false;
  while (it < nodesp.size() && !exists) {
    if (nodesp[it]->getId() == idNode) {
      //nodes[it].addLink(idLink);
      nodesp[it]->addLink(idLink);
      writeLog( "NETWORK >> addLinkToNode >> Link " + to_string(idLink) +
      " added to node " + to_string(idNode) );
      exists = true;
    }
    it++;
  }
  if (!exists) {
    writeLog( "ERROR : NETWORK >> addLinkToNode >> It was impossible to add Link because node " + to_string(idNode) + "doesn't exists\n" );
  }
}

void Network::addNodeInLayer(Node node) {
  //layers[node.getLayer_id()].push_back(node);
  //layersp[node.getLayer_id()].push_back(shared_ptr<Node>(&node));
  layersp[node.getLayer_id()].push_back(&node);
}

//void Network::addNodepInLayer(shared_ptr<Node> node) {
void Network::addNodepInLayer(Node *node) {
  //layers[node->getLayer_id()].push_back(*node); //TODO : WRONG
  layersp[node->getLayer_id()].push_back(node);
}

int Network::generateIdNode() {
  //shared_ptr<Node> n = nodesp[nodesp.size() - 1];
  Node *n = nodesp[nodesp.size() - 1].get();
  int id = n->getId() + 1;
  return id;
}

int Network::computeNumLinks() {
  int result = 0;
  writeLog( "NETWORK >> computing number of links ...") ;
  for (int i = 0; i < num_layers - 1; i++) {
    //vector<shared_ptr<Node>> nodes_act = getNodespFromLayer(i);
    //vector<shared_ptr<Node>> nodes_adv = getNodespFromLayer(i + 1);
    vector<Node *> nodes_act = getNodespFromLayer(i);
    vector<Node *> nodes_adv = getNodespFromLayer(i + 1);
    result = result + (int) (nodes_act.size() * nodes_adv.size());
  }
  return result;
}

int Network::whereIsLink(int idLink) {
  size_t it = 0;
  while (it < links.size()) {
   if (links[it].getId() == idLink) {
     return (int) it;
   }
   it++;
  }
  return -1;
}

int Network::whereIsLinkp(int idLink) {
  int it = 0;
  while (it < (int)linksp.size()) {
    if (linksp[it]->getId() == idLink) {
      return it;
    }
    it++;
  }
  return -1;
}


void Network::finishTransmission() {
  std::lock_guard<std::mutex> lock_guard(this->m_transmission);
  transmissionFinished = true;
}

int Network::isTransmissionFinished() {
  std::lock_guard<std::mutex> lock_guard(this->m_transmission);
  return transmissionFinished;
}

//int Network::checkIfNodeExists(shared_ptr<Node> node) {
int Network::checkIfNodeExists(Node *node) {
  int id = node->getId();
  int layer_id = node->getLayer_id();
  int exists = 0;
  int i = 0;
  while (i < (int) nodesp.size() && exists == 0) {
    if (nodesp[i]->getId() == id && nodesp[i]->getLayer_id() == layer_id) {
      exists = 2;
      writeLog( "NETWORK >> Exists a node with id = " + to_string(id) + " in layer " + to_string(nodesp[i]->getLayer_id()) + "\n");
    }
    else if ((nodesp[i]->getId() == id) && nodesp[i]->getLayer_id() != layer_id) {
      writeLog("NETWORK >> Exists a node with id = " + to_string(id) + " BUT is in layer " + to_string(nodesp[i]->getLayer_id()) + "\n" );
      //nodesp[i]->toString();
      exists = 1;
      break;
    }
    i++;
  }
  return exists;
}
/*
bool Network::checkIfNodeExistsInLayer(int layer, Node *node) {
  vector<shared_ptr<Node>> nodesInLayer = getNodespFromLayer(layer);
  bool exists = false;   //doesn't exist
  int i = 0;
  int id = node->getId();
  while (i < nodesInLayer.size() && !exists) {
    Node* n = nodesInLayer[i];
    if (node->checkEqualsP(n)) {
      std::cout << "Network >> Node" << node->getId() << " exists in layer" << layer << std::endl;
      exists = true;   //it exists
    }
    else if ((id == n->getId()) && (!node->checkEqualsP(n))) {
      std::cout << "Network >> There is a node with id = " << id << ". The node you entered is not the same as this one" << std::endl;
      std::cout << "Network >> Node" << id << " saved is :" << std::endl;
      node->toString();
      exists = true;   //it exists a node with the same id but it's not the same node.
    }
    i++;
  }
  return exists;
}
 */




//TOSTRINGS
void Network::toStringNodes() {
  std::cout << ">> NODES: " << '\n';
  for (auto it = begin (nodes); it != end (nodes); ++it) {
    it->toString();
  }
}

void Network::toStringLinks() {
  std::cout <<  std::endl << ">> LINKS: " << '\n';
  for (auto it = begin (links); it != end (links); ++it ) {
    it->toString();
  }
}

void Network::toStringIdLinks() {
  std::cout << std::endl << ">> LINKS: " << '\n' << " - ";

  for (auto it = begin (links); it != end (links); ++it ) {
    std::cout << it->getId() << "; ";
  }
  std::cout << std::endl;
}

void Network::toStringNodesInLayer(int idLayer) {
  std::cout << ">> NDES IN LAYER : " << idLayer << '\n' << " - ";
  //vector<shared_ptr<Node>> nodesLayer = getNodespFromLayer(idLayer);
  vector<Node *> nodesLayer = getNodespFromLayer(idLayer);
  for (auto it = begin (nodesLayer); it != end (nodesLayer); ++it ) {
    std::cout << (*it)->getId() << "; ";
  }
  std::cout << std::endl;
}

void Network::toStringIdNodes() {
  std::cout << ">> NODES : " << '\n' << " - ";
  for (auto it = begin (nodes); it != end (nodes); ++it ) {
    std::cout << it->getId() << "; ";
  }
  std::cout << std::endl;
}

void Network::nodesStoppedRunning() {
  std::lock_guard<std::mutex> lock_guard(this->m_stopped);
  stoppedRunning ++;
}

int Network::howManyNodesStopped() {
  std::lock_guard<std::mutex> lock_guard(this->m_stopped);
  return stoppedRunning;
}

bool Network::allNodesRunning() {
    std::lock_guard<std::mutex> lock_guard(this->m_running);
    return this->nodesRunning;
}

void Network::notifyNodes() {
    /*
    vector<Node *> ns = getNodespFromLayer();
    for (auto it = begin(ns); it != end(ns); it ++) {
        nck_flush_coded(*it);
    }
     */
}

std::string Network::dateTime() {
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, 80,"%d-%m-%Y_%H-%M-%S", timeinfo);
  return std::string(buffer);
}


std::string Network::timeHour() {
  using std::chrono::system_clock;
  auto currentTime = std::chrono::system_clock::now();
  char buffer[80];

  auto transformed = currentTime.time_since_epoch().count() / 1000000;

  auto millis = transformed % 1000;

  std::time_t tt;
  tt = system_clock::to_time_t ( currentTime );
  auto timeinfo = localtime (&tt);
  strftime (buffer,80,"%H:%M:%S",timeinfo);
  sprintf(buffer, "%s:%03d",buffer,(int)millis);

  return std::string(buffer);
}

void Network::writeLog(string text) {
  std::lock_guard<std::mutex> lock_guard(this->m_logFile);
  log_file << "[" + timeHour() + "] -- " + text << endl;
}

void Network::writeRes(string text) {
    std::lock_guard<std::mutex> lock_guard(this->m_resFile);
    res_file << "[" + timeHour() + "] " + text << endl;
}

void Network::closeFiles() {
  log_file.close();
  res_file.close();
}