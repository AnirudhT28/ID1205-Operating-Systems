CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -pthread

TARGET = question_8
SRC = question_8.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)