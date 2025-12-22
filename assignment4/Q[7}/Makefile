# Compiler to use
CC = gcc

# Compiler flags
# -pthread: Required for the threading library
# -Wall: Warn about potential issues in code
# -g: Add debugging information (useful if you need gdb)
CFLAGS = -pthread -Wall -g

# The name of the executable to build
TARGET = hw4_io_perf

# The source file
SRC = hw4_io_perf.c

# Default target: build the executable
all: $(TARGET)

# Rule to link the program
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Rule to clean up the directory
# Removes the executable and the temporary data file created by the program
clean:
	rm -f $(TARGET) test_data.bin
