#include <functional>
#include "apps/kv/client.h"

namespace specpaxos {
namespace kv {

using namespace proto;

KVClient::KVClient(Transport *transport,
                   Client *client)
    : transport(transport), client(client)
{
    this->replied = false;
    this->transportThread = new std::thread(&KVClient::RunTransport, this);
}

KVClient::~KVClient()
{
    this->transport->Stop();
    this->transportThread->join();
    delete this->transportThread;
}

bool
KVClient::Read(const std::string &key, std::string &value)
{
    KVRequest request;
    KVReply reply;
    std::string request_str, reply_str;

    request.mutable_read()->set_key(key);
    request.SerializeToString(&request_str);
    reply_str = Invoke(request_str, nullptr, 0);

    reply.ParseFromString(reply_str);
    value = reply.value();
    return reply.status() == KVReply::SUCCESS;
}

bool
KVClient::Write(const std::string &key, const std::string &value)
{
    KVRequest request;
    KVReply reply;
    std::string request_str, reply_str;

    request.mutable_write()->set_key(key);
    request.mutable_write()->set_value(value);
    request.SerializeToString(&request_str);
    reply_str = Invoke(request_str, nullptr, 0);

    reply.ParseFromString(reply_str);
    return reply.status() == KVReply::SUCCESS;
}

std::string
KVClient::Invoke(const std::string &request,
                 void *app_header,
                 size_t app_header_len)
{
    this->transport->Timer(0, [ = ]() {
        this->client->Invoke(request,
                             bind(&KVClient::InvokeCallback,
                                  this,
                                  std::placeholders::_1,
                                  std::placeholders::_2),
                             app_header,
                             app_header_len);
    });
    this->replied = false;
    std::unique_lock<std::mutex> lck(this->mtx);
    while (!this->replied) {
        this->cv.wait(lck);
    }
    return this->reply;
}

void
KVClient::InvokeCallback(const std::string &request, const std::string &reply)
{
    std::unique_lock<std::mutex> lck(this->mtx);
    this->reply = reply;
    this->replied = true;
    this->cv.notify_all();
}

void
KVClient::RunTransport()
{
    this->transport->Run();
}

} // namespace kv
} // namespace specpaxos
