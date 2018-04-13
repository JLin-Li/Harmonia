#ifndef __KV_CLIENT_H__
#define __KV_CLIENT_H__

#include <thread>
#include <mutex>
#include <condition_variable>
#include "common/client.h"
#include "apps/app-header.h"
#include "apps/kv/kv-proto.pb.h"

namespace specpaxos {
namespace kv {

class KVClient
{
public:
    KVClient(Transport *transport,
             Client *client);
    ~KVClient();

    bool Read(const std::string &key, std::string &value);
    bool Write(const std::string &key, const std::string &value);

private:
    Transport *transport;
    Client *client;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread *transportThread;
    std::string reply;
    bool replied;

    std::string Invoke(const std::string &request,
                       void *app_header,
                       size_t app_header_len);
    void InvokeCallback(const std::string &request, const std::string &reply);
    void RunTransport();
    void ConstructAppHeader(kvop_t op, const std::string &key, void **app_header, size_t &app_header_len);
};

} // namespace kv
} // namespace specpaxos

#endif /* __KV_CLIENT_H__ */
