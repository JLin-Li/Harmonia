#include <vector>
#include <map>
#include <gtest/gtest.h>
#include "lib/simtransport.h"
#include "apps/kv/client.h"
#include "apps/kv/server.h"
#include "nopaxos/client.h"
#include "nopaxos/replica.h"

using namespace specpaxos;
using namespace specpaxos::kv;
using namespace specpaxos::nopaxos;

class KVTest : public ::testing::Test
{
protected:
    std::vector<KVServer *> servers;
    std::vector<NOPaxosReplica *> replicas;
    KVClient *kvClient;
    NOPaxosClient *protoClient;
    SimulatedTransport *transport;
    Configuration *config;

    virtual void SetUp() {
        std::map<int, std::vector<ReplicaAddress> > nodeAddrs =
        {
            {
                0,
                {
                    { "localhost", "12300" },
                    { "localhost", "12301" },
                    { "localhost", "12302" }
                }
            }
        };

        this->config = new Configuration(1, 3, 1, nodeAddrs);
        this->transport = new SimulatedTransport(true);

        for (int i = 0; i < config->n; i++) {
            KVServer *server = new KVServer;
            NOPaxosReplica *replica = new NOPaxosReplica(*config, i, true, transport, server);
            this->servers.push_back(server);
            this->replicas.push_back(replica);
        }

        this->protoClient = new NOPaxosClient(*config, transport);
        this->kvClient = new KVClient(transport, this->protoClient);
    }

    virtual void TearDown() {
        this->transport->Stop();
        delete this->kvClient;
        delete this->protoClient;
        for (auto replica : this->replicas) {
            delete replica;
        }
        for (auto server : this->servers) {
            delete server;
        }
        delete transport;
        delete config;
    }
};

TEST_F(KVTest, SimpleWriteReadTest) {
    EXPECT_TRUE(this->kvClient->Write("k1", "v1"));
    for (const auto replica : this->replicas) {
        EXPECT_EQ(replica->log.LastOpnum(), 1);
    }
    std::string value;
    EXPECT_TRUE(this->kvClient->Read("k1", value));
    EXPECT_EQ(value, "v1");
    for (const auto replica : this->replicas) {
        EXPECT_EQ(replica->log.LastOpnum(), 2);
    }
    EXPECT_FALSE(this->kvClient->Read("k2", value));
    EXPECT_EQ(value, "");
    bool executed = false;
    for (const auto replica : this->replicas) {
        if (executed) {
            EXPECT_EQ(replica->log.LastOpnum(), 2);
        } else {
            if (replica->log.LastOpnum() == 3) {
                executed = true;
            } else {
                EXPECT_EQ(replica->log.LastOpnum(), 2);
            }
        }
    }
}
