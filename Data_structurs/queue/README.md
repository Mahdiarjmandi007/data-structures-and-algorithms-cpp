# Queue (Circular Array Implementation) in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

## Overview

A **queue** is a linear data structure that follows **FIFO** (First In, First Out) order: the element that has been in the queue the longest is removed first. This implementation uses a **circular array** of 100 integers. Two indices, `front` and `rear`, track the two ends, and a separate `count` makes it possible to distinguish a full queue from an empty one even after the indices wrap around.

The implementation has two front ends: the original console menu in `queue.cpp`, and a **raylib GUI** in `gui.cpp` that uses a copy of the same queue operations from `queue.h`. The GUI adds a logical queue view, a 100-slot circular-buffer map, enqueue/dequeue animations, and a clean control panel without changing the original console program.

## Two Versions

This folder contains two completely separate front ends:

- **CLI version (`queue.cpp`)** — the original console program, written without AI. It is the reference implementation; use it to study how the circular queue is implemented in code (`front`, `rear`, `count`, modulo arithmetic, full/empty checks, menu loop).
- **UI version (`gui.cpp` + `queue.h`)** — a separate raylib program, generated entirely with AI as a visual copy of the CLI operations. It is the best place to see **what the queue does**: FIFO order, front/rear movement, occupied slots, wrap-around, full/empty states, and animations.

The two versions are independent: separate source files, separate executables (`queue.exe` vs `gui.exe`), and no runtime dependency. The CLI version is unchanged.

## Files

| File | Purpose |
| --- | --- |
| [`queue.cpp`](queue.cpp) | Original console program: circular queue, operations, and menu (unchanged) |
| [`queue.h`](queue.h) | Header copy of the existing queue globals and functions, used by the GUI |
| [`gui.cpp`](gui.cpp) | raylib visualizer: logical queue, circular array map, input, and animations |
| [`build_gui.bat`](build_gui.bat) | Windows/MinGW build script for `gui.exe` |

## Build and Run

### Console version

```bash
g++ -std=c++17 queue.cpp -o queue
./queue
```

On Windows:

```powershell
g++ -std=c++17 queue.cpp -o queue.exe
.\queue.exe
```

> The console program calls `system("cls")` directly, which works on Windows only. The `CLEAR` macro at the top of the file is defined but not used by the menu.

### GUI version

The GUI uses [raylib](https://www.raylib.com/) 6.2 and MinGW-w64 on Windows:

```bat
cd Data_structurs\queue
build_gui.bat
gui.exe
```

If raylib is installed elsewhere, set `RAYLIB_HOME` before building:

```bat
set RAYLIB_HOME=C:\path\to\raylib
build_gui.bat
```

The script outputs `gui.exe` in the same folder. It does not modify or rebuild the console `queue.exe`.

Optional runtime flags:

```bat
gui.exe --demo          REM start with a demo queue
gui.exe --smoke-test    REM draw 30 frames and exit
```

## How to Use

### Console menu

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

### GUI

The GUI shows the queue in two complementary ways:

- **Logical queue:** up to ten elements in FIFO order, with the front on the left and the rear on the right.
- **Circular array:** all 100 slots in a 10 × 10 map, showing where `front` and `rear` currently point and how the queue wraps around the end of the array.

| Action | Control |
| --- | --- |
| Enqueue a value | Type an integer and press **Enter** or click **Enqueue** |
| Dequeue | Click **Dequeue** or press **D** |
| Peek | Click **Peek** or press **K** |
| Quit | **Esc** or close the window |

The **Enqueue** animation sends a card from the right side of the circular map into the new slot. **Dequeue** moves the front card out to the left and fades it. Green marks the front, orange marks the rear, and blue marks occupied slots. The right panel shows the size, front index, rear index, state (Empty / Ready / Full), and a capacity bar.

## How It Works

### Shared operations

The functions and globals are defined exactly as before in `queue.cpp`. `queue.h` is a copy of those definitions created for the GUI, and `gui.cpp` includes it. The GUI calls the existing `enqueue()`, `dequeue()`, `peek()`, `length()`, `isempty()`, and `is_full()` functions, and reads `queue[]`, `front`, `rear`, and `count` only to draw the current state. No queue logic was changed.

`display()` is still used by the console version. The GUI draws the logical queue and the circular map instead.

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

### GUI (`gui.cpp`)

The raylib program has a title bar, a logical-queue panel, a circular-array panel, a controls panel, and a status bar:

- `update()` reads typed digits, the Enter/D/K keys, and the buttons. It calls the same `enqueue()`, `dequeue()`, and `peek()` functions used by the console menu.
- `drawLanePanel()` shows up to ten elements in FIFO order. If the queue is longer, a `+N` chip represents the remaining elements.
- `drawGridPanel()` draws all 100 array slots. `isFilledIndex()` uses `front`, `count`, and `MAX` to decide which slots are occupied; `rear` is used to highlight the newest slot. The front is green and the rear is orange.
- Enqueue and dequeue store an `Animation` with a start and end rectangle. The animation is visual only; the data structure is updated immediately by the existing functions.
- The GUI reads the globals for rendering. It does not reimplement or modify any queue operation.

> The GUI code was generated with AI; the queue operations and console program are the author's original work. See the main [AI Assistance](../../README.md#ai-assistance) note.

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
- **GUI is a separate front end:** `gui.cpp` includes the copied operations in `queue.h`, while the original console program in `queue.cpp` remains unchanged.
- **AI-generated UI:** the raylib interface and animations were generated with AI; the queue operations are the author's original hand-written code.
- **Global state:** the array and indices are global variables.
- **Dequeue does not return the removed value.** The console menu only reports success/failure; the GUI records the front value before calling `dequeue()` so it can animate the removed card.
- **`count` is essential:** without it, `front == rear` would be ambiguous (empty or full). This implementation uses both `count` and the `-1` reset convention.
- **Windows-only clearing:** the console `system("cls")` is used directly even though a cross-platform `CLEAR` macro is defined at the top. The GUI does not use either.
- **No dynamic allocation:** the array lives for the whole program.
- **Invalid input:** the console menu handles out-of-range choices, but non-numeric input can leave `cin` in a failed state. The GUI only accepts digits and a leading minus sign.
- **The display is not a true queue operation:** it is a diagnostic helper that walks the logical order.

## What This Teaches

- The FIFO discipline and how it differs from LIFO (stack).
- How modulo arithmetic turns a linear array into a circular buffer.
- Why a separate `count` (or an empty slot) is needed to distinguish full from empty.
- Why circular queues avoid the O(n) shifting cost of a naive array queue.
- How to iterate over a wrapped range correctly.
