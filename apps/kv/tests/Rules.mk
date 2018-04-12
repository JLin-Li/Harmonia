d := $(dir $(lastword $(MAKEFILE_LIST)))

GTEST_SRCS += $(d)kv-test.cc

COMMON-OBJS := $(OBJS-kv-client) $(OBJS-kv-server) $(LIB-simtransport) $(GTEST_MAIN)

$(d)kv-test: $(o)kv-test.o $(COMMON-OBJS) $(OBJS-nopaxos-client) $(OBJS-nopaxos-replica)

TEST_BINS += $(d)kv-test
