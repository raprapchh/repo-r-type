# Technical and Comparative Study: Choice of Data Structures

## 1. Introduction
As part of the R-Type project, the performance of the game engine is critical. The ECS (Entity Component System) architecture relies on intensive manipulation of components. This document compares different data structures for storing these components, focusing on time efficiency and space efficiency.

## 2. Candidates and Hypotheses

* **Sparse Array (`std::vector<std::optional<T>>`)**: Structure imposed by the bootstrap. Uses a contiguous array with holes.
    * *Hypothesis*: Excellent iteration performance thanks to CPU cache, but high memory consumption if entities are sparse.
* **Std Map (`std::map<size_t, T>`)**: Red-black binary tree.
    * *Hypothesis*: $O(\log N)$ insertion and search, but slow iteration due to memory fragmentation (cache miss).
* **Std Vector (`std::vector<T>`)**: Dense contiguous array.
    * *Hypothesis*: Absolute performance reference, but does not easily handle entity IDs (index 0 is not necessarily entity 0).

## 3. Test Protocol
Tests were performed on [YOUR OS / CPU].
We measure:
1.  **Iteration time**: Time to traverse all elements and modify a value (Simulation of a `MovementSystem`).
2.  **Memory consumption (RSS)**: Peak RAM usage.

## 4. Results

### Summary table (Time in microseconds µs)

| N (Entities) | Structure | Insertion Time | Iteration Time (Critical) | Memory (Max RSS) |
| :--- | :--- | :--- | :--- | :--- |
| **10 000** | Map | 1 696 | 105 | 4 708 KB |
| | Sparse Array | 113 | 63 | 3 932 KB |
| | Vector | 410 | 65 | 3 928 KB |
| **100 000** | Map | 33 700 | 2 536 | 14 428 KB |
| | Sparse Array | 1 985 | 714 | 8 792 KB |
| | Vector | 5 924 | 859 | 8 372 KB |
## 5. Technical Analysis

### CPU Cache and Iteration
As mentioned in the bootstrap subject [Step 1], the processor loads memory by **Cache Lines**.
* **SparseArray**: Data is contiguous. Even with empty `std::optional` values, the CPU's "Prefetcher" can efficiently load the following data.
* **Map**: Each node is allocated separately in the heap. Iterating requires jumping from address to address, causing constant **Cache Misses**.

### Memory Footprint
The `SparseArray` shows its limitations when density is low. For a rare component (e.g., `BossAI` present on 1 entity out of 100,000), the SparseArray allocates 100,000 `std::optional` (mostly empty), while the `Map` allocates only a single node.

## 6. Conclusion
For the R-Type engine:
1.  We validate the use of **SparseArray** for frequent components (`Position`, `Velocity`, `Drawable`) because the iteration performance gain (x5 to x10) is vital to maintain 60 FPS.
2.  [cite_start]We will use **SparseArray** in accordance with Bootstrap Step 1.
