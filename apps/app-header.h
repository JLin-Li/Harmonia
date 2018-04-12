#ifndef __APP_HEADER_H__
#define __APP_HEADER_H__

typedef uint8_t apptype_t;
enum class AppType {
    KV = 0,
    UNKNOWN = 1
};

/*
 * KV app header
 * format: AppType (8) + KVOp (8) + key (null terminated)
 */
typedef uint8_t kvop_t;
enum class KVOp {
    READ = 0,
    WRITE = 1
};
const size_t KV_HEADER_BASE_LEN = sizeof(apptype_t) + sizeof(kvop_t);

#endif /* __APP_HEADER_H__ */
