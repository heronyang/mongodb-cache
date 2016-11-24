.PHONY: proto cached-test cache-test clean

CC 				= gcc

PB_CFLAGS		= `pkg-config --cflags 'libprotobuf-c >= 1.0.0'`
PB_LDFLAGS		= `pkg-config --libs 'libprotobuf-c >= 1.0.0'` -lprotobuf-c

MONGOD_CLFLAGS	= `pkg-config --cflags libmongoc-1.0` -LLIBDIR
MONGOD_LDFLAGS	= `pkg-config --libs libmongoc-1.0`

CFLAGS 			+= $(PB_CFLAGS) $(MONGOD_CLFLAGS) -Wall
LDFLAGS 		+= $(PB_LDFLAGS) $(MONGOD_LDFLAGS) -pthread
SOURCES 		= cached.c cache.c wrapper.c helper.c proto/meta.pb-c.c proto/operation.pb-c.c
OBJ 			= $(SOURCES:.c=.o)
TARGET 			= cached

PROTO_DIR		= "./proto/"

TEST_SOURCES    = mcache.c proto/*.c wrapper.c helper.c
TEST_TARGET 	= mcache
TEST_CFLAGS 	= $(PB_CFLAGS)
TEST_LDFLAGS 	= $(PB_LDFLAGS) -pthread

all: proto $(TARGET) cached-test

proto:
	$(MAKE) -C $(PROTO_DIR)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

cached-test:
	$(CC) $(TEST_CFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET) $(TEST_LDFLAGS)

cache-test: proto
	$(CC) $(CFLAGS) wrapper.c cache.c cache-test.c proto/*.c helper.c -o cache-test $(LDFLAGS)

clean:
	$(MAKE) clean -C $(PROTO_DIR)
	-rm -f $(TARGET) *.o cache-test proto/*.o
