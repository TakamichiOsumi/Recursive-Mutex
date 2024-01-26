# Recursive-Mutex

Learning reinvention of recursive mutex program that follows below rules:

1. If thread T1 has a lock on recursive mutex, any other thread T2 must get blocked if it tries to lock recursive mutex.

2. Other thread T2 must resume its execution only when T1, the thread holding the lock, releases all locks on recursive mutex.

3. Recursive mutex, like normal mutexes, must guarantee mutual exclusion.

4. Owning thread must invoke as many unlock calls as lock calls to release the recursive mutex.

5. Assert if any thread tries to unlock an unlocked recursive mutex.
