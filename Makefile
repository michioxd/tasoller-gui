SRC_DIR := src
OBJ_DIR := obj
BINARY_NAME := host_aprom

# OPTIM := -O0
OPTIM := -O1
# OPTIM := -Os -flto
include GCC.mk
LIBRARY_MODULES := clk uart timer i2c
include NUC123/NUC123.mk

include generic.mk
