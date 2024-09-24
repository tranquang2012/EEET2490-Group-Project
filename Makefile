# Directories
SRC_DIR = ./src
BUILD_DIR = ./obj

# Source and object files
CFILES = $(wildcard $(SRC_DIR)/*.c)
OFILES = $(CFILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OFILES := $(filter-out $(BUILD_DIR)/uart.o $(BUILD_DIR)/uart0.o, $(OFILES)) # Exclude both by default

# Raspberry Pi version
RPI_VER = -DRPI_VERSION=3

# Compiler flags
GCCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib $(RPI_VER)

# Default UART settings
UART_FLAG = -DUSE_UART1
UART_SOURCE = $(SRC_DIR)/uart.c

# Phony targets
.PHONY: all uart0 uart1 clean run

# Default target (builds with UART1)
all: kernel8.img

# Target for UART0 build
uart0: UART_FLAG = -DUSE_UART0
uart0: UART_SOURCE = $(SRC_DIR)/uart0.c
uart0: kernel8.img

# Target for UART1 build
uart1: UART_FLAG = -DUSE_UART1
uart1: UART_SOURCE = $(SRC_DIR)/uart.c
uart1: kernel8.img

# Object file compilation for boot.S
$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.S
	aarch64-none-elf-gcc $(GCCFLAGS) -c $(SRC_DIR)/boot.S -o $(BUILD_DIR)/boot.o

# Compile selected UART file with UART_FLAG
$(BUILD_DIR)/uart.o: $(UART_SOURCE)
	aarch64-none-elf-gcc $(GCCFLAGS) $(UART_FLAG) -c $(UART_SOURCE) -o $(BUILD_DIR)/uart.o

# General rule for all other object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	aarch64-none-elf-gcc $(GCCFLAGS) $(UART_FLAG) -c $< -o $@

# Linker script and kernel image generation
kernel8.img: $(BUILD_DIR)/boot.o $(OFILES) $(BUILD_DIR)/uart.o
	aarch64-none-elf-ld -nostdlib $(BUILD_DIR)/boot.o $(OFILES) $(BUILD_DIR)/uart.o -T $(SRC_DIR)/link.ld -o $(BUILD_DIR)/kernel8.elf
	aarch64-none-elf-objcopy -O binary $(BUILD_DIR)/kernel8.elf kernel8.img

# Clean the build directory and generated files
clean:
	del /f kernel8.elf $(subst /,\,$(BUILD_DIR))\*.o kernel8.img

# Run QEMU with the kernel image
run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio

# Run QEMU with UART0
uart0-run: uart0
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio

# Run QEMU with UART1
uart1-run: uart1
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio
