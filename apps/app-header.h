#ifndef __APP_HEADER_H__
#define __APP_HEADER_H__

typedef uint8_t apptype_t;
static const apptype_t APPTYPE_KV = 0;

/*
 * KV app header
 * format: AppType (8) + KVOp (8) + key (null terminated)
 */
typedef uint8_t kvop_t;
static const kvop_t KVOP_READ = 0;
static const kvop_t KVOP_WRITE = 1;
static const kvop_t KVOP_READ_ONE = 2;

const size_t KV_HEADER_BASE_LEN = sizeof(apptype_t) + sizeof(kvop_t);

#endif /* __APP_HEADER_H__ */
