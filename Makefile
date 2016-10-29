.PHONY: clean

CC 		= gcc
CFLAGS += `pkg-config --cflags libmongoc-1.0` -LLIBDIR -Wall
LDFLAGS += `pkg-config --libs libmongoc-1.0`
SOURCES = cache.c meta.c mcache.c
OBJ 	= $(SOURCES:.c=.o)
TARGET 	= mcache

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(LDFLAGS)

clean:
	-rm -f $(TARGET) *.o
