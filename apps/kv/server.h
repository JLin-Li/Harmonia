#ifndef __KV_SERVER_H__
#define __KV_SERVER_H__

#include <unordered_map>
#include "common/replica.h"
#include "apps/kv/kv-proto.pb.h"

namespace specpaxos {
namespace kv {

class KVServer : public AppReplica
{
public:
    KVServer() {};
    ~KVServer() {};

    void ReplicaUpcall(opnum_t opnum, const string &str1, string &str2,
                       void *arg = nullptr, void *ret = nullptr) override;

private:
    void HandleRead(const proto::ReadMessage &msg, proto::KVReply &reply);
    void HandleWrite(const proto::WriteMessage &msg, proto::KVReply &reply);

    std::unordered_map<std::string, std::string> store;
};

} // namespace kv
} // namespace specpaxos

#endif /* __KV_SERVER_H__ */
