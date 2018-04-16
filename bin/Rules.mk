d := $(dir $(lastword $(MAKEFILE_LIST)))

SRCS += $(addprefix $(d), controller.cc)

$(d)controller: $(o)controller.o $(LIB-udptransport)

BINS += $(d)controller
