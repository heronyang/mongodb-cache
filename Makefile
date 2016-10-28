.PHONY: clean

CC 		= gcc
CFLAGS 	= -Wall
SOURCES = cache.c meta.c mcache.c
OBJ 	= $(SOURCES:.c=.o)
TARGET 	= mcache

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(LDFLAGS)

clean:
	-rm $(TARGET) $(OBJ)
