CC = gcc
CFLAGS = -Wall -Werror -O2
LDFLAGS = -pthread
TARGET = pthread_stack
SOURCE = pthread_stack.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	# -pthread handles both compilation and linking against the Pthreads library
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
