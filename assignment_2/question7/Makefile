CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -pthread

TARGET = question_7
SRC = question_7.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)