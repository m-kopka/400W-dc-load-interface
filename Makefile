
# RP2040 Makefile
# Martin Kopka 2022
#-----------------------------------------------------------------------------------------------------------------------------------------------------------------

# CPU core
CPU = cortex-m0plus

#-----------------------------------------------------------------------------------------------------------------------------------------------------------------

# build target name
TARGET = build/build

# list folders containing C and Assembly source files here
SRC_DIRS = . lib/rp2040-hal/src/ lib/rp2040-hal/src/* lib/mini-kernel/src/ lib/mini-kernel/src/* src src/*

# list folders containing header files here
INC_DIRS = . lib/rp2040-hal/include/ lib/mini-kernel/include/ ./include/

# linker script path
LINKER_SCRIPT = lib/rp2040-hal/rp2040_ls.ld

#-----------------------------------------------------------------------------------------------------------------------------------------------------------------

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
OPENOCD = openocd

OPENOCD_INTERFACE = interface/picoprobe.cfg
OPENOCD_TARGET    = target/rp2040.cfg

# optimization flag
OPT = s

# compiler flags
CFLAGS = -Wall -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-variable -mcpu=$(CPU) -mthumb -std=gnu11 -pipe -O$(OPT) -ggdb -fno-builtin -nodefaultlibs -nostartfiles $(foreach D,$(INC_DIRS), -I$(D)) -MP -MD

# linker flags
LFLAGS = -mcpu=$(CPU) -mthumb -nostdlib -pipe -T $(LINKER_SCRIPT) -Wl,-Map=$(TARGET).map

#-----------------------------------------------------------------------------------------------------------------------------------------------------------------

# find all C source files
CFILES  = $(foreach D,$(SRC_DIRS),$(wildcard $(D)/*.c))

# find all Assembly source files
SFILES  = $(foreach D,$(SRC_DIRS),$(wildcard $(D)/*.S))

#define all object files
OFILES  = $(patsubst %.c,%.o,$(CFILES))
OFILES += $(patsubst %.S,%.o,$(SFILES))

#define all dependency files
DFILES  = $(patsubst %.c,%.d,$(CFILES))
DFILES += $(patsubst %.S,%.d,$(SFILES))

#=================================================================================================================================================================

all: build flash

#---- BUILD ------------------------------------------------------------------------------------------------------------------------------------------------------

build: $(TARGET).elf

# create object files from C source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# create object files from Assembly source files
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# link object files to ELF file
$(TARGET).elf: $(OFILES)
	$(CC) $(LFLAGS) $^ -o $@
	$(OBJCOPY) -O binary -j .text $@ $(TARGET).bin
	$(OBJDUMP) -d $@ > $(TARGET).dis

#---- FLASH ------------------------------------------------------------------------------------------------------------------------------------------------------

flash: $(TARGET).elf
	$(OPENOCD) -f $(OPENOCD_INTERFACE) -f $(OPENOCD_TARGET) -c "program $< verify reset exit"

#---- CLEAN ------------------------------------------------------------------------------------------------------------------------------------------------------

clean:
	rm -rf build/* $(OFILES) $(DFILES)

c: clean

#-----------------------------------------------------------------------------------------------------------------------------------------------------------------

-include $(DFILES)