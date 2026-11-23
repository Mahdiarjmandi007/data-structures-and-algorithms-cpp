# Stack (Array Implementation) in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

## Overview

A **stack** is a linear data structure that follows **LIFO** (Last In, First Out) order: the most recently pushed element is the first one popped. This implementation uses a fixed-size array of 100 integers and an integer `top` that points to the last inserted element.

The program is an interactive console menu. Every operation prints the resulting stack contents, so the behavior is easy to observe while learning.

## Files

| File | Purpose |
| --- | --- |
| [`stack.cpp`](stack.cpp) | Complete implementation: array stack, operations, and menu |

## Build and Run

```bash
g++ -std=c++17 stack.cpp -o stack
./stack
```

On Windows:

```powershell
g++ -std=c++17 stack.cpp -o stack.exe
.\stack.exe
```

> The program calls `system("cls")` to clear the screen. That works on Windows; on Linux/macOS replace it with `system("clear")` in your local copy.

## How to Use

The menu has four options:

```text
Menu
1.push
2.pop
3.peek
4.exit
Enter The choice :
```

| Choice | Action | What it asks / prints |
| --- | --- | --- |
| `1` | Push | Asks for an integer, inserts it at the top, then prints the stack |
| `2` | Pop | Removes the top element, then prints the stack |
| `3` | Peek | Prints the current top value without removing it |
| `4` | Exit | Ends the program |

After push, pop, and peek the program waits for one character so you can read the result before the screen is cleared.

### Example session

```text
Menu
1.push
2.pop
3.peek
4.exit
Enter The choice :1
<<push>>
Enter the value :10

push is succesfully

stack : 10
Press any key to return:
```

After pushing `10`, `20`, and `30`, `display()` shows:

```text
stack : 10 20 30
```

`peek()` prints `30`. A `pop()` removes `30`, leaving `10 20`.

## How It Works

### Data representation

```cpp
const int MAX = 100;
int stack[MAX];
int top = -1;
```

`top == -1` means the stack is empty. When the stack contains `n` elements, `top == n - 1` and the valid elements are `stack[0]` through `stack[top]`.

```text
top = 2
+------+------+------+------+
|  10  |  20  |  30  |      |
+------+------+------+------+
   0      1      2      3
```

### `push(int value)`

1. Checks `top >= MAX - 1`. If true, the stack is full and the function returns `false`.
2. Increments `top`.
3. Stores `value` at `stack[top]`.
4. Returns `true`.

Time: **O(1)**.

### `pop()`

1. Checks `top == -1`. If true, the stack is empty and the function returns `false`.
2. Decrements `top`, which logically removes the top element.
3. Returns `true`.

The value itself is not returned or printed. The array slot still contains the old bytes, but it is outside the logical stack because `top` no longer includes it.

Time: **O(1)**.

### `peek()`

Checks for an empty stack and prints `stack[top]` if one exists. It does not modify `top`.

Time: **O(1)**.

### `display()`

Loops from index `0` to `top` and prints each element, so the output is ordered **bottom to top**:

```text
stack : 10 20 30
```

Time: **O(n)**, where `n` is the number of elements.

### `main()`

The `while (1)` loop prints the menu, reads a choice, and dispatches it through a `switch`. After each operation it prints the boolean result, displays the stack, waits for a character, and clears the screen.

## Complexity Summary

| Operation | Time | Space |
| --- | --- | --- |
| `push` | O(1) | O(1) |
| `pop` | O(1) | O(1) |
| `peek` | O(1) | O(1) |
| `display` | O(n) | O(1) |
| Entire stack | — | O(MAX) = O(100) |

## Notes and Limitations

- **Fixed capacity:** `MAX` is 100. There is no dynamic resizing.
- **Global state:** `stack` and `top` are global variables. A class-based version would keep them as members.
- **Pop does not return a value:** the menu only reports success/failure. If you need the removed value, change `pop()` to return it (or use an output parameter).
- **Fixed element type:** only `int` values are supported.
- **Windows-only screen clearing:** `main()` uses `system("cls")`, while the `CLEAR` macro defined at the top is not actually used.
- **Small cosmetic issue:** the overflow message `"stack is full"` has no newline, so following output can appear on the same line.
- **No exception handling:** invalid menu input is reported by the `default` branch, but non-numeric input can confuse `cin` and leave it in a failed state.

## What This Teaches

- The relationship between `top` and the number of elements.
- Why push and pop are O(1) when the top is tracked directly.
- The difference between a logical size and the physical array capacity.
- Why overflow and underflow checks are required before every operation.
- How a simple array can implement an abstract data type without any dynamic memory allocation.
