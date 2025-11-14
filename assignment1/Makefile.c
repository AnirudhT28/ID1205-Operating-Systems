
CC = gcc


CFLAGS = -Wall -Wextra -std=c11 -g

 //Define the name of your source file(s)
SRCS = question_8.c

// Define the name of your final executable
TARGET = question_8


LDFLAGS = -lm -lrt 



.PHONY: all clean

all: $(TARGET)


$(TARGET): $(SRCS)
	
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)


clean:

	-rm -f $(TARGET)