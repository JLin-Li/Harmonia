#include "lib/udptransport.h"
#include "lib/configuration.h"
#include "lib/transport.h"
#include "apps/app-header.h"

#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <string>

class Controller : public TransportReceiver
{
public:
    Controller(specpaxos::Configuration *config,
               UDPTransport *transport)
        : transport(transport)
    {
        transport->Register(this, *config, -1, -1);
    };
    ~Controller() {};

    void ReceiveMessage(const TransportAddress &remote,
                        const string &type,
                        const string &data,
                        void *meta_data) override { };

    void Run()
    {
        char buf[sizeof(identifier_t) + sizeof(seqtype_t)];
        memcpy(buf, &SEQUENCER_ID, sizeof(identifier_t));
        *(seqtype_t *)(buf + sizeof(identifier_t)) = SEQTYPE_RESET;
        this->transport->SendMessageToSequencer(this, (void*)buf, sizeof(identifier_t) + sizeof(seqtype_t));
    }

private:
    UDPTransport *transport;
};

int main(int argc, char **argv)
{
    const char *configPath = nullptr;

    // Parse arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
        case 'c': {
            configPath = optarg;
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

    Controller controller(&config, &transport);
    controller.Run();

    return 0;
}
