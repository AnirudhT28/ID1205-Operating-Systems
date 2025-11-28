CC = gcc
CFLAGS = -Wall -Werror -O2
LDFLAGS = -pthread
TARGET = stack_app
SOURCE = stack_app.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	# -pthread handles both compilation and linking against the Pthreads library
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
