# CH32V307 LED Blink Makefile
# Toolchain: riscv-none-embed-gcc (MounRiver Studio)
# Install: export PATH="/home/dennis/software/riscv-none-elf/bin:$PATH"

# Project name
TARGET = LED_Blink

# Build path
BUILD_DIR = build

# Source files
C_SOURCES =  \
User/main.c \
User/ch32v30x_it.c \
User/system_ch32v30x.c

# CH32V30x Standard Peripheral Library
LIB_SOURCES = \
LIB/Peripheral/src/ch32v30x_gpio.c \
LIB/Peripheral/src/ch32v30x_rcc.c \
LIB/Peripheral/src/ch32v30x_usart.c \
LIB/Peripheral/src/ch32v30x_misc.c \
LIB/Peripheral/src/ch32v30x_dbgmcu.c \
LIB/Debug/debug.c

# ASM sources
ASM_SOURCES = \
LIB/Startup/startup_ch32v30x_D8C.S

# Include paths
C_INCLUDES =  \
-IUser \
-ILIB/Peripheral/inc \
-ILIB/Core \
-ILIB/Debug

# Compiler
PREFIX = riscv-none-embed-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

# MCU flags
# CH32V307: rv32imac + zicsr + zifencei, ilp32
MCU = -march=rv32imac_zicsr_zifencei -mabi=ilp32

# C flags
CFLAGS = $(MCU) $(C_INCLUDES) -Wall -fdata-sections -ffunction-sections -specs=nano.specs
CFLAGS += -DCH32V30x_D8C

# Debug flags
ifeq ($(DEBUG), 1)
CFLAGS += -Og -g -gdwarf-2
else
CFLAGS += -Os
endif

# Linker script
LDSCRIPT = Ld/Link.ld

# Libraries
LIBS = -lc -lm -lnosys
LIBDIR =

# Linker flags
LDFLAGS = $(MCU) -nostartfiles -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# Build rules
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

# List of C objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# List of library objects
LIB_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(LIB_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(LIB_SOURCES)))

# List of ASM objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LIB_OBJECTS)
	$(CC) $(OBJECTS) $(LIB_OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	-rm -fR $(BUILD_DIR)

# Flash using WCH-Link
flash: $(BUILD_DIR)/$(TARGET).bin
	wch-isp flash -a 0x00000000 $<

.PHONY: all clean flash
