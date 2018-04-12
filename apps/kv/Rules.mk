d := $(dir $(lastword $(MAKEFILE_LIST)))

SRCS += $(addprefix $(d), \
	client.cc server.cc)

PROTOS += $(addprefix $(d), \
	  kv-proto.proto)

OBJS-kv-client := $(o)client.o $(o)kv-proto.o
OBJS-kv-server := $(o)server.o $(o)kv-proto.o

include $(d)tests/Rules.mk
