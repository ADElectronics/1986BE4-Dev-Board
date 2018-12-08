######################################
# target
######################################

TARGET = Firmware

######################################
# building variables
######################################

# debug build?
DEBUG = 1

# optimization
#OPT = -Os
#OPT = -O0 -ggdb3
OPT = -O1

#######################################
# paths
#######################################

BUILD_DIR = build

######################################
# source
######################################

C_SOURCES =  \
Project/main.c \
Project/syscalls.c \
Project/delay.c \
Project/lcd_st7789.c \
Project/lcd_fonts.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_adc.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_adciu.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_bkp.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_comp.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_crc.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_dac.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_dma.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_eeprom.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_iwdg.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_port.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_power.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_rst_clk.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_ssp.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_timer.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_uart.c \
MDR1986BE4_StdPeriph_Driver/src/MDR32F9Qx_wwdg.c \
CMSIS/CM0/DeviceSupport/MDR1986BE4/startup/gcc/system_MDR1986BE4.c

ASM_SOURCES =  \
CMSIS/CM0/DeviceSupport/MDR1986BE4/startup/gcc/startup_MDR1986BE4.s

#######################################
# binaries
#######################################
GCC_PATH ?= ~/gcc-arm-none-eabi-7-2017-q4-major/bin
PREFIX = arm-none-eabi-

ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################

CPU = -mcpu=cortex-m0
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

AS_DEFS = 

C_DEFS =  \
-DUSE_MDR1986BE4 

AS_INCLUDES =  \
-I\Project

C_INCLUDES =  \
-IProject \
-IMDR1986BE4_StdPeriph_Driver/inc \
-IConfig \
-ICMSIS/CM0/CoreSupport \
-ICMSIS/CM0/DeviceSupport/MDR1986BE4/startup \
-ICMSIS/CM0/DeviceSupport/MDR1986BE4/startup/gcc \
-ICMSIS/CM0/DeviceSupport/MDR1986BE4/inc

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = CMSIS/CM0/DeviceSupport/MDR1986BE4/startup/MDR1986BE4.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections
# Для работы float в printf, sprintf - занимает много места!!! Если не используете - лучше закомментировать.
# LDFLAGS += -u _printf_float

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@echo $<
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo $<
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)
