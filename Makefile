TARGET = tbostrings
LIBS = 
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -O3
LDFLAGS = 

.PHONY: default all clean install

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $@
    
install: $(TARGET)
	install $(TARGET) /usr/local/bin

clean:
	-rm -f *.o
	-rm -f $(TARGET)
