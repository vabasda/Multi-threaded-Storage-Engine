# LSM-Tree Storage Engine Multi-Threading (Kiwi Bench)

An implementation of multi-threaded operations on top of the **Kiwi** Log-Structured Merge-tree (LSM-tree) key-value storage engine. This project introduces thread-safety and concurrency mechanisms using the Linux **Pthreads** library, optimizing concurrent data access while safely handling background compaction threads.

This repository contains the work developed for **Lab 1** of the **MYY601 Operating Systems** course (Spring 2024) at the Department of Computer Science and Engineering, University of Ioannina.

## 👥 Authors
* **Evangelos Basdavanos** 
* **Konstantinos Kapsimalis** 
* **Romanos Kotsis** 

---

## 🚀 Features & Architecture

The storage engine architecture centers around an LSM-tree leveraging an in-memory **Memtable** (implemented via a Skip List) and disk-bound **Sorted String Tables (SST)**. 

We developed two main multi-threading paradigms to replace the initial unsafe single-threaded approach:

### 1. Simple Approach (Mutual Exclusion)
* Employs `pthread_mutex_t` to restrict entry into the critical zones of `db_add` and `db_get`.
* Ensures that only one operations thread utilizes the core engine at any single moment.
* Utilizes a non-blocking `pthread_mutex_trylock` loop to check the status of the background compaction/merge process (`cv_lock`), mitigating race conditions with the background thread without causing application deadlocks.

### 2. Advanced Approach (Readers-Writers Concurrency)
* Implemented using POSIX semaphores (`<semaphore.h>`) to decouple readers (`get`) and writers (`add`).
* Restricts updates (`add`) to behave strictly in mutual exclusion, while allowing multiple concurrent lookup threads (`get`) to execute safely.
* Safeguards `sst_get` and `memtable_add` inside critical segments to avoid state pollution during active structural updates.

### 3. Concurrency Benchmarking Extension (`readwrite`)
* Augmented the original driver code to natively support arbitrary operation mixes and configurable scaling.
* Introduced a shared multi-argument thread tracking context (`struct data`) passed safely across POSIX thread boundaries.

---

## 🛠️ Compilation & Usage

### Prerequisites
Tested inside a Debian 12 environment using `gcc` and `make`.

### Building the Project
To compile the storage engine library along with the modified benchmarking suite, navigate to the root directory and run:
```bash
make all
```

### Running the Benchmarks
The binary `kiwi-bench` resides within the `bench/` directory.

#### Basic Writes/Reads:
Execute synchronous isolated updates or lookups specifying the raw number of elements $x$:
```bash
# Sequential/Random Writes
./kiwi-bench write 100000

# Sequential/Random Reads
./kiwi-bench read 100000
```

#### Mixed ReadWrite Concurrent Execution:
Test the engine under parallel multi-threaded load utilizing the newly implemented syntax:
```bash
./kiwi-bench readwrite <total_ops> <write_percentage> <read_percentage> <read_threads>
```
* **Example (1 thread read-write mix):** 
  ```bash
  ./kiwi-bench readwrite 400000 50 50 1
```
* **Example (Advanced mode - 5 parallel read threads):**
  ```bash
  ./kiwi-bench readwrite 200000 30 70 5
```

---

## 📊 Experimental Highlights

Comprehensive operational benchmarks evaluated scaling capabilities across key workloads ($x \in [100k, 200k, 300k, 1M, 2M]$ items).

Key takeaways include:
* **Background Merges Cost Overhead:** Writing a vast quantity of elements ($1,000,000+$) shows a sharper growth in runtime costs compared to readers due to structural overheads incurred by disk **SST compaction/merge operations**.
* **Readers-Writers Scalability:** Deploying the advanced approach across intensive read distributions (e.g., 30%-70% Write-Read ratio) demonstrates notable performance gains when switching from 3 up to 5 concurrent reader threads for elevated data scales.

---

## 📂 Referenced Materials
For a deeper dive into the exact performance figures, code structure, and theoretical background, please consult the following documentation included in this repository:
* **`MYY601-L1-2024-GR.pdf`**: The official lab assignment guidelines outlining the LSM architecture and implementation specifications.
* **`Report.pdf`**: Our final comprehensive report containing the detailed line-by-line code documentation, full hardware testing specifications, and performance graphs.
