CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -pthread

TARGET = pthread_sum
SRC = pthread_sum.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
