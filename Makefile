.PHONY: clean

CC 			= gcc
CFLAGS 		+= `pkg-config --cflags libmongoc-1.0` -LLIBDIR -Wall
LDFLAGS 	+= `pkg-config --libs libmongoc-1.0` -pthread
SOURCES 	= cached.c cache.c meta.c wrapper.c
OBJ 		= $(SOURCES:.c=.o)
TARGET 		= cached

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f $(TARGET) *.o
