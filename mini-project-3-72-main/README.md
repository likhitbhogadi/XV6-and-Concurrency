[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/k72s6R8f)
# LAZY™ Corp
OSN Monsoon 2024 mini project 3

## Some pointers
- main xv6 source code is present inside `initial_xv6/src` directory. This is where you will be making all the additions/modifications necessary for the xv6 part of the Mini Project. 
- work inside the `concurrency/` directory for the concurrency part of the Mini Project.

- You are free to delete these instructions and add your report before submitting. 


# REPORT

## Part 1: Distributed Sorting System Performance

### Implementation Analysis:
        Mention why you chose a certain approach to distribute tasks across the systems and the pros/cons of said approach. For example, for distributed Merge Sort, if you’ve chosen to create a thread per merge operation, you have to explain why and the pros/cons of this approach. (kindly note that this is not the only way to go about this!)
### Execution Time Analysis:
        Measure the execution time for both Distributed Count Sort and Distributed Merge Sort with a few different file counts (small, medium, and large).
        The boss wants to know how well each sorting method scales, so log the times for just a handful of different data sizes.
### Memory Usage Overview:
        Provide a brief assessment of the memory usage for each algorithm when sorting both small and large datasets.
        This can be a short summary rather than an in-depth breakdown.
### Graphs:
        Include a simple line graph showing execution time across different file counts for each sorting algorithm.
        Use a bar chart to give a quick comparison of memory usage for Distributed Count Sort and Distributed Merge Sort.
### Summary:
        Write a brief summary of the findings, focusing on how each sorting method handles different file counts.
        Mention any potential optimizations that could improve performance for larger datasets.

## Part 2: Copy-On-Write (COW) Fork Performance Analysis

### Page Fault Frequency:

#### 1. frequency of page faults during the operation of the COW fork (Testing it with processes that read only, as well as those that modify memory.)

    for lazytest, in total, no. of cow page faults is 56,544

#### 2. how many times the COW mechanism is triggered under different scenarios.

    simple  1
    simple  1
    three   18843
    three   18843
    three   18843
    file    13

    the fuctions tested in lazytest

### Brief Analysis:

#### the benefits of COW fork in terms of efficiency and memory conservation 

    COW fork offers substantial improvements in memory efficiency by delaying and often preventing memory duplication. By sharing pages until a write occurs, COW reduces the need for immediate copying, cutting down on memory usage and making fork operations significantly faster for processes that are mostly reading memory.

#### areas where COW could be further optimized.
    While COW is efficient, there may be room to further reduce the frequency of page faults, especially in scenarios where processes only slightly modify shared pages. Potential optimizations could include batching writes to reduce COW trigger frequency, or implementing finer-grained COW for sub-page regions if possible.
