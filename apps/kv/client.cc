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

    void *app_header;
    size_t app_header_len;
    ConstructAppHeader(KVOP_READ, key, &app_header, app_header_len);
    reply_str = Invoke(request_str, app_header, app_header_len);
    free(app_header);

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

    void *app_header;
    size_t app_header_len;
    ConstructAppHeader(KVOP_WRITE, key, &app_header, app_header_len);
    reply_str = Invoke(request_str, app_header, app_header_len);
    free(app_header);

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

void
KVClient::ConstructAppHeader(kvop_t op, const std::string &key, void **app_header, size_t &app_header_len)
{
    app_header_len = KV_HEADER_BASE_LEN + key.length() + 1; // +1 for terminating null
    *app_header = malloc(app_header_len);
    char *ptr = (char *)*app_header;
    *(apptype_t *)ptr = APPTYPE_KV;
    ptr += sizeof(apptype_t);
    *(kvop_t *)ptr = op;
    ptr += sizeof(kvop_t);
    memcpy(ptr, key.c_str(), key.length() + 1);
}

} // namespace kv
} // namespace specpaxos
