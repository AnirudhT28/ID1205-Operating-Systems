CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -pthread

TARGET = question_6
SRC = question_6.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
