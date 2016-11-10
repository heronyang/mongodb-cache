.PHONY: clean test

CC 			= gcc
CFLAGS 		+= `pkg-config --cflags libmongoc-1.0` -LLIBDIR -Wall
LDFLAGS 	+= `pkg-config --libs libmongoc-1.0`
SOURCES 	= mcache.c cache.c meta.c
OBJ 		= $(SOURCES:.c=.o)
TARGET 		= mcache

TEST_SOURCE = mcache-test.c cache.c meta.c
TEST_TARGET = mcache-test
TEST_OBJ	= $(TEST_SOURCE:.c=.o)

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(LDFLAGS)

clean:
	-rm -f $(TARGET) *.o
