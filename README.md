# mongodb-cache

A cache implementation using MongoDB in C

## Dependency

- mongo-c-driver-1.4.2

## Install

```
> ./setup.sh
> make
```

## Run Example

```
> ./mcache
```

## Run Test

```
Usage: ./mcache-test [OPTIONS]
  -p            Amount of put operations per thread worker.
  -t            Number of threads.
  -s            Chunk size (byte).
  -h            Displays this help.
```

## Result

|        | Thread | Chunk size | # Put Operations | Execution Time |
|--------|--------|------------|------------------|--------------- |
| Test 1 |      1 |       1 MB |              500 |  15.951422 sec |

## Issue

- mongod crashes after around 800 put operations
- haven't implemented any chunk discard policy
