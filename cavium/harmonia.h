#ifndef __HARMONIA_H__
#define __HARMONIA_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define HARMONIA_PROC

/*
 * Packet macros
 */
#define ETH_DST     0
#define ETH_SRC     6
#define IP_SIZE     20
#define IP_HEADER   14
#define IP_CKSUM    24
#define IP_SRC      26
#define IP_DST      30
#define UDP_SRC     34
#define UDP_DST     36
#define UDP_CKSUM   40
#define APP_HEADER  42

/*
 * KV message header fields and offsets
 */
typedef uint32_t identifier_t;
typedef uint32_t meta_len_t;
typedef uint16_t udp_port_t;
typedef uint64_t sess_num_t;
typedef uint32_t ngroups_t;
typedef uint32_t group_num_t;
typedef uint64_t msg_num_t;
typedef uint16_t appheader_len_t;
typedef uint8_t apptype_t;
typedef uint8_t kvop_t;

static const identifier_t HARMONIA_ID = 0x20050318;
static const identifier_t RESET_ID = 0xDEADBEEF;
static const apptype_t APPTYPE_KV = 0;
static const kvop_t KVOP_READ = 0;
static const kvop_t KVOP_WRITE = 1;
static const kvop_t KVOP_READ_ONE = 2;

/*
 * Controller message header fields
 */

/*
 * Data structures
 */
typedef enum {
    ORDERED_MCAST,
    RESET,
    UNKNOWN
} packet_type_t;

/*
 * Pegasus packet processing functions
 */
int harmonia_init();
void harmonia_packet_proc(uint64_t buf);

#endif /* __HARMONIA_H__ */
