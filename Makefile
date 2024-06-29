SRC_DIR := src
OBJ_DIR := obj
BINARY_NAME := host_aprom

OPTIM := -O0
# OPTIM := -Ofast -flto
# OPTIM := -Os -flto
include GCC.mk
LIBRARY_MODULES := clk uart timer i2c
include NUC123/NUC123.mk

CFLAGS += -Wno-gnu-variable-sized-type-not-at-end

include generic.mk
