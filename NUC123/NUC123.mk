SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += -I${SELF_DIR}inc
INCLUDES += -I${SELF_DIR}StdDriver/inc

LDFLAGS += -T ${SELF_DIR}NUC123.ld

$(OBJ_DIR)/startup_NUC123.o: $(SELF_DIR)startup_NUC123.s
	@echo Compiling $<
	@$(AS) $(ASFLAGS) -o $@ $<

$(OBJ_DIR)/system_NUC123.o: $(SELF_DIR)system_NUC123.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/_syscalls.o: $(SELF_DIR)_syscalls.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PRECIOUS: $(OBJ_DIR)/NUC123_%.o
$(OBJ_DIR)/NUC123_%.o: $(SELF_DIR)StdDriver/src/%.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

DEVICE_OBJECTS :=
DEVICE_OBJECTS += $(OBJ_DIR)/system_NUC123.o
ifneq ($(USE_CUSTOM_STARTUP_ASSEMBLY), 1)
	DEVICE_OBJECTS += $(OBJ_DIR)/startup_NUC123.o
endif
DEVICE_OBJECTS += $(OBJ_DIR)/_syscalls.o

$(foreach module,$(LIBRARY_MODULES),$(eval DEVICE_OBJECTS += $(OBJ_DIR)/NUC123_$(module).o))
