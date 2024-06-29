C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
ASM_SOURCES = $(wildcard $(SRC_DIR)/*.s)

C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS = $(patsubst $(SRC_DIR)/%.s,$(OBJ_DIR)/%.o,$(ASM_SOURCES))

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS) $(DEVICE_OBJECTS)

all: $(BINARY_NAME).bin
.DEFAULT_GOAL=all

# Speed up compiles by not deleting these
.PRECIOUS: $(OBJ_DIR)/%.o
.PRECIOUS: $(OBJ_DIR)/%.elf

%.bin: $(OBJ_DIR)/%.elf
	@echo Creating binary $@
	@$(OBJCOPY) -O binary $< $@

$(OBJ_DIR)/%.elf: $(OBJECTS)
	@echo Linking $@
	@$(LD) $(LDFLAGS) -o $@ $^
	@echo ==============================================================================
	@$(SIZE) $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	@echo Compiling $<
	@$(AS) $(ASFLAGS) -o $@ $<

clean:
	-del $(BINARY_NAME).bin
	-rmdir /Q /S $(OBJ_DIR)

$(shell mkdir $(OBJ_DIR))

.PHONY: all clean
