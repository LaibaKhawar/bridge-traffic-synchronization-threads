# Bridge Traffic Synchronization with POSIX Threads

A C++ pthreads simulation of a narrow bridge where each vehicle is a thread: at most two cars may be on the bridge together, and a bus must cross alone. Access is controlled with one mutex and two condition variables.

**Interactive demo:** [Run it in your browser](https://laiba-khawar-portfolio.vercel.app/work/bridge-sync#demo)

The demo reproduces the stall described under Known limitations and shows a fixed wake-up policy side by side.

## Rules enforced

- Up to **2 cars** on the bridge at once.
- A **bus** crosses **alone**: no cars and no other bus on the bridge.
- A car may not enter while a bus is on the bridge.

## How it works (`Q2.cpp`)

Shared state (protected by `bridgeMutex`): `carsOnBridge` and `busesOnBridge`, declared as function-local `static` counters.

Condition variables:

- `canCrossBridge`: cars wait here.
- `canCrossBridgeForBus`: buses wait here.

Each vehicle thread:

1. Locks the mutex. A bus first waits while another bus is on the bridge.
2. Waits while its rule is violated: a car while `carsOnBridge >= 2 || busesOnBridge > 0`; a bus while `busesOnBridge > 0 || carsOnBridge > 0`.
3. Increments its counter and broadcasts on its own condition variable, unlocks, prints that it is crossing, and sleeps 1 second.
4. Re-locks and decrements its counter. On exit a **bus calls `pthread_cond_signal` on the bus condition variable only**, and a **car calls `pthread_cond_broadcast` on the car condition variable only**.

`main` asks for the maximum number of cars, the maximum number of buses and a direction (0 or 1). It then creates `maxCars + maxBuses` threads, choosing car or bus at random (`rand() % 2`, seeded with the time) until both quotas are filled, with a 1 second pause between threads. It sleeps 60 seconds and then joins all threads.

## Repository contents

- `Q2.cpp`: the full program.

## Building and running

Requires a C++11 compiler with pthreads (Linux or macOS).

```bash
git clone https://github.com/LaibaKhawar/bridge-traffic-synchronization-threads.git
cd bridge-traffic-synchronization-threads
g++ -std=c++11 -pthread Q2.cpp -o bridge
./bridge
```

The code uses brace initialisation (`new Vehicle{direction, randomType}`), so it needs C++11 or later. Recent GCC defaults to a newer standard and builds with plain `g++ -pthread Q2.cpp -o bridge`; Apple clang 16 rejected that command until `-std=c++11` was added.

Example input: 3 cars, 2 buses, direction 0. Each vehicle prints a "waiting" line whenever it blocks and a "crossing" line when it enters.

## Known limitations

- **Lost wake-up for buses.** When a car leaves, it broadcasts only to `canCrossBridge`. A bus that is waiting on `canCrossBridgeForBus` because cars were on the bridge is never woken, even after the bridge empties, unless another bus happens to signal later. With the right arrival order the bus thread waits forever and `main` blocks in `pthread_join`.
- **Same problem in the other direction.** When a bus leaves, it signals only `canCrossBridgeForBus`, so cars that queued behind the bus are not woken by its exit.
- **Direction is not used.** The direction read from stdin is stored in each `Vehicle` and printed, but no rule depends on it, and every vehicle gets the same value.
- The 60 second `sleep` in `main` before joining is unnecessary; `pthread_join` alone would wait for the threads.
- `main` uses a variable-length array (`pthread_t threads[numThreads]`), a GCC/Clang extension rather than standard C++.

## Author

[Laiba Khawar](https://github.com/LaibaKhawar) · [LinkedIn](https://www.linkedin.com/in/laiba-k-00b2b1249/)
