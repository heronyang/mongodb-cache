.PHONY: clean operation-test

CC 			= gcc
CFLAGS 		+= `pkg-config --cflags libmongoc-1.0` -LLIBDIR -Wall
LDFLAGS 	+= `pkg-config --libs libmongoc-1.0` -pthread
SOURCES 	= cached.c cache.c operation.c meta.c wrapper.c
OBJ 		= $(SOURCES:.c=.o)
TARGET 		= cached

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

operation-test: operation-test.c operation.c cache.c meta.c wrapper.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

clean:
	-rm -f $(TARGET) *.o operation-test
