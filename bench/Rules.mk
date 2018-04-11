d := $(dir $(lastword $(MAKEFILE_LIST)))

SRCS += $(addprefix $(d), \
	client.cc benchmark.cc replica.cc kvclient.cc kvserver.cc)

OBJS-benchmark := $(o)benchmark.o \
                  $(LIB-message) $(LIB-latency)

$(d)client: $(o)client.o $(OBJS-spec-client) $(OBJS-vr-client) $(OBJS-fastpaxos-client) $(OBJS-unreplicated-client) $(OBJS-nopaxos-client) $(OBJS-benchmark) $(LIB-udptransport)

$(d)replica: $(o)replica.o $(OBJS-spec-replica) $(OBJS-vr-replica) $(OBJS-fastpaxos-replica) $(OBJS-unreplicated-replica) $(OBJS-nopaxos-replica) $(LIB-udptransport)

$(d)kvclient: $(o)kvclient.o $(OBJS-nopaxos-client) $(OBJS-kv-client) $(LIB-udptransport)

$(d)kvserver: $(o)kvserver.o $(OBJS-nopaxos-replica) $(OBJS-kv-server) $(LIB-udptransport)

BINS += $(d)client $(d)replica $(d)kvclient $(d)kvserver
