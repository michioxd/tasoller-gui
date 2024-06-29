PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
AS = $(PREFIX)gcc  # Rather than AS, so we get preprocessing
AR = $(PREFIX)ar
LD = $(PREFIX)gcc  # To pull in stdlib as needed
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

SPECS := -specs=./picolibc/lib/gcc/arm-none-eabi/13.2.1/picolibc.specs

INCLUDES += -I./picolibc/arm-none-eabi/picolibc/arm-none-eabi/include

OPTIM ?= -flto -Os
GCCFLAGS := $(SPECS) -nolibc -nostdlib -nostartfiles -nodefaultlibs -mcpu=cortex-m0 -mthumb -Wl,--gc-sections $(OPTIM) -g

CFLAGS := $(GCCFLAGS) -c -ffunction-sections -fdata-sections -fsigned-char \
         -fmessage-length=0 -ffreestanding
ASFLAGS := $(GCCFLAGS) -c -x assembler-with-cpp
LDFLAGS := $(GCCFLAGS) -Lpicolibc/arm-none-eabi/picolibc/arm-none-eabi/lib/thumb/v6-m/nofp
