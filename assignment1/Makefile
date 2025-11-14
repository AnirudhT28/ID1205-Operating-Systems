# 1. Variables
# --------------------
# Define the compiler
CC = gcc

# Define compiler flags
CFLAGS = -Wall -Wextra -std=c11 -g

# Define the name of your source file(s)
SRCS = question_8.c

# Define the name of your final executable
TARGET = question_8

# Define linker flags
# -lm = link math library
# -lrt = link real-time library (for clock_gettime)
LDFLAGS = -lm -lrt 


# 2. Rules
# --------------------

# The '.PHONY' target tells 'make' that 'all' and 'clean' are not
# actual files to be created.
.PHONY: all clean

# The 'all' rule is the default.
all: $(TARGET)

# Rule for building the target executable
$(TARGET): $(SRCS)
	# This command MUST start with a TAB
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

# The 'clean' rule removes the built files.
clean:
	# This command MUST start with a TAB
	-rm -f $(TARGET)
