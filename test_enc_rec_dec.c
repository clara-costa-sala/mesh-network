
//
// Created by clara on 22.01.19.
//
//

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <bits/types/struct_timeval.h>
#include <nckernel/api.h>
#include <nckernel/skb.h>
#include <nckernel/timer.h>
#include <nckernel/nckernel.h>
#include <nckernel/skb.h>
#include <nckernel/timer.h>
#include <math.h>

#include "../src/private.h"

#define NUM_RECS 5

struct link_struct {
    int id;
    double error_rate;
    uint64_t bps;
    struct timeval latency;
    double max_jitter_ms;
    struct nck_timer *timer;
    struct nck_timer_entry *ready_timer;
    //struct packet_struct pkt;
    int ready;
    struct sk_buff packet;
};

struct send_struct {
    struct link_struct *link;
    struct nck_encoder *enc;
    struct nck_recoder *recs[NUM_RECS];
    struct nck_decoder *dec;
    int id_recoder;
    int is_recoder;                    //if is a recoder
    struct sk_buff *packet;
    int is_feedback;                //is a feedback packet?
};


static void receive(struct nck_timer_entry *handle, void *context, int success) {
    struct send_struct *s = (struct send_struct *)context;
    struct link_struct *link = s->link;
    struct sk_buff *packet = s->packet;

    if (s->is_recoder) {   //we need to recode.
        //struct nck_recoder *rec = s->rec;

    }


    int received = drand48() >= s->link->error_rate;


    if (success && received) {
        nck_put_coded(s->rec, &(s->link->packet));
    }

    if (handle) {
        nck_timer_free(handle);
    }
    free(frame->new_pkt);
    free(frame);
}

int getRandomvalue (int min, int max) {
    srand(time(NULL));
    int random_num = rand() % max;
    if (random_num < min){
        random_num += min;
    }
    return random_num;
}

