# Technical Study: Data Security and Integrity

## 1. Identification of Vulnerabilities (Chosen Technology)

The choice of **C++** and the **UDP** protocol for the R-Type project exposes the engine to specific risks that must be properly handled.

### A. Vulnerabilities Related to C++
* **Buffer Overflow:** A critical risk when receiving raw network packets (`char* buffer`). If a malformed packet declares a size larger than the allocated buffer, the server may crash or execute arbitrary code.
* **Memory Leaks:** The server must run continuously. A leak of even a few bytes per frame (e.g., a missile destroyed but not deallocated) will saturate RAM within hours, causing a denial of service (DoS).
* **Dangling Pointers:** Accessing an entity (e.g., a player) that has been destroyed (`delete`) but whose pointer still exists inside a System.

### B. Vulnerabilities Related to the UDP Protocol
The project requires the use of UDP.
* **Lack of Authentication (IP Spoofing):** UDP is stateless. An attacker can easily spoof a client IP to send “Fire” or “Disconnect” commands on their behalf.
* **Replay Attacks:** An attacker can capture a valid packet (“Shoot”) and resend it 1000 times to the server.
* **Malformed Packets (Fuzzing):** Sending random data to attempt to crash the server’s parser.

## 2. Protection Measures and Tools (Qualitative Benchmark)

To mitigate these risks, we integrated the following tools and practices into our development pipeline (CI/CD).

### A. Static Analysis (Before Compilation)
We use **Clang-Tidy** to enforce coding standards and detect dangerous constructs before runtime.

| Tool | Role | Configuration |
| :--- | :--- | :--- |
| **Clang-Tidy** | Detection of potential bugs, coding style | `.clang-tidy` (Checks: `cppcoreguidelines-*`, `bugprone-*`) |
| **Compilation** | Strict warnings | `-Wall -Wextra -Werror` |

### B. Dynamic Analysis (During Execution)
This is where our security “Benchmark” takes place. We compile the project in “Sanitizer” mode for testing.

1. **AddressSanitizer (ASan):**
   * *Purpose:* Detect buffer overflows and invalid memory accesses in real time.
   * *Implementation:* Compilation flag `-fsanitize=address`.
   * *Result:* If the server reads outside an array (e.g., during network parsing), it stops immediately with a detailed error report.

2. **Valgrind (Memcheck):**
   * *Purpose:* Ensure the complete absence of memory leaks.
   * *Test Protocol:* Launch the server, connect 4 clients, play for 5 minutes, disconnect everyone, stop the server.
   * *Validation Criterion:* `definitely lost: 0 bytes`.

### C. Secure Software Architecture (RAII)
To avoid manual errors (`new`/`delete`), we strictly apply the RAII idiom:
* Exclusive use of **Smart Pointers** (`std::unique_ptr`, `std::shared_ptr`) for components and entities.
* Use of `std::vector` and standard containers (instead of C arrays `[]`) to guarantee bounds checking via the `.at()` method.

## 3. Secure Network Protocol
To counter UDP vulnerabilities:
* **Magic Number:** Each binary packet begins with a unique identifier (e.g., `0xCAFEBABE`). Any packet that does not start with it is rejected (protection against network noise).
* **Size Validation:** The server checks that `bytes_received >= sizeof(Header) + expected_payload_size` before reading any data.
