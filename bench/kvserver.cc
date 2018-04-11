#include "lib/configuration.h"
#include "common/replica.h"
#include "lib/udptransport.h"
#include "nopaxos/replica.h"
#include "apps/kv/server.h"

#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

int
main(int argc, char **argv)
{
    int index = -1;
    const char *configPath = NULL;

    // Parse arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:i:")) != -1) {
        switch (opt) {
        case 'c': {
            configPath = optarg;
            break;
        }

        case 'i': {
            index = stoi(std::string(optarg));
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
    if (index == -1) {
        fprintf(stderr, "option -i is required\n");
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

    if (index >= config.n) {
        fprintf(stderr, "replica index %d is out of bounds; "
                "only %d replicas defined\n", index, config.n);
        exit(1);
    }

    UDPTransport transport;
    specpaxos::kv::KVServer server;

    specpaxos::nopaxos::NOPaxosReplica replica(config,
                                               index,
                                               false,
                                               &transport,
                                               &server);
    transport.Run();
}
