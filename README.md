# COSC 519 — Labs & Project (C on Ubuntu)

This repository contains multiple COSC 519 labs and assignments organized into separate folders.

## Folder Structure
- `Lab1_process/` — fork() / process basics
- `Lab2Thread/` — pthread basics + thread exercises
- `Lab3_synchronization/` — producer/consumer synchronization (semaphores + shared memory)
- `Assignment1_cpu_scheduling/` — CPU scheduling simulator (FCFS / RR / Priority)
- `Project_semaphore_monitor/` — Producer/Consumer project (semaphore + monitor versions)

> **Note:** Exact filenames may differ on your machine—replace the example `.c` filenames below with your actual filenames.

---

## Requirements
- Ubuntu (or WSL)
- `gcc`
- POSIX threads (`-pthread`)
- (Sometimes) realtime library (`-lrt`)

Install build tools if needed:
```bash
sudo apt update
sudo apt install build-essential
```
## Lab 1 — Process (fork)

This lab demonstrates fork() and parent/child behavior. 
Build & Run
```bash
cd Lab1_process
gcc -Wall -Wextra -O2 process_lab.c -o process_lab
./process_lab
```

## Lab 2 — Threads (pthreads)

Uses pthread_create() and pthread_join() for thread creation and joining. 
Build & Run
```bash
cd Lab2Thread
gcc -Wall -Wextra -O2 lab2.c -o lab2 -pthread
./lab2
```

## Lab 3 — Synchronization (Producer/Consumer)

This lab contains producer/consumer synchronization using semaphores.

- A) Thread-based bounded buffer (semaphores) Uses sem_wait, sem_post, and a circular buffer pattern. 
raLab3
Build & Run
```bash
cd Lab3_synchronization
gcc -Wall -Wextra -O2 sync_threads.c -o sync_threads -pthread
./sync_threads
```
- B) Process-based version (named semaphores + shared memory)
Uses sem_open() plus System V shared memory (shmget, shmat). 
Build
```bash
cd Lab3_synchronization
gcc -Wall -Wextra -O2 producer.c  -o producer  -pthread
gcc -Wall -Wextra -O2 processor.c -o processor -pthread
gcc -Wall -Wextra -O2 consumer.c  -o consumer  -pthread
```
- Run (3 terminals)
Terminal 1:
```bash
./producer
```
- Terminal 2
```bash
./processor
```
- Terminal 3:
```bash
./consumer
```

## Assignment 1 — CPU Scheduling (FCFS / RR / Priority)

The program builds as assin1_3 and supports both file mode and random mode. 
Build
```bash
cd Assignment1_cpu_scheduling
gcc -o assin1_3 assin1_3.c
```
Run (file mode)
```bash
./assin1_3 fcfs file ex5_19.txt
./assin1_3 rr   file ex5_19.txt 1
./assin1_3 pri  file ex5_19.txt
```
Run (random mode)
```bash
./assin1_3 fcfs random 6
./assin1_3 rr   random 6 2
```

## Project — Producer/Consumer (Semaphore + Monitor)
- Semaphore version

Open the code, optionally change the log filename, compile, and run: 
```bash
cd Project_semaphore_monitor

# (optional) edit
vi semaphore.c

# compile
gcc -Wall -Wextra -O2 semaphore.c -o semaphore -pthread -lm

# if you get clock_gettime link error:
gcc -Wall -Wextra -O2 pc_semaphore.c -o semaphore -pthread -lm -lrt

# run
./semaphore
```
When running, it will prompt you for buffer size, number of producers, and number of consumers.
- Monitor version

```bash
gcc -Wall -Wextra -O2 monitor.c -o monitor -pthread -lm
./monitor

```

