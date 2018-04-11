#include "apps/kv/server.h"
#include "lib/message.h"

namespace specpaxos {
namespace kv {

using namespace proto;

void
KVServer::ReplicaUpcall(opnum_t opnum, const string &str1, string &str2, void *arg, void *ret)
{
    KVRequest request;
    KVReply reply;
    request.ParseFromString(str1);

    switch (request.msg_case()) {
    case KVRequest::MsgCase::kRead: {
        this->HandleRead(request.read(), reply);
        break;
    }
    case KVRequest::MsgCase::kWrite: {
        this->HandleWrite(request.write(), reply);
        break;
    }
    default:
        Panic("Unexpected KVRequest format");
    }

    reply.SerializeToString(&str2);
}

void
KVServer::HandleRead(const ReadMessage &msg, KVReply &reply)
{
    if (this->store.count(msg.key()) == 0) {
        reply.set_status(KVReply::FAILED);
    } else {
        reply.set_status(KVReply::SUCCESS);
        reply.set_value(this->store.at(msg.key()));
    }
}

void
KVServer::HandleWrite(const WriteMessage &msg, KVReply &reply)
{
    reply.set_status(KVReply::SUCCESS);
    this->store[msg.key()] = msg.value();
}

} // namespace kv
} // namespace specpaxos
