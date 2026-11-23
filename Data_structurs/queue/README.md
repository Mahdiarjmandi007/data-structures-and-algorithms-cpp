# Queue (Circular Array Implementation) in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

## Overview

A **queue** is a linear data structure that follows **FIFO** (First In, First Out) order: the element that has been in the queue the longest is removed first. This implementation uses a **circular array** of 100 integers. Two indices, `front` and `rear`, track the two ends, and a separate `count` makes it possible to distinguish a full queue from an empty one even after the indices wrap around.

The program is an interactive console menu. It displays the queue after every enqueue and dequeue, which makes the circular behavior easy to observe.

## Files

| File | Purpose |
| --- | --- |
| [`queue.cpp`](queue.cpp) | Complete implementation: circular queue, operations, and menu |

## Build and Run

```bash
g++ -std=c++17 queue.cpp -o queue
./queue
```

On Windows:

```powershell
g++ -std=c++17 queue.cpp -o queue.exe
.\queue.exe
```

> The program calls `system("cls")` directly, which works on Windows only. The `CLEAR` macro at the top of the file is defined but not used by the menu.

## How to Use

```text
Menu
1.enqueue
2.dequeue
3.peek
4.exit
Enter The choice :
```

| Choice | Action | What it asks / prints |
| --- | --- | --- |
| `1` | Enqueue | Asks for an integer, inserts it at the rear, then prints the queue |
| `2` | Dequeue | Removes the front element, then prints the queue |
| `3` | Peek | Prints the front value without removing it |
| `4` | Exit | Ends the program |

After enqueue/dequeue/peek, the program waits for one character before clearing the screen.

### Example session

Starting empty, enqueue `10`, `20`, and `30`:

```text
Queue: 10
Queue: 10 20
Queue: 10 20 30
```

Then dequeue twice:

```text
Queue: 20 30
Queue: 30
```

`peek()` prints `Front element: 30` and leaves the queue unchanged.

## How It Works

### Data representation

```cpp
const int MAX = 100;

int queue[MAX];
int front = -1;
int rear = -1;
int count = 0;
```

- `front` is the index of the first (oldest) element.
- `rear` is the index of the last (newest) element.
- `count` is the number of stored elements.
- Both indices start at `-1` because the queue is empty.

### Why a circular array?

In a naive array queue, removing from the front would require shifting every remaining element left, which costs **O(n)**. A circular array avoids that: `front` simply moves forward, and when it reaches the end of the array it wraps to index `0` using modulo arithmetic:

```cpp
front = (front + 1) % MAX;
rear  = (rear + 1) % MAX;
```

A sequence of enqueues and dequeues can therefore occupy the end of the array and then continue at the beginning:

```text
MAX = 8, count = 4, front = 6, rear = 1

index:   0    1    2    3    4    5    6    7
        [40] [50] [  ] [  ] [  ] [  ] [10] [20]
          ^                                  ^
        rear                               front

logical order: 10, 20, 40, 50  (6 -> 7 -> 0 -> 1)
```

### Helper functions

| Function | Behavior |
| --- | --- |
| `length()` | Returns `count` |
| `isempty()` | Returns `count == 0` |
| `is_full()` | Returns `count == MAX` |

### `enqueue(int value)`

1. If the queue is full, prints `"Queue is full"` and returns `false`.
2. If the queue is empty, sets `front = 0`.
3. Advances `rear = (rear + 1) % MAX`.
4. Stores `value` at `queue[rear]`.
5. Increments `count` and returns `true`.

Time: **O(1)**.

### `dequeue()`

1. If the queue is empty, prints `"Queue is empty"` and returns `false`.
2. If `front == rear`, the queue contains exactly one element: reset `front` and `rear` to `-1`.
3. Otherwise advance `front = (front + 1) % MAX`.
4. Decrement `count` and return `true`.

The removed value is not returned. Time: **O(1)**.

The `front == rear` special case is necessary because after removing the last element, both indices should return to the empty state. The `count` variable would also make this detectable, but resetting both indices keeps the invariants simple.

### `peek()`

If the queue is not empty, prints `queue[front]`. Time: **O(1)**.

### `display()`

If the queue is empty, prints `"Queue is empty"`. Otherwise it starts at `front` and follows the circular order until it reaches `rear`:

```cpp
int i = front;
while (true) {
    cout << queue[i] << " ";
    if (i == rear) break;
    i = (i + 1) % MAX;
}
```

Time: **O(n)**, where `n` is the number of elements.

## Complexity Summary

| Operation | Time | Space |
| --- | --- | --- |
| `enqueue` | O(1) | O(1) |
| `dequeue` | O(1) | O(1) |
| `peek` | O(1) | O(1) |
| `display` | O(n) | O(1) |
| Entire queue | — | O(MAX) = O(100) |

## Notes and Limitations

- **Fixed capacity:** the queue holds at most 100 integers; there is no resizing.
- **Global state:** the array and indices are global variables.
- **Dequeue does not return the removed value.** The menu only reports success/failure.
- **`count` is essential:** without it, `front == rear` would be ambiguous (empty or full). This implementation uses both `count` and the `-1` reset convention.
- **Windows-only clearing:** `system("cls")` is used directly even though a cross-platform `CLEAR` macro is defined at the top.
- **No dynamic allocation:** the array lives for the whole program.
- **Invalid input:** the menu handles out-of-range choices, but non-numeric input can leave `cin` in a failed state.
- **The display is not a true queue operation:** it is a diagnostic helper that walks the logical order.

## What This Teaches

- The FIFO discipline and how it differs from LIFO (stack).
- How modulo arithmetic turns a linear array into a circular buffer.
- Why a separate `count` (or an empty slot) is needed to distinguish full from empty.
- Why circular queues avoid the O(n) shifting cost of a naive array queue.
- How to iterate over a wrapped range correctly.
