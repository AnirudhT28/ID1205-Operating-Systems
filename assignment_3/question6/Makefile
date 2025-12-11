# Compiler variable
CC = gcc

# Compiler flags (-Wall shows warnings, -g adds debug info)
CFLAGS = -Wall -g

# The name of your executable
TARGET = question_6

# Default target: compile the program
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

# Helper target to run the assignment test specifically
run: $(TARGET)
	/usr/bin/time --verbose ./$(TARGET) 128

# Clean up: remove the executable
clean:
	rm -f $(TARGET)
