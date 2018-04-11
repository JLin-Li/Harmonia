#include "lib/assert.h"
#include "lib/message.h"
#include "lib/udptransport.h"

#include "apps/kv/client.h"
#include "lib/configuration.h"
#include "nopaxos/client.h"

#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <string>

int main(int argc, char **argv)
{
    const char *configPath = NULL;
    const char *keysPath = NULL;
    int duration = 10;

    // Parse arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:d:k:")) != -1) {
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
            keysPath = optarg;
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

    UDPTransport transport;
    specpaxos::nopaxos::NOPaxosClient protoClient(config, &transport);
    specpaxos::kv::KVClient kvClient(&transport, &protoClient);
    return 0;
}
