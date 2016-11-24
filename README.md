# mongodb-cache

A cache implementation using MongoDB in C

## Dependency

- mongo-c-driver-1.4.2

## Install

```
> ./setup.sh
> make
```

## Run Cache Daemon

```
> ./cached
```

## Run Example

```
> ./mcache
```

## Run Test - Cache Daemon API

```
Usage: ./mcache-test [OPTIONS]
  -p            Amount of put operations in total.
  -t            Number of threads.
  -s            Chunk size (byte).
  -h            Displays this help.
```

## Run Test - Cache API

```
> make cache-test
> ./cache-test
```

## Result

## Todo

- implement garbage collector thread
