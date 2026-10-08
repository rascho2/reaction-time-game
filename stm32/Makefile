# TODO add path of CMSIS and HAL base directory
CMSIS	= ../esho1_task6/task6_stm32/STM32CubeF0/Drivers/CMSIS
HAL		= ../esho1_task6/task6_stm32/STM32CubeF0/Drivers/STM32F0xx_HAL_Driver

BUILDDIR = $(PWD)/build
SRCDIR = $(PWD)/src
INCDIR = $(PWD)/include

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

INCLUDE		=  -I$(CMSIS)/Include
INCLUDE		+= -I$(CMSIS)/Device/ST/STM32F0xx/Include
INCLUDE		+= -I$(HAL)/Inc
INCLUDE		+= -I$(INCDIR)

# Flags - Overall Options
CPPFLAGS += -specs=nosys.specs

# Flags - C Language Options
# CFLAGS += -ffreestanding

# Flags - C++ Language Options
CXXFLAGS += -fno-threadsafe-statics
CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-exceptions
CXXFLAGS += -fno-unwind-tables
# CXXFLAGS += -std=gnu++11

# Flags - Warning Options
CPPFLAGS += -Wall
CPPFLAGS += -Wextra
CPPFLAGS += -Wundef

# Flags - Debugging Options
# CPPFLAGS += -g
CPPFLAGS += -ggdb

# Flags - Optimization Options
CPPFLAGS += -ffunction-sections
CPPFLAGS += -fdata-sections
CPPFLAGS += -fno-common
CPPFLAGS += -O2
CPPFLAGS += -fomit-frame-pointer 
# CPPFLAGS += -falign-functions=16
# Be careful, link time optimizations might cause problems...
# CPPFLAGS += -flto

# Flags - Preprocessor options
CPPFLAGS += -DSTM32F030xC

# Flags - Machine-dependant options
CPPFLAGS += -mcpu=cortex-m0
CPPFLAGS += -mlittle-endian
CPPFLAGS += -mthumb
CPPFLAGS += -mfloat-abi=soft
CPPFLAGS += -masm-syntax-unified

# Flags - Linker Options
# CPPFLAGS += -nostdlib
LDFLAGS	+= -Tlinker/STM32F030xC_FLASH.ld -nostartfiles -nostdlib
# LDFLAGS += --gc-sections

CFLAGS		+= $(CPPFLAGS) $(CXXFLAGS)
AFLAGS		+= $(CFLAGS)

CSRC = $(shell find $(SRCDIR) -name "*.c")
CSRC += $(CMSIS)/Device/ST/STM32F0xx/Source/Templates/system_stm32f0xx.c
HAL_SRC := $(shell find $(HAL)/Src/*.c ! -name '*_template.c')
CSRC += $(HAL_SRC)

ASRC = startup/startup_stm32f030xc.s
ELFFILE = main.elf

COBJS = $(patsubst %, $(BUILDDIR)%,$(CSRC:.c=.o))
AOBJS = $(patsubst %, $(BUILDDIR)/%,$(ASRC:.s=.o))

all: setup build

setup:
	@mkdir -p $(BUILDDIR)

build: setup $(COBJS) $(AOBJS)
	$(LD) $(CFLAGS) $(LDFLAGS) -o $(ELFFILE) $(COBJS) $(AOBJS)

$(COBJS): $(BUILDDIR)%.o : %.c
	mkdir -p $(dir $@)
	$(CPPC) $(INCLUDE) $(CFLAGS) -c $< -o $@

$(AOBJS): $(BUILDDIR)/%.o : %.s
	mkdir -p $(dir $@)
	$(CPPC) $(INCLUDE) $(AFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)

flash: build
	# TODO add openocd command to program ELF file

.PHONY: all setup build clean flash
