.PHONY: proto cached-test cache-test clean

CC 				= g++

MONGOD_CLFLAGS	= `pkg-config --cflags libmongoc-1.0` -LLIBDIR
MONGOD_LDFLAGS	= `pkg-config --libs libmongoc-1.0`

CFLAGS 			+= `pkg-config --cflags protobuf` $(MONGOD_CLFLAGS) -Wall -g
LDFLAGS 		+= `pkg-config --libs protobuf` $(MONGOD_LDFLAGS) -pthread -lprotobuf
SOURCES 		= cached.cc cache.cc wrapper.cc bcon-wrapper.cc helper.cc proto/meta.pb.cc proto/operation.pb.cc
OBJ 			= $(SOURCES:.cc=.o)
TARGET 			= cached

PROTO_DIR		= "./proto/"

TEST_SOURCES    = proto/*.cc wrapper.cc helper.cc
TEST_CFLAGS 	= -g
TEST_LDFLAGS 	= -pthread

all: proto $(TARGET) cached-test

proto:
	$(MAKE) -C $(PROTO_DIR)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.cc
	$(CC) $(CFLAGS) -c $< -o $@

cached-test: proto
	$(CC) $(TEST_CFLAGS) $(CFLAGS) $(TEST_SOURCES) mcache.cc -o mcache $(TEST_LDFLAGS) $(LDFLAGS)
	$(CC) $(TEST_CFLAGS) $(CFLAGS) $(TEST_SOURCES) mcache-test.cc -o mcache-test $(TEST_LDFLAGS) $(LDFLAGS)

cache-test: proto
	$(CC) $(CFLAGS) wrapper.cc bcon-wrapper.cc cache.cc cache-test.cc proto/*.cc helper.cc -o cache-test $(LDFLAGS)

test: proto
	$(CC) $(TEST_CFLAGS) $(CFLAGS) $(TEST_SOURCES) test.cc -o test $(TEST_LDFLAGS) $(LDFLAGS)

clean:
	$(MAKE) clean -C $(PROTO_DIR)
	-rm -f $(TARGET) *.o cache-test proto/*.o
