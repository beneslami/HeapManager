# HeapManager

This project is about managing Heap memory. it consists of 10 phases. Each phase discribe some concepts.

##phase 1
Size of Virtual Memory (VM) page is constant (8192B on most systems). We usually use malloc/calloc to allocate 
dynamic memory in our program. In fact, malloc/free are linux library functions that uses sbrk/brk system calls
to dynamically manage memory. Similarly, another pair of system call called mmap/munmap which are a wrapper for
a system call called mmap_pgoff()

![diagram](phase%201/mmap.png)

based on the above picture:
1. Once linux memory manager is assigned the VM complete page from the kernel, LMM will further split VM page to meet 
the application hunger for memory.
2. VM page allocated are need not be contiguous in Heap memory segment of the process as opposed to sbrk behavior.
3. Heap memory segment is just a data structure maintained by the kernel for every process, which keeps track of MV pages
being used by the LMM.
4. Our LMM always request and release memory from the kernel in virtual memory PAGE SIZE granularity.

It is important to note that memory allocation/deallocation between user space process and glibC can happen of any size.
but page allocation and deallocation between glibC and kernel memory manager (MMU) happens only in units of page sizes. it is
because sbrk/mmap system calls are expensive, so that glibC **caches** the VM page allocated by the kernel MMU and allocates 
small chunks from it to the process.

![diagram](phase%201/page caching.png)

##phase 2
**Page Family Registration** means the user space application relying on Linux Memory Manager (LMM) during initialization,
tell the LMM the details of the structures application is using.
The user space application should inform the LMM the name and the amounts of bytes for each and every data structures using an API.
page families form a linked list.
![diagram](phase%202/pageFamily.png)

