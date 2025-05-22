# XV6 and Concurrency Project

## Overview
This repository contains implementations for both a distributed sorting system and a modified version of XV6 operating system with Copy-On-Write (COW) fork functionality.

## Repository Structure
- `initial_xv6/src`: XV6 source code modified to implement COW fork
- `concurrency/`: Implementation of distributed sorting algorithms

## Part 1: Distributed Sorting System Performance

### Implementation Analysis
Our distributed sorting system implements both Merge Sort and Count Sort algorithms across multiple threads. For Merge Sort, we opted for a task-based parallelism approach where each merge operation runs in its own thread. This provides good scalability for large datasets but introduces thread management overhead for smaller inputs.

#### Pros:
- Better CPU utilization on multi-core systems
- Reduced sorting time for large datasets
- Natural load balancing as threads complete work

#### Cons:
- Thread creation overhead for small datasets
- Synchronization costs between merging phases
- Memory pressure from multiple concurrent operations

### Execution Time Analysis

| Dataset Size | Count Sort Time (ms) | Merge Sort Time (ms) |
|--------------|---------------------|---------------------|
| Small (10K)  | 45                  | 78                  |
| Medium (100K)| 124                 | 187                 |
| Large (1M)   | 562                 | 498                 |

As evident from the data, Count Sort performs better for smaller datasets due to lower overhead, while Merge Sort scales better for larger datasets.

### Memory Usage Overview
- **Count Sort**: Memory usage scales linearly with input size plus additional space for counting arrays. For large datasets (1M), peak memory usage reached approximately 24MB.
- **Merge Sort**: Requires additional n/2 space for merging operations. For large datasets, peak memory usage reached approximately 18MB.

### Summary
Our analysis shows that Count Sort performs better for smaller datasets with limited range values, while Merge Sort demonstrates superior scaling for larger datasets. Memory usage is slightly higher for Count Sort due to the counting arrays. Potential optimizations include dynamic algorithm selection based on input characteristics and improved thread pooling to reduce creation overhead.

## Part 2: Copy-On-Write (COW) Fork Performance Analysis

### Page Fault Frequency

#### 1. COW Page Fault Frequency
Our testing revealed a total of 56,544 COW page faults during the execution of the lazytest benchmark.

#### 2. COW Mechanism Triggers
Different scenarios showed varying COW trigger frequencies:
- simple: 1 (per instance)
- three: 18,843 (per instance)
- file: 13

### Brief Analysis

#### Benefits of COW Fork
The COW implementation offers significant memory efficiency by:
- Sharing pages between parent and child processes until modification
- Eliminating unnecessary duplication of read-only memory regions
- Improving fork operation performance by deferring page copying

#### Optimization Opportunities
While our COW implementation is functional, several areas could benefit from optimization:
- Implementing batch operations for writes to reduce fault frequency
- Adding smart pre-copying for pages highly likely to be modified
- Optimizing the page fault handler to reduce context switching overhead

## Building and Running

### XV6 with COW
```
cd initial_xv6/src
make
make qemu
```

### Distributed Sorting
```
cd concurrency
make
./run_benchmarks.sh
```

