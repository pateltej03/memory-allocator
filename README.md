# Dynamic Memory Allocator – `mm.c`

### CMPSC 473 – Project 1

**Team Members:** Skyler Hawkins and Tej Patel

---

## 🔧 Project Overview

This project is a custom implementation of `malloc`, `free`, `realloc`, and a heap consistency checker (`mm_checkheap`) using an **explicit free list** allocator strategy. All code is contained within `mm.c`, and we’ve worked carefully to meet correctness, space utilization, and throughput targets per the assignment’s requirements.

---

## 🚀 Features and Design Highlights

### 🔹 Explicit Free List

-   Maintains doubly-linked list of free blocks.
-   Quick insertions/removals during allocation and coalescing.

### 🔹 16-byte Alignment

-   All blocks (allocated or free) are aligned to 16-byte boundaries as required.

### 🔹 Block Structure

-   **Allocated Block:** `[ allocHdr | payload ]`
-   **Free Block:** `[ header | payload | footer ]`
-   Headers/footers include size and allocation status (plus a previous allocation bit for coalescing).

### 🔹 Coalescing Support

-   **Immediate coalescing** with both next and previous blocks.
-   Supports all 4 cases: no merge, merge with next, prev, or both.
-   Footers help find previous blocks efficiently.

### 🔹 Splitting Support

-   Blocks are split during allocation or `realloc` if large enough to avoid internal fragmentation.
-   Minimum block size respected during splitting.

### 🔹 Footer Optimization

-   **Footers are only used in free blocks**, reducing overhead for allocated ones.

---

## ✅ Implemented Functions

| Function               | Description                                                      |
| ---------------------- | ---------------------------------------------------------------- |
| `mm_init()`            | Initializes the heap and explicit free list.                     |
| `malloc(size)`         | Finds a fitting free block or extends the heap using `mem_sbrk`. |
| `free(ptr)`            | Adds the block back to the free list and coalesces.              |
| `realloc(ptr, size)`   | Attempts in-place growth; otherwise allocates a new block.       |
| `calloc(nmemb, size)`  | Zero-initialized memory using `malloc` + `memset`.               |
| `mm_checkheap(lineno)` | Heap checker for correctness and debugging (explained below).    |

---

## 🧪 Heap Checker: `mm_checkheap`

Our checker validates:

-   Every block in the free list is marked as free.
-   No overlapping blocks.
-   All free blocks exist in the free list.
-   No uncoalesced adjacent free blocks.
-   All pointers are within heap bounds.
-   Total free block count matches heap traversal.

Debug output can be enabled with `#define DEBUG` at the top.

---

## ⚙️ Testing and Compilation

To build the allocator:

make

To test all traces:

make test

To test a specific trace:

./mdriver -f traces/<tracefile>.rep

To debug:

make debug
gdb ./mdriver

---

## 🧠 Let’s Connect!

**Tej Jaideep Patel**  
B.S. Computer Engineering  
📍 Penn State University  
✉️ tejpatelce@gmail.com  
📞 814-826-5544

---
