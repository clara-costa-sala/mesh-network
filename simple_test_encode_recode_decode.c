//
// Created by clara on 22.01.19.
//

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

#include "../../src/private.h"

/*nck_create_encoder:
 * Configures a generic encoder structure as a specific encoder implementation.
 * @encoder: Encoder structure to configure
 * @timer: Timer implementation that will be used by the coder
 * @context: Contextual object that will be passed to get_opt
 * @get_opt: Function used to get configuration values for the coder
 * @return: Returns 0 on success*/

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

struct nodes_struct {
    int id;
    uint32_t sent_pkts;
    uint32_t recv_pkts;
    struct nck_recoder *rec;
};

struct send_struct {
    struct link_struct *link;
    struct nck_recoder *rec;
    struct nck_decoder *dec;
    int rec_int;
};

int getRandomvalue(int min, int max);
static void receive(struct nck_timer_entry *handle, void *context, int success);

int main() {

    uint16_t num_recoders = 1;

    struct nck_encoder enc;
    struct nck_recoder recoders[num_recoders];
    struct nck_decoder dec;

    // ENC ----- REC ----- DEC

    struct nck_schedule schedule;
    struct timeval step;
    struct nck_timer timer;
    struct timespec clock;
    srand(time(NULL));
    struct sk_buff skb_buff_enc, sk_buff_dec;
    struct sk_buff skb_buffers[num_recoders];
    uint32_t seq_gen_num = 0;
    uint32_t global_seq_num = 0;
    int num_pkts = 1000;

    nck_schedule_init(&schedule);
    clock_gettime(CLOCK_MONOTONIC, &clock);
    schedule.time.tv_sec = clock.tv_sec;
    schedule.time.tv_usec = clock.tv_nsec / 1000;
    nck_schedule_timer(&schedule, &timer);


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

    if (nck_create_encoder(&enc, &timer, options_enc, nck_option_from_array)) {
        fprintf(stderr, "Failed to create encoder");
        return -1;
    }
    fprintf(stderr, "\nEncoder created");

    for (int i = 0; i < num_recoders; i++) {
        if (nck_create_recoder(&(recoders[i]), &timer, options_rec, nck_option_from_array)) {
            fprintf(stderr, "Failed to create recoder");
            return -1;
        }
    }

    fprintf(stderr, "\nRecoders created\n");

    if (nck_create_decoder(&dec, &timer, options_dec, nck_option_from_array)) {
        fprintf(stderr, "Failed to create decoder");
        return -1;
    }
    fprintf(stderr, "\nDecoder created\n");

    // **************************** INITIALIZE ************************

    uint8_t buffer_enc_source[enc.source_size];
    memset(buffer_enc_source, 0, enc.source_size);

    uint8_t buffer_enc_source1[enc.source_size];
    memset(buffer_enc_source1, 0, enc.source_size);

    uint8_t buffer_enc_source2[enc.source_size];
    memset(buffer_enc_source2, 0, enc.source_size);

    uint8_t buffer_rec_source0[recoders[0].source_size];
    memset(buffer_rec_source0, 0, recoders[0].source_size);

    uint8_t buffer_rec_source1[recoders[0].source_size];
    memset(buffer_rec_source1, 0, recoders[0].source_size);

    uint8_t buffer_rec_coded0[recoders[0].coded_size];
    memset(buffer_rec_coded0, 0, recoders[0].coded_size);

    uint8_t buffer_rec_coded1[recoders[0].coded_size];
    memset(buffer_rec_coded1, 0, recoders[0].coded_size);

    uint8_t buffer_dec_source0[dec.source_size];
    memset(buffer_dec_source0, 0, dec.source_size);

    uint32_t pkts_coded = 0;
    uint32_t pkts_sent = 0;

    /************************* initialize links ********************/
    struct link_struct links[num_recoders + 1];

    for (int i = 0; i < num_recoders + 1; i++) {
        links[i].id = i;
        links[i].error_rate = (float) (getRandomvalue(0, 100)/ 100);
        links[i].ready = 1;
        links[i].bps = getRandomvalue(50000, 200000000);
        links[i].max_jitter_ms = getRandomvalue(0, 100);
        int latency = getRandomvalue(0, 300);
        links[i].latency.tv_sec = latency / 1000;
        links[i].latency.tv_usec = (latency % 1000) * 1000;
    }

    //START ENCODING ***********************************************
    for (int i = 0; i < num_pkts; i++) {
        uint8_t payload[enc.source_size - sizeof(uint32_t)];
        for (uint32_t j = 0; j < enc.source_size - sizeof(uint32_t); j++) {
            payload[j] = (uint8_t) rand();
        }
        skb_new(&skb_buff_enc, buffer_enc_source, sizeof(buffer_enc_source));
        skb_reserve(&skb_buff_enc, sizeof(uint32_t));
        skb_put(&skb_buff_enc, skb_tailroom(&skb_buff_enc));
        memcpy(skb_buff_enc.data, payload, enc.source_size - sizeof(uint32_t));
        skb_push_u32(&skb_buff_enc, pkts_coded);
        while (nck_full(&enc)) {
            //sendCodedPkt
            while (nck_has_coded(&enc)) {
                // next node to send is recoders[0];
                if (links[0].ready) {
                    uint8_t buffer[enc.coded_size];
                    memset(buffer, 0, enc.coded_size);
                    struct sk_buff pkt_coded;
                    skb_new(&pkt_coded, buffer, enc.coded_size);
                    nck_get_coded(&enc, &pkt_coded);

                    //compute delay + jitter:
                    struct timeval delay;
                    uint64_t bits = (uint64_t) pkt_coded.len*8;
                    delay.tv_sec = bits / links[0].bps;
                    delay.tv_usec = (bits*1000000) / links[0].bps;
                    double actual_jitter = links[0].max_jitter_ms*((drand48()*2 - 1));
                    if (actual_jitter == 0){
                        delay.tv_sec += links[0].latency.tv_sec;
                        delay.tv_usec += links[0].latency.tv_usec;
                    } else {
                        delay.tv_sec += links[0].latency.tv_sec + (links[0].max_jitter_ms / 1000);
                        delay.tv_usec += links[0].latency.tv_usec + ((links[0].max_jitter_ms - delay.tv_sec * 1000) * 1000);
                    }

                    //put packet in link
                    links[0].packet = pkt_coded;
                    links[0].ready = 0;

                    struct send_struct *s;
                    s->link = &links[0];
                    s->rec = &recoders[0];

                    nck_timer_add(&timer, &delay, s, &receive);

                    pkts_sent += 1;
                }
            }
        }
    }

    //ALL PKTS SRC HAVE BEEN SENT


}

static void receive(struct nck_timer_entry *handle, void *context, int success) {
    struct send_struct *s = (struct send_struct *)context;
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