d := $(dir $(lastword $(MAKEFILE_LIST)))

include $(d)kv/Rules.mk
