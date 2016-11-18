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
  -p            Amount of put operations in total.
  -t            Number of threads.
  -s            Chunk size (byte).
  -h            Displays this help.
```

## Result

|        | Thread | Chunk size | # Put Operations | Execution Time |
|--------|--------|------------|------------------|--------------- |
| Test 1 |      1 |       1 MB |              500 |  15.951422 sec |
| Test 2 |      1 |       1 MB |              200 |  11.564633 sec |
| Test 3 |      2 |       1 MB |              200 |  10.297076 sec |

## Issue

- mongod crashes after around 800 put operations
- haven't implemented any chunk discard policy

## Todo

- create operation object (action, meta)
- pass operations via socket, handled by cached worker
- add created time / accessed time
- implement garbage collector thread
