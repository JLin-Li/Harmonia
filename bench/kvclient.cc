#include "lib/assert.h"
#include "lib/message.h"
#include "lib/udptransport.h"
#include "lib/configuration.h"
#include "lib/latency.h"
#include "lib/timeval.h"

#include "apps/kv/client.h"
#include "nopaxos/client.h"
#include "bench/header.h"

#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>

int rand_key(int nkeys);

int main(int argc, char **argv)
{
    const char *configPath = NULL;
    const char *keysPath = NULL;
    int duration = 10, nkeys = 100, readRatio = 100;
    std::vector<std::string> keys;

    // Parse arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:d:k:p:r:")) != -1) {
        switch (opt) {
        case 'c': {
            configPath = optarg;
            break;
        }
        case 'd': {
            duration = stoi(std::string(optarg));
            break;
        }
        case 'k': {
            nkeys = stoi(std::string(optarg));
            break;
        }
        case 'p': {
            keysPath = optarg;
            break;
        }
        case 'r': {
            readRatio = stoi(std::string(optarg));
            break;
        }
        default:
            fprintf(stderr, "Unknown argument %s\n", argv[optind]);
            break;
        }
    }

    if (!configPath) {
        fprintf(stderr, "option -c is required\n");
        exit(1);
    }

    // Load configuration
    std::ifstream configStream(configPath);
    if (configStream.fail()) {
        fprintf(stderr, "unable to read configuration file: %s\n",
                configPath);
        exit(1);
    }
    specpaxos::Configuration config(configStream);

    // Read in keys
    string key;
    std::ifstream in;
    in.open(keysPath);
    if (!in) {
        fprintf(stderr, "Could not read keys from: %s\n", keysPath);
        exit(1);
    }
    for (int i = 0; i < nkeys; i++) {
        getline(in, key);
        keys.push_back(key);
    }
    in.close();

    UDPTransport transport;
    specpaxos::nopaxos::NOPaxosClient protoClient(config, &transport);
    specpaxos::kv::KVClient kvClient(&transport, &protoClient);

    // Benchmark
    Phase phase = WARMUP;
    struct timeval start, now, m1, m2;
    struct Latency_t latency;
    std::vector<uint64_t> latencies;
    uint64_t nreads = 0, nwrites = 0, total_latency = 0;

    _Latency_Init(&latency, "op");
    latencies.reserve(duration * 10000);
    gettimeofday(&start, nullptr);

    while (true) {
        gettimeofday(&now, nullptr);
        uint64_t time_elapsed = now.tv_sec - start.tv_sec;

        if (phase == WARMUP) {
            if (time_elapsed >= (uint64_t)duration / 4) {
                phase = MEASURE;
                m1 = now;
            }
        } else if (phase == MEASURE) {
            if (time_elapsed >= (uint64_t)duration * 3 / 4) {
                phase = COOLDOWN;
                m2 = now;
            }
        } else if (phase == COOLDOWN) {
            if (time_elapsed >= (uint64_t)duration) {
                break;
            }
        }

        Latency_Start(&latency);
        std::string key = keys[rand_key(nkeys)];
        std::string value;
        int op_select = rand() % 100;
        if (op_select < readRatio) {
            kvClient.Read(key, value);
            if (phase == MEASURE) {
                nreads++;
            }
        } else {
            value = key;
            kvClient.Write(key, value);
            if (phase == MEASURE) {
                nwrites++;
            }
        }
        uint64_t ns = Latency_End(&latency);

        if (phase == MEASURE) {
            latencies.push_back(ns);
            total_latency += (ns / 1000);
        }
    }

    struct timeval diff = timeval_sub(m2, m1);
    uint64_t total_ops = latencies.size();

    Notice("Completed %lu ops in " FMT_TIMEVAL_DIFF " seconds",
           total_ops, VA_TIMEVAL_DIFF(diff));
    Notice("Read proportion: %.2f", (float)nreads / total_ops);
    Notice("Write proportion: %.2f", (float)nwrites / total_ops);

    char buf[1024];
    std::sort(latencies.begin(), latencies.end());

    uint64_t ns  = latencies[total_ops / 2];
    LatencyFmtNS(ns, buf);
    Notice("Median latency is %ld ns (%s)", ns, buf);
    Notice("Average latency is %ld us", total_latency / total_ops);

    ns = latencies[total_ops * 90 / 100];
    LatencyFmtNS(ns, buf);
    Notice("90th percentile latency is %ld ns (%s)", ns, buf);

    ns = latencies[total_ops * 95 / 100];
    LatencyFmtNS(ns, buf);
    Notice("95th percentile latency is %ld ns (%s)", ns, buf);

    ns = latencies[total_ops * 99 / 100];
    LatencyFmtNS(ns, buf);
    Notice("99th percentile latency is %ld ns (%s)", ns, buf);

    return 0;
}

int rand_key(int nkeys)
{
    return rand() % nkeys;
}
