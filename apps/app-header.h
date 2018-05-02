#ifndef __APP_HEADER_H__
#define __APP_HEADER_H__

typedef uint8_t apptype_t;
static const apptype_t APPTYPE_KV = 0;
typedef uint32_t opid_t;

/*
 * KV app header
 * format: AppType (8) + KVOp (8) + OpID (32) + key (null terminated)
 */
typedef uint8_t kvop_t;
static const kvop_t KVOP_READ = 0;
static const kvop_t KVOP_WRITE = 1;
static const kvop_t KVOP_READ_ONE = 2;

static const size_t KV_HEADER_BASE_LEN = sizeof(apptype_t) + sizeof(kvop_t) + sizeof(opid_t);

/*
 * Sequencer packet header
 */
typedef uint32_t identifier_t;
static const identifier_t SEQUENCER_ID = 0xDEADBEEF;
typedef uint8_t seqtype_t;
static const seqtype_t SEQTYPE_RESET = 0;
static const seqtype_t SEQTYPE_BEG_SYNC = 1;
static const seqtype_t SEQTYPE_FIN_SYNC = 2;
static const seqtype_t SEQTYPE_COMPLETE_SYNC = 3;
typedef uint16_t nkeys_t;

// sync begin format: identifier (8) + seqtype (8) + timestamp (64) (filled by sequencer)
static const size_t BEG_SYNC_LEN = sizeof(identifier_t) + sizeof(seqtype_t) + sizeof(msgnum_t);
// sync begin reply format: identifier (8) + seqtype (8) + timestamp (64)
static const size_t BEG_SYNC_REPLY_LEN = sizeof(identifier_t) + sizeof(seqtype_t) + sizeof(msgnum_t);
// sync finish format: identifier (8) + seqtype (8) + timestamp (64)
static const size_t FIN_SYNC_LEN = sizeof(identifier_t) + sizeof(seqtype_t) + sizeof(msgnum_t);
// sync finish reply format: identifier (8) + seqtype (8) + timestamp (64)
static const size_t FIN_SYNC_REPLY_LEN = sizeof(identifier_t) + sizeof(seqtype_t) + sizeof(msgnum_t);
// sync complete format: identifier (8) + seqtype (8) + nkeys (16) + key1 + key2 + ...
static const size_t COMPLETE_SYNC_LEN = sizeof(identifier_t) + sizeof(seqtype_t) + sizeof(nkeys_t);
static const size_t MAX_COMPLETE_SYNC_LEN = 1400;
static const size_t COMPLETE_SYNC_LEN_THRESHOLD = 1200;

#endif /* __APP_HEADER_H__ */
