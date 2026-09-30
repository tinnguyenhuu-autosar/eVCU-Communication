# Makefile - COM ECU
# STM32F103

TARGET = com_ecu
BUILD_DIR = build

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# ============================================================
# Compiler flags
# ============================================================

CFLAGS  = -mcpu=cortex-m3 -mthumb -O0 -g3
CFLAGS += -Wall -Wextra
CFLAGS += -ffreestanding
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -DSTM32F10X_MD
CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -DRUN_ON_RENODE

LDFLAGS = -T ../.vscode/hala-sim-generated/stm32f103c8.ld
LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=nosys.specs
LDFLAGS += -Wl,--gc-sections

# ============================================================
# Include paths
# ============================================================

INC  = -I../autosar/include
INC += -I../autosar/can
INC += -I../autosar/canif
INC += -I../autosar/pdur
INC += -Iautosar/Com
INC += -I../config
INC += -Iconfig
INC += -I../spl/inc
INC += -I../bsp/cmsis

# ============================================================
# Source files
# ============================================================

SRCS = \
	main.c \
	autosar/Com/Com.c \
	../autosar/PduR/PduR.c \
	../autosar/Canif/CanIf.c \
	../autosar/Can/Can.c \
	config/Com_Cfg.c \
	../config/CanIf_Cfg.c \
	../config/PduR_Cfg.c \
	../config/Can_Cfg.c \
	../spl/src/stm32f10x_rcc.c \
	../spl/src/stm32f10x_gpio.c \
	../spl/src/stm32f10x_can.c \
	../spl/src/misc.c \
	../spl/src/system_stm32f10x.c\
	../.vscode/hala-sim-generated/startup_stm32f103.c

# ============================================================
# Object files
# ============================================================

OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
OBJS += $(patsubst %.s,$(BUILD_DIR)/%.o,$(ASM_SRCS))

# ============================================================
# Default target
# ============================================================

all: $(TARGET).elf $(TARGET).bin $(TARGET).hex

# ============================================================
# Link
# ============================================================

$(TARGET).elf: $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	@echo ""
	@echo "=============================================="
	@echo " BUILD SUCCESS: $(TARGET).elf"
	@echo "=============================================="
	$(SIZE) $@

# ============================================================
# Compile C
# ============================================================

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

# ============================================================
# Compile Assembly
# ============================================================

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================
# Binary / HEX
# ============================================================

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# ============================================================
# Clean
# ============================================================

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).hex

.PHONY: all clean