int main() {

    struct nck_encoder *enc;
    struct nck_recoder *rec[NUM_RECS];
    struct nck_decoder *dec;

    struct nck_schedule schedule;
    struct timeval step;
    struct nck_timer timer;
    struct timespec clock;
    srand(time(NULL));

    nck_schedule_init(&schedule);
    clock_gettime(CLOCK_MONOTONIC, &clock);
    schedule.time.tv_sec = clock.tv_sec;
    schedule.time.tv_usec = clock.tv_nsec / 1000;
    nck_schedule_timer(&schedule, &timer);


    struct nck_option_value options_enc[12] = {
            {"protocol",              "pacemg"},
            {"symbol_size",           "1500"},
            {"symbols",               "10"},
            {"max_active_containers", "2"},
            {"max_history",           "64"},
            {"coding_ratio",          "130"},
            {"redundancy",            "100"},
            {"timeout",               "200000000"},
            {"tail_packets",          "0"},
            {"feedback",              "1"},
            {"field",                 "binary8"},
            {"codec",                 "on_the_fly"},
    };

    struct nck_option_value options_rec[13] = {
            {"protocol",              "pacemg"},
            {"symbol_size",           "1500"},
            {"symbols",               "10"},
            {"max_active_containers", "2"},
            {"max_history",           "64"},
            {"coding_ratio",          "130"},
            {"redundancy",            "100"},
            {"timeout",               "200000000"},
            {"tail_packets",          "0"},
            {"feedback",              "1"},
            {"field",                 "binary8"},
            {"codec",                 "on_the_fly"}
    };

    struct nck_option_value options_dec[5] = {
            {"protocol",              "pacemg"},
            {"symbol_size",           "1500"},
            {"symbols",               "10"},
            {"max_active_containers", "2"},
            {"field",                 "binary8"}
    };


    if (nck_create_encoder(&enc, NULL, options_enc, nck_option_from_array)) {
        fprintf(stderr, "Failed to create encoder");
        return -1;
    }
    printf("\nEncoder created");

    if (nck_create_decoder(&dec,  NULL, options_dec, nck_option_from_array)) {
        fprintf(stderr, "Failed to create decoder");
        return -1;
    }
    printf("\nDecoder created");

    if (nck_create_recoder(&rec,  NULL, options_rec, nck_option_from_array)) {
        fprintf(stderr, "Failed to create recoder");
        return -1;
    }
    printf("\nRecoder created");


    // **************************** INITIALIZE ************************

    uint32_t seq_gen_num = 0;
    uint32_t global_seq_num = 0;
    int num_pkts = 100;

    size_t source_size = enc.source_size;
    size_t coded_size = enc.coded_size;
    printf("\nSource size = %lu, Coded_size = %lu\n", source_size, coded_size);

    uint8_t buffer_enc_source[source_size];
    memset(buffer_enc_source, 0, source_size);
    uint8_t buffer_enc_source1[source_size];
    memset(buffer_enc_source1, 0, source_size);
    uint8_t buffer_enc_coded[coded_size];
    memset(buffer_enc_coded, 0, coded_size);
    uint8_t buffer_enc_coded1[coded_size];
    memset(buffer_enc_coded1, 0, coded_size);

    uint8_t buffer_feedback[enc.feedback_size];
    memset(buffer_feedback, 0, enc.feedback_size);
    uint8_t buffer_feedback2[enc.feedback_size];
    memset(buffer_feedback2, 0, enc.feedback_size);
    struct sk_buff skb_feedback, skb_feedback2;

    struct sk_buff skb_buff_enc, skb_buff_enc1, sk_buff_rec, sk_buff_dec;
    /************************* initialize links ********************/
    struct link_struct links[2];
    links[0].error_rate = (float)getRandomvalue(0, 30)/100;
    //links[0].error_rate = 0.00;
    links[1].error_rate = (float)getRandomvalue(0, 1)/100;
    //links[1].error_rate = 0.00;
    printf("LINKS -> error values: \nLink0 - %f, Link1 - %f\n",   links[0].error_rate,   links[1].error_rate);

//    for (int i = 0; i < 2; i++) {
//        links[i].id = i;
//        links[i].error_rate = (float) (getRandomvalue(0, 100)/ 100);
//        printf("\n link - error rate = %f",links[i].error_rate);
//        links[i].ready = 1;
//        links[i].bps = getRandomvalue(50000, 200000000);
//        links[i].max_jitter_ms = getRandomvalue(0, 100);
//        int latency = getRandomvalue(0, 300);
//        links[i].latency.tv_sec = latency / 1000;
//        links[i].latency.tv_usec = (latency % 1000) * 1000;
//    }

    uint32_t pkts_coded = 1;
    uint32_t pkts_sent = 0;
    uint32_t pkt_decoded = 1;
    uint32_t pkts_lost = 0;
    uint32_t pkts_rcv_rec = 0;
    uint32_t pkts_coded_rec = 1;
    uint32_t pkts_put_dec = 1;



    for (int i = 0; i < num_pkts; i++) {

        //START ENCODING
        uint8_t payload[source_size - sizeof(uint32_t)];
        for (uint32_t j = 0; j < source_size - sizeof(uint32_t); j++) {
            payload[j] = (uint8_t) rand();
        }
        skb_new(&skb_buff_enc, buffer_enc_source, sizeof(buffer_enc_source));
        skb_reserve(&skb_buff_enc, sizeof(uint32_t));
        skb_put(&skb_buff_enc, skb_tailroom(&skb_buff_enc));
        memcpy(skb_buff_enc.data, payload, enc.source_size - sizeof(uint32_t));
        skb_push_u32(&skb_buff_enc, (uint32_t )i);
        while (nck_full(&enc)) {
            while (nck_has_coded(&enc)) {
                struct sk_buff pkt_coded;
                skb_new(&pkt_coded, buffer_enc_coded, sizeof(buffer_enc_coded));
                nck_get_coded(&enc, &pkt_coded);
                int received = drand48() >= links[0].error_rate;
                if (received) {
                    nck_put_coded(&rec , &pkt_coded);
                } else {
                    pkts_lost ++;
                    printf("\nLost packet -> pkts lost: %d", pkts_lost);
                }
                pkts_coded ++;
                pkts_sent += 1;
            }
        }
        nck_put_source(&enc, &skb_buff_enc);

        //get coded packets and send it to recoder

        while (nck_has_coded(&enc)) {
            skb_new(&skb_buff_enc1, buffer_enc_coded1, sizeof(skb_buff_enc1));
            nck_get_coded(&enc, &skb_buff_enc1);
            int received = drand48() >= links[0].error_rate;
            pkts_coded ++;
            pkts_sent += 1;
            // printf("\nENC - pkt sent %d", pkts_sent);
            if (received) {
                if (!nck_put_coded(&rec, &skb_buff_enc1)) {
                    printf("\nReceived packet IN LINK 0 -> pkts recv: %d", pkts_rcv_rec);
                    pkts_rcv_rec ++;
                }
            }
            else {
                pkts_lost ++;
                printf("\nLost packet IN LINK 0 -> pkts lost: %d", pkts_lost);
            }
        }

        while (nck_has_feedback(&rec)) {
            skb_new(&skb_feedback, buffer_feedback, sizeof(buffer_feedback));
            nck_get_feedback(&rec, &skb_feedback);
            int received = drand48() >= links[0].error_rate;
            if (received) {
                nck_put_feedback(&enc, &skb_feedback);
            }
            else {
                printf("\nLost FEEDBACK PACKET IN LINK 0");
            }
        }

        // Recoding...
        while (nck_has_coded(&rec)) {
            skb_new(&sk_buff_rec, buffer_enc_coded1, sizeof(buffer_enc_coded));
            nck_get_coded(&rec, &sk_buff_rec);
            //printf("\nREC - pkt coded %d", pkts_coded_rec);
            pkts_coded_rec ++;
            int received = drand48() >= links[1].error_rate;
            if (received) {
                //printf("\nDEC - rcvd pkt %d", pkts_put_dec);
                nck_put_coded(&dec, &sk_buff_rec);
                pkts_put_dec ++;
            }
            else {
                pkts_lost ++;
                printf("\nLost packet IN LINK 1 -> pkts lost: %d", pkts_lost);
            }
        }


        while (nck_has_feedback(&dec)) {
            skb_new(&skb_feedback2, buffer_feedback2, sizeof(buffer_feedback2));
            nck_get_feedback(&dec, &skb_feedback2);
            int received = drand48() >= links[1].error_rate;
            if (received) {
                nck_put_feedback(&rec, &skb_feedback2);
            }
            else {
                printf("\nLost FEEDBACK PACKET IN LINK 0");
            }
        }

        while (nck_has_source(&dec)) {
            uint8_t buffer[rec.coded_size];
            memset(buffer, 0, rec.coded_size);
            skb_new(&sk_buff_dec, buffer, dec.source_size);
            nck_get_source(&dec, &sk_buff_dec);
            uint32_t seq = skb_pull_u32(&sk_buff_dec);
            printf("\npkt decded %d seqno %d\n", pkt_decoded, seq);
            pkt_decoded++;
        }
    }
    //while (pkt_decoded < num_pkts) {
    while (nck_has_coded(&rec)) {
        skb_new(&sk_buff_rec, buffer_enc_coded1, sizeof(buffer_enc_coded));
        nck_get_coded(&rec, &sk_buff_rec);
        int received = drand48() >= links[1].error_rate;
        if (received) {
            nck_put_coded(&dec, &sk_buff_rec);
            pkts_coded_rec++;
            //printf("\nREC - pkt coded %d", pkts_coded_rec);
        }
    }
    nck_flush_source(&dec);

    while (nck_has_source(&dec)) {
        uint8_t buffer[rec.coded_size];
        memset(buffer, 0, rec.coded_size);
        skb_new(&sk_buff_dec, buffer, dec.source_size);
        nck_get_source(&dec, &sk_buff_dec);
        uint32_t seq = skb_pull_u32(&sk_buff_dec);
        printf("\npkt decded %d - seqno %d\n", pkt_decoded, seq);
        pkt_decoded++;
    }

    if (pkt_decoded >= num_pkts) {
        printf("\nPACKETS HAVE BEEN DECODED");
    }
}





