# Complete Binary Tree in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

## Overview

A **complete binary tree** is a binary tree in which every level is completely filled except possibly the last level, and the last level is filled from left to right. This implementation builds such a tree from user input using **level-order insertion** and prints it as an ASCII diagram.

The program does not use a menu. It reads the root value first, then keeps reading values until it sees `0`, inserting each one at the first available position in level order.

## Files

| File | Purpose |
| --- | --- |
| [`complete-binary-tree.cpp`](complete-binary-tree.cpp) | Complete implementation: node, insertion queue, height, and tree printing |

## Build and Run

```bash
g++ -std=c++17 complete-binary-tree.cpp -o complete-tree
./complete-tree
```

On Windows:

```powershell
g++ -std=c++17 complete-binary-tree.cpp -o complete-tree.exe
.\complete-tree.exe
```

## How to Use

The input format is:

```text
<root> <value> <value> ... 0
```

- The **first number** becomes the root.
- Every following number is inserted at the next free position in level order.
- `0` ends the input and triggers printing.
- `0` cannot be stored as a node value.

### Example

Input:

```text
1 2 3 4 5 6 7 0
```

Output:

```text
        1
        /\
    2        3
    /\        /\
  4    5    6    7
```

The same values create this tree:

```text
          1
        /   \
       2     3
      / \   / \
     4   5 6   7
```

If the input stops earlier, for example `1 2 3 4 5 0`, the last level is filled from the left:

```text
        1
        /\
    2        3
    /\
  4    5
```

## How It Works

### Node structure

```cpp
struct node {
    int data;
    node* right;
    node* left;
};
```

The tree is referenced through a global root pointer:

```cpp
node* head = NULL;
```

### The insertion queue

```cpp
std::queue<node*> q;
```

The queue stores nodes that may still have an empty child slot. It is the key to level-order insertion:

1. Look at the front node.
2. If its left child is empty, attach the new node there.
3. Otherwise, if its right child is empty, attach the new node there.
4. Otherwise, the front node is full: pop it and try the next node.

After attaching a child, the new node is pushed onto the queue because it may receive children later.

A trace for `1 2 3 4 5`:

| Step | Value | Front of queue | Action | Queue after |
| --- | --- | --- | --- | --- |
| 1 | `1` | — | Create root | `[1]` |
| 2 | `2` | `1` | Attach as `1.left` | `[1, 2]` |
| 3 | `3` | `1` | Attach as `1.right` | `[1, 2, 3]` |
| 4 | `4` | `1` | `1` is full; pop; front becomes `2`; attach as `2.left` | `[2, 3, 4]` |
| 5 | `5` | `2` | Attach as `2.right` | `[2, 3, 4, 5]` |

This is exactly the shape produced by a level-order (BFS) traversal, so the resulting tree is complete.

### `insert_bigroot(int value)`

Creates the root node, sets both children to `NULL`, assigns `head`, and pushes the root into the queue. If `head` is already set, the function does nothing, so it is only intended for the first value.

Time: **O(1)**.

### `insert_child(int value)`

Implements the queue-based insertion described above. The `while (!q.empty())` loop pops full parents until it finds a free slot. Each node is popped at most once over the lifetime of the tree, so insertion is **O(1) amortized**.

### `height(node* root)`

Recursively computes:

```text
height(root) = 1 + max(height(left), height(right))
```

An empty subtree has height `0`. Time: **O(n)**, recursion depth **O(h)**.

### `print_tree(node* root)`

1. Computes the tree height `h`.
2. Starts with `curr_level = [root]`.
3. For each level:
   - Calculates horizontal spacing from `max_width = 2^h * 2` and the current level.
   - Prints each node's value (or a blank for a missing child).
   - Prints a connector line with `/` and `\` where children exist.
   - Builds the next level, inserting `nullptr` placeholders for missing nodes.
4. Stops after `h` levels.

The drawing is an approximation. It is readable for small trees, but the spacing is based on powers of two and does not account for multi-digit values, so large trees or large numbers can look misaligned.

Time is proportional to the number of nodes plus the printed padding; with the current spacing the output can be **O(n log n)** characters. Auxiliary space is **O(n)** for the level vectors.

## Complexity Summary

| Operation | Time | Extra space |
| --- | --- | --- |
| Insert root | O(1) | O(1) |
| Insert next value | O(1) amortized | O(1) |
| Compute height | O(n) | O(h) recursion |
| Print tree | O(n) nodes / O(n log n) output | O(n) |
| Build whole tree | O(n) | O(n) |

## Notes and Limitations

- **`0` is a sentinel:** it terminates input, so the value `0` cannot be a node.
- **A root is mandatory:** the program reads one value before the loop. It cannot build an empty tree.
- **Complete-tree shape only:** insertion fills the next level-order slot. This is not a binary search tree, and the values are not ordered.
- **No search, delete, or traversal API:** the only output is the printed tree.
- **Global state:** `head` and the queue `q` are global and are never reset during the run.
- **Approximate drawing:** the ASCII layout uses fixed powers of two and does not measure the width of printed numbers. Multi-digit values can shift the diagram.
- **No allocation-failure handling:** `new node` is assumed to succeed.
- **The queue is not cleared at the end:** it still contains leaf nodes when the program exits.
- **Recursive height:** a degenerate tree with `n` nodes would use `O(n)` call stack, although a complete tree has height `O(log n)`.

## What This Teaches

- The definition of a complete binary tree and why it has a compact array representation.
- How a queue performs level-order (BFS) insertion.
- Why each node can be popped at most once, giving amortized O(1) insertion.
- How recursion computes tree height.
- How to render a tree level by level, including placeholders for missing children.
- Why a complete binary tree is the shape behind binary heaps.
