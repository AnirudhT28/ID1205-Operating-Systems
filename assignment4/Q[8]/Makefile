# Compiler to use
CC = gcc

# Compiler flags: -Wall (show all warnings), -pthread (required for semaphores/sync)
CFLAGS = -Wall -pthread

# The name of your source file and the resulting executable
TARGET = mmap_fork
SRC = mmap_fork.c

# Default rule: build the executable
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Rule to create the 1MB file required for the assignment
file:
	dd if=/dev/zero of=file_to_map.txt bs=1M count=1

# Rule to clean up the directory
clean:
	rm -f $(TARGET) file_to_map.txt
