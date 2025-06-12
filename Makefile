CC = gcc
CFLAGS = -Os -Wall -s
TARGET = lilstatus

$(TARGET): lilstatus.c
	$(CC) $(CFLAGS) -o $(TARGET) lilstatus.c

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: clean install