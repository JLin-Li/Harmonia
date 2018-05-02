#include <string.h>
#include <stdlib.h>
#include "nic_mem.h"
#include "hash_table.h"
#include "harmonia.h"

/*
 * Hard-coded addresses
 */
static const uint32_t HARMONIA_ADDR = 0x0A0A01FF;

/*
 * Global variables
 */
static msg_num_t counter;
static sess_num_t sess_num;
static uint32_t last_replica;
#define N_REPLICAS 3
static concurrent_ht_t *modified_keys_completed = NULL;
static concurrent_ht_t *modified_keys_started = NULL;
static sync_pt_t sync_pt_completed;
static sync_pt_t sync_pt_started;

/*
 * Static function declarations
 */
static void convert_endian(void *dst, const void *src, size_t n);
static uint64_t checksum(const void *buf, size_t n);
static int init();
static packet_type_t match_harmonia_packet(uint64_t buf);
static void process_ordered_multicast(uint64_t buf);
static void process_sequencer(uint64_t buf);
static int process_kv(uint64_t buf);

/*
 * Static function definitions
 */
static void convert_endian(void *dst, const void *src, size_t n)
{
  size_t i;
  uint8_t *dptr, *sptr;

  dptr = (uint8_t*)dst;
  sptr = (uint8_t*)src;

  for (i = 0; i < n; i++) {
    *(dptr + i) = *(sptr + n - i - 1);
  }
}

static uint64_t checksum(const void *buf, size_t n)
{
    uint64_t sum;
    uint16_t *ptr = (uint16_t *)buf;
    size_t i;
    for (i = 0, sum = 0; i < (n / 2); i++) {
        sum += *ptr++;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

static int init()
{
    printf("Harmonia initializing...\n");
    sess_num = 0;
    counter = 0;
    last_replica = 0;
    sync_pt_completed = 0;
    sync_pt_started = 0;

    if (modified_keys_completed != NULL) {
        concur_hashtable_free(modified_keys_completed);
    }
    modified_keys_completed = concur_hashtable_init(0);
    if (modified_keys_started != NULL) {
        concur_hashtable_free(modified_keys_started);
    }
    modified_keys_started = concur_hashtable_init(0);
    return 0;
}

static packet_type_t match_harmonia_packet(uint64_t buf)
{
    if (*(uint32_t *)(buf + IP_DST) == HARMONIA_ADDR) {
        identifier_t identifier;
        convert_endian(&identifier, (const void *)(buf+APP_HEADER), sizeof(identifier_t));
        if (identifier == HARMONIA_ID) {
            return ORDERED_MCAST;
        } else if (identifier == SEQUENCER_ID) {
            return SEQUENCER;
        } else {
            return UNKNOWN;
        }
    } else {
        return UNKNOWN;
    }
}

static void process_ordered_multicast(uint64_t buf)
{
    udp_port_t udp_src = (1 << N_REPLICAS) - 1;
    int update_counter = 1;
    uint64_t ptr = buf + APP_HEADER;
    ptr += sizeof(identifier_t) + sizeof(meta_len_t);
    *(udp_port_t*)ptr = *(udp_port_t*)(buf+UDP_SRC);
    ptr += sizeof(udp_port_t);
    convert_endian((void*)ptr, &sess_num, sizeof(sess_num_t));
    ptr += sizeof(sess_num_t);
    ptr += sizeof(ngroups_t) + sizeof(group_num_t);
    msg_num_t *msg_num = (msg_num_t *)ptr;
    ptr += sizeof(msg_num_t);
    appheader_len_t header_len;
    convert_endian(&header_len, (void*)ptr, sizeof(appheader_len_t));

    if (header_len > 0) {
        ptr += sizeof(appheader_len_t);
        apptype_t apptype = *(apptype_t *)ptr;
        ptr += sizeof(apptype_t);
        if (apptype == APPTYPE_KV) {
            update_counter = process_kv(ptr);
        }
    }

    if (update_counter) {
        counter++;
        convert_endian((void*)msg_num, &counter, sizeof(msg_num_t));
    } else {
        udp_src = 1 << last_replica;
        last_replica = (last_replica + 1) % N_REPLICAS;
    }
    *(udp_port_t*)(buf+UDP_SRC) = udp_src;
    *(uint16_t *)(buf + UDP_CKSUM) = 0;
}

static int process_kv(uint64_t buf)
{
    ht_status ret;
    kvop_t *kvop = (kvop_t *)buf;
    buf += sizeof(kvop_t);
    // XXX Currently not using opid
    buf += sizeof(opid_t);
    char *key = (char *)buf;

    if (*kvop == KVOP_WRITE) {
        uint8_t val = 1;
        ret = concur_hashtable_insert(modified_keys_completed, key, &val, sizeof(uint8_t));
        if (ret == HT_INSERT_FAILURE_TABLE_FULL) {
            printf("ERROR: failed to insert into modified_keys_sc\n");
        }
        /*
        ret = concur_hashtable_insert(modified_keys_started, key, &val, sizeof(uint8_t));
        if (ret == HT_INSERT_FAILURE_TABLE_FULL) {
            printf("ERROR: failed to insert into modified_keys_ss\n");
        }
        */
        return 1;
    } else if (*kvop == KVOP_READ) {
        uint8_t *val;
        size_t val_len;
        ret = concur_hashtable_find(modified_keys_completed, key, (void **)&val, &val_len);

        if (ret == HT_FOUND) {
            // Conflict detected!
            return 1;
        } else {
            // Safe to do single replica read
            *kvop = KVOP_READ_ONE;
            return 0;
        }
    } else {
        printf("ERROR: wrong KV packet format\n");
        return 1;
    }
}

static void process_sequencer(uint64_t buf)
{
    uint64_t ptr = buf + APP_HEADER + sizeof(identifier_t);
    seqtype_t type = *(seqtype_t *)ptr;
    ptr += sizeof(seqtype_t);
    if (type == SEQTYPE_RESET) {
        init();
    } else if (type == SEQTYPE_BEG_SYNC) {
        if (modified_keys_started != NULL) {
            concur_hashtable_free(modified_keys_started);
        }
        modified_keys_started = concur_hashtable_init(0);
        convert_endian((void *)ptr, &counter, sizeof(msg_num_t));
        sync_pt_started = counter;
    } else if (type == SEQTYPE_FIN_SYNC) {
        msg_num_t ts;
        convert_endian(&ts, (void *)ptr, sizeof(msg_num_t));
        if (ts == sync_pt_started && ts != sync_pt_completed) {
            if (modified_keys_completed != NULL) {
                concur_hashtable_free(modified_keys_completed);
            }
            modified_keys_completed = modified_keys_started;
            modified_keys_started = concur_hashtable_init(0);
            sync_pt_completed = sync_pt_started;
            sync_pt_started = 0;
        }
    } else if (type == SEQTYPE_COMPLETE_SYNC) {
        nkeys_t nkeys;
        convert_endian(&nkeys, (void *)ptr, sizeof(nkeys_t));
        ptr += sizeof(nkeys_t);
        nkeys_t i;
        for (i = 0; i < nkeys; i++) {
            char *key = (char *)ptr;
            concur_hashtable_delete(modified_keys_completed, key);
            ptr += strlen(key) + 1;
        }
    }
}

/*
 * Public function definitions
 */
int harmonia_init()
{
#ifdef USE_NIC_MEMORY
    nic_local_shared_mm_init();
#endif
    return init();
}

void harmonia_packet_proc(uint64_t buf)
{
    switch (match_harmonia_packet(buf)) {
        case ORDERED_MCAST: {
            process_ordered_multicast(buf);
            break;
        }
        case SEQUENCER: {
            process_sequencer(buf);
            break;
        }
        default:
            return;
    }
}
