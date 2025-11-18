# Systems Programming & Distributed Computing Labs

> **Linux System, Modern C++, Multi-threading, and Distributed Computing**

이 저장소는 로우 레벨 메모리 관리부터 멀티스레딩, 분산 병렬 처리에 이르는 시스템 프로그래밍의 핵심 주제들을 C++로 구현한 프로젝트 모음입니다. STL 컨테이너를 직접 구현하며 내부 동작 원리를 파악하고, `pthread`/`std::thread`와 `MPI`를 활용하여 성능을 최적화하는 데 중점을 두었습니다.

## Tech Stack & Environment
* **Language:** C++11/14 (Modern C++), C
* **Core Libraries:** STL, POSIX Threads (pthread), OpenMPI
* **Tools:** GCC/G++, Make, Valgrind (Memory Leak Check), GDB
* **OS:** Linux (Ubuntu 24.04 LTS via WSL2)

## Projects Overview

| Project | Topic | Key Technical Accomplishments |
| :--- | :--- | :--- |
| [**HW1: Custom Array**](./HW1) | **Memory Management** | • `malloc`/`free`와 Placement New를 이용한 Raw Memory 관리<br>• Deep Copy 및 Exception Safety를 고려한 컨테이너 설계 |
| [**HW2: Template Vector**](./HW2) | **Generic Programming** | • C++ Template을 활용한 Type-Agnostic 컨테이너 구현<br>• `std::vector`의 Capacity/Size 관리 정책 및 재할당 최적화 |
| [**HW3: Parallel Sort**](./HW3) | **Multi-threading** | • `std::thread`를 활용한 Merge Sort 병렬화 (Linear Speedup 달성)<br>• 임시 메모리 재사용(Pre-allocation)을 통한 오버헤드 최소화 |
| [**HW4: Distributed FFT**](./HW4) | **MPI & Algorithms** | • **MPI**를 활용한 분산 메모리 기반 2D FFT 구현 ($O(N \log N)$)<br>• Non-blocking 통신(`Irecv`)을 이용한 Latency Hiding 및 동기화 |

## How to Build & Run

각 프로젝트 폴더(`HW1` ~ `HW4`) 내에는 독립적인 `Makefile`이 포함되어 있습니다.

```bash
# Clone the repository
git clone [https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git](https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git)

# Example: Running HW3 (Parallel Sort)
cd HW3
make
./thread 4  # Run with 4 threads
```

## 🧪 Verification Strategy

모든 프로젝트는 엄격한 검증 과정을 거쳤습니다.
1.  **Correctness:** 제공된 Test Case 및 Reference Output과의 일치 여부 확인.
2.  **Performance:** 스레드/프로세스 수 증가에 따른 실행 시간 단축(Speedup) 측정.
3.  **Memory Safety:** `Valgrind`를 이용한 Memory Leak 및 Invalid Access 전수 검사 완료.

