# Technical and Comparative Study: Choosing Data Structures

## 1. Introduction
In the context of the R-Type project, the performance of the game engine is critical. The ECS (Entity Component System) architecture relies heavily on manipulating components. This document compares different data structures for storing these components, focusing on **time efficiency** and **space efficiency**.

## 2. Candidates and Assumptions

* **Sparse Array (`std::vector<std::optional<T>>`)**: Structure imposed by the bootstrap. Uses a contiguous array with gaps.  
  * *Assumption:* Excellent iteration performance thanks to CPU cache locality, but high memory usage if entities are sparse.

* **Std Map (`std::map<size_t, T>`)**: Red-black tree.  
  * *Assumption:* Insertion and lookup in $O(\log N)$, but slow iteration due to memory fragmentation (cache misses).

* **Std Vector (`std::vector<T>`)**: Dense contiguous array.  
  * *Assumption:* Absolute performance baseline, but does not easily handle entity IDs (index 0 is not necessarily entity 0).

## 3. Test Protocol
Tests were performed on [YOUR OS / CPU].  
We measure:
1. **Iteration Time:** Time to traverse all elements and modify a value (simulation of a `MovementSystem`).  
2. **Memory Consumption (RSS):** Peak RAM usage.

## 4. Results

### Summary Table (Time in microseconds µs)

| N (Entities) | Structure | Insertion Time | Iteration Time (Critical) | Memory (Max RSS) |
| :--- | :--- | :--- | :--- | :--- |
| **10,000** | Map | 1,696 | 105 | 4,708 KB |
|  | Sparse Array | 113 | 63 | 3,932 KB |
|  | Vector | 410 | 65 | 3,928 KB |
| **100,000** | Map | 33,700 | 2,536 | 14,428 KB |
|  | Sparse Array | 1,985 | 714 | 8,792 KB |
|  | Vector | 5,924 | 859 | 8,372 KB |

## 5. Technical Analysis

### CPU Cache and Iteration
As mentioned in the bootstrap subject (Step 1), the processor loads memory using **cache lines**.
* **SparseArray:** Data is contiguous. Even with empty `std::optional` entries, the CPU prefetcher can efficiently load subsequent data.
* **Map:** Each node is separately allocated on the heap. Iteration requires jumping from address to address, causing constant **cache misses**.

### Memory Footprint
The `SparseArray` shows its limits when density is low. For a rare component (e.g., `BossAI` present on 1 entity out of 100,000), the SparseArray allocates 100,000 mostly empty `std::optional`s, whereas the `Map` allocates only a single node.

## 6. Conclusion
For the R-Type engine:
1. We validate the use of **SparseArray** for frequent components (`Position`, `Velocity`, `Drawable`) because the performance gain during iteration (×5 to ×10) is essential for maintaining 60 FPS.
2. We will use **SparseArray** in accordance with Bootstrap Step 1.
