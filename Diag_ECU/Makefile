# ============================================================
# Diagnostic ECU - STM32F103
# ============================================================

TARGET := diag_ecu

# ------------------------------------------------------------
# Toolchain
# ------------------------------------------------------------

CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size


# ------------------------------------------------------------
# Compiler flags
# ------------------------------------------------------------

CFLAGS := \
    -mcpu=cortex-m3 \
    -mthumb \
    -O0 \
    -g3 \
    -Wall \
    -Wextra \
    -ffreestanding \
    -ffunction-sections \
    -fdata-sections \
    -DSTM32F10X_MD \
    -DUSE_STDPERIPH_DRIVER \
    -DCANIF_USE_CANTP \
	-DPDUR_USE_CANTP


# ------------------------------------------------------------
# Linker
# ------------------------------------------------------------

LDFLAGS := \
    -T ../.vscode/hala-sim-generated/stm32f103c8.ld \
    --specs=nano.specs \
    --specs=nosys.specs \
    -Wl,--gc-sections


# ------------------------------------------------------------
# Include paths
# ------------------------------------------------------------

INCLUDES := \
    -I. \
    -I./autosar/CanTp \
    -I./autosar/Dcm \
    -I./config \
    -I../config \
	-I../autosar/include \
    -I../autosar/Can \
    -I../autosar/Canif \
    -I../autosar/PduR \
    -I../Diag_ECU/config \
    -I../spl/inc \
    -I../bsp/cmsis


# ------------------------------------------------------------
# Source files
# ------------------------------------------------------------

SRCS := \
    main.c \
    autosar/CanTp/CanTp.c \
    autosar/Dcm/Dcm.c \
    ../autosar/Can/Can.c \
    ../autosar/Canif/CanIf.c \
    ../autosar/PduR/PduR.c \
    config/CanTp_Cfg.c \
    ../config/Can_Cfg.c \
    ../config/CanIf_Cfg.c \
    ../config/PduR_Cfg.c \
    ../spl/src/stm32f10x_rcc.c \
    ../spl/src/stm32f10x_gpio.c \
    ../spl/src/stm32f10x_can.c \
    ../spl/src/misc.c \
    ../spl/src/system_stm32f10x.c \
    ../.vscode/hala-sim-generated/startup_stm32f103.c


# ------------------------------------------------------------
# Object files
# ------------------------------------------------------------

OBJS := $(SRCS:.c=.o)


# ------------------------------------------------------------
# Default target
# ------------------------------------------------------------

all: $(TARGET).elf


# ------------------------------------------------------------
# Link
# ------------------------------------------------------------

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@


# ------------------------------------------------------------
# Compile
# ------------------------------------------------------------

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


# ------------------------------------------------------------
# Clean
# ------------------------------------------------------------

clean:
	rm -f $(OBJS)
	rm -f $(TARGET).elf