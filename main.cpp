#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iterator>
#include <algorithm>
#include <stdbool.h>
#include <cstring>
#include <thread>
#include <nckernel/nckernel.h>
#include <nckernel/skb.h>
#include <nckernel/timer.h>
#include <math.h>
#include "src/topology/Network.h"


int main() {

    struct nck_option_value options_enc[11] = {
            {"protocol",              "pacemg"},
            {"symbol_size",           "1500"},
            {"symbols",               "100"},
            {"max_active_containers", "100"},
            {"max_history",           "64"},
            {"coding_ratio",          "130"},
            {"timeout",               "2000000"},
            {"tail_packets",          "1"},
            {"feedback",              "0"},
            {"field",                 "binary8"},
            {"codec",                 "on_the_fly"}
    };

    struct nck_option_value options_rec[10] = {
            {"protocol",              "pacemg"},
            {"symbol_size",           "1500"},
            {"symbols",               "100"},
            {"max_active_containers", "100"},
            {"max_history",           "64"},
            {"coding_ratio",          "130"},
            //{"redundancy",            "80"},
            {"timeout",               "2000000"},
            {"tail_packets",          "1"},
            {"feedback",              "0"},
            {"field",                 "binary8"}
    };

    struct nck_option_value options_dec[5] = {
            {"protocol",              "pacemg"},
            {"symbol_size",           "1500"},
            {"symbols",               "100"},
            {"max_active_containers", "100"},
            {"field",                 "binary8"}
    };


    srand48(time(NULL));
    struct nck_schedule schedule;
    struct nck_timer timer;
    struct timeval step, timeout_bf;
    int num_sources = 1;
    int num_clients = 1;
    uint32_t total_pkts = 100;

    vector<struct timeval> time_start, time_end; //[num_clients];

    vector<int> nodesInEachLayer = {num_sources, 1, num_clients};

    Network network;

    cout << "Initializing Network..." << endl;
    for (auto it: nodesInEachLayer) {
        cout << " * " + to_string(it) + " * ";
    }
    network.initializeNetwork(nodesInEachLayer, options_enc, options_dec, options_rec, &schedule, &timer, &step);

    cout << endl << "Setting random values for links " << endl;

    network.initializeRandomLinks();

    int num_nodes = network.getNumNodes();
    cout << "num nodes = " + to_string(num_nodes) << endl;

    /***************** NETWORK INITIALIZED *************************/

    /*
    vector<shared_ptr<Node>> nodesp = network.getNodesp();
    vector<thread> nodeThreads, sourceThreads, destThreads;
     */

    // *********************************************************************
    // run all scheduled events

    nck_schedule_init(&schedule);
    nck_schedule_timer(&schedule, &timer);

    if (nck_schedule_run(&schedule, &step)) {
        // stop if nothing was scheduled
    }

    vector<Node *> sourceNodes = vector<Node *>( network.getNodespFromLayer(0));
    vector<Node *> destNodes = vector<Node *>(network.getNodespFromLayer(nodesInEachLayer.size() - 1));

    int c = 0;
    for (int i = 0; i < num_sources * total_pkts; i++) {
        nck_schedule_run(&schedule, &step);
        c = (c + 1) % num_sources;
        sourceNodes[c]->encodeSrcPkt();         // creates and encodes the source packets and send
        timeradd(&schedule.time, &step, &schedule.time);
    }
    timeradd(&schedule.time, &step, &schedule.time);
    nck_schedule_run(&schedule, &step);

    int decoded;
    while (decoded < total_pkts * num_sources * num_nodes) {
        decoded = 0;
        for (int i = 0; i < num_clients; i ++) {
            if (nck_schedule_run(&schedule, &step)) {
                destNodes[i]->decodePkt();
            }
            decoded = decoded + destNodes[i]->decodedPkts();
        }
        timeradd(&schedule.time, &step, &schedule.time);
    }
    /*
    for (int i = 0; i < num_clients; i ++) {
        while (destNodes[i]->decodedPkts() < total_pkts*num_sources){
            nck_schedule_run(&schedule, &step);
            timeradd(&schedule.time, &step, &schedule.time);
        }
        cout << endl << "Node " << destNodes[i]->getId() << " has received all packets" << endl;
        network.writeLog("Node " + to_string(destNodes[i]->getId()) + " Decoded pkts: " + to_string(destNodes[i]->decodedPkts() - 1));
    }
     */
    cout << endl << "Destination nodes have received all packets" << endl;

    //PKts have been decoded




    // *********************************************************************


    /*

    cout << thread::hardware_concurrency() << endl;
    for( int i = 0; i < num_nodes; i++ ) {

        cout << "Creating thread, " << i << endl;
        Node *n = nodesp[i].get();

        if (nodesp[i].get()->getType() == 0) {
            sourceThreads.push_back(std::thread(&Node::run, n, total_pkts, &schedule, &timer));
        }
        else if (nodesp[i].get()->getType() == 1) {
            destThreads.push_back(std::thread(&Node::run, n, total_pkts, &schedule, &timer));
        }
        else {
            nodeThreads.push_back(std::thread(&Node::run, n, total_pkts, &schedule, &timer));
        }
    }

    network.setNodesRunning();
    network.writeLog("MAIN >>>> all nodes are running");

    //threads are running,
    //Nodes are going to be encoding and decoding and putting the packets in the link

    int i = 0;
    //wait till the encoders have finiished encoding ...
    for (int it = 0; it < num_sources; it ++) {
        sourceThreads[it].join();
        i ++;
    }
    cout << "... Packets created and sent ..." << endl;
    //now all the sources have finished creating packets and sending them

    while (network.howManyNodesStopped() < num_nodes) {}
    //all clients have received:
    cout << "... Transmission has been finished ... packets have arrived the destination ... " << endl;

    //all threads have finished.

     */

    //finish the programm -> all the data is stored in the files.
    network.closeFiles();

    nck_schedule_free_all(&schedule);
    return 0;
}



