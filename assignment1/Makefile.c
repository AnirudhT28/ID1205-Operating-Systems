CC = gcc
	
LDFLAGS = -lm -lrt
	
CFLAGS = -Wall -Wextra -std=c11 -g

SRCS = question_8.c


TARGET = question_8


.PHONY: all clean


all: $(TARGET)


$(TARGET): $(SRCS)

	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	# The '-' in front of 'rm' means 'make' will not
	# stop if it fails (e.g., if the file isn't there).
	-rm -f $(TARGET)
