CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lpthread

TARGET = pid_tid_test
SRC = pid_tid_test.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean
