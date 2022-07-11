# Implementing a Garbage Collector in C 
Garbage Collectors (GC) serve as an automatic memory manager that allocates and releases memory in a program. This project is a simple implementation of a memory garbage collector written in C that allocates memory for letters in the English alphabet and frees those chunks of memory. 

<center>
    <img src = "public/assets/demo.jpg"/>
</center> 

## Inspiration 
I wanted to try and build my own simple garbage collector to learn about memory management in C. 

## Implementation 
### Malloc() and Free()
The first step to implement a garbage collector is to write a malloc function to allocate memory into the heap. When a user requests for memory (maybe from creating new objects of a fixed size), memory is allocated as fixed chunks onto a heap. Now, consider that the user might want to free a specific chunk that is neither the first or the last chunk. Then maybe later the user wants to allocate the chunk again. However, this would require us to find the location of the missing chunk, taking O(N) time. 

<center>
    <img src = "public/assets/freechunk.jpg"/>
</center> 

Ideally, we would want a faster method. We can try to design a more efficient implementation that uses two pointer on an array in which the second pointer is used to track the latest free chunk. 

### Scanning and Bytes
Say we have implement the function to allocate and free memory from the heap. The next step would be to implement a function to scan through the heap in 8 bytes (since that is the size of a pointer) to find allocated chunks of memory. For the allocated chunks we find, we want to find a pointer and see if that pointer points something within the heap since if it points to one of the allocated chunks we can label that pointed chunk as reachable. If it turns out that the pointed chunk is unreachable, we can deallocate that chunk. With that, we have the general idea of how to implement a garbage collector. 

## Installation
1) git clone https://github.com/alnmathw/garbage-collector-in-c.git
2) make
3) cd src
4) ./heap

## Limitation 
1) The pointers in the heap can only be located in the heap and the stack.


