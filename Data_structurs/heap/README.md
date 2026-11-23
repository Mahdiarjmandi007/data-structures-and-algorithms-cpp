# Min Heap and Max Heap in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

## Overview

A **binary heap** is a complete binary tree stored in an array. It satisfies the **heap property**:

- **Min heap:** every parent is less than or equal to its children, so the smallest element is at the root.
- **Max heap:** every parent is greater than or equal to its children, so the largest element is at the root.

This program builds **both** a min heap and a max heap from the same input list, supports inserting and deleting values, and prints either heap in list form and as an ASCII tree. It is an interactive console menu.

## Files

| File | Purpose |
| --- | --- |
| [`heap.cpp`](heap.cpp) | Complete implementation: heap construction, insert, delete, and display |

## Build and Run

```bash
g++ -std=c++17 heap.cpp -o heap
./heap
```

On Windows:

```powershell
g++ -std=c++17 heap.cpp -o heap.exe
.\heap.exe
```

The program defines a cross-platform `CLEAR` macro (`cls` on Windows, `clear` elsewhere), so it works on both Windows and Unix-like terminals.

## How to Use

```text
**********Menu**********
1.Insert Primary number of heap
2.Insert number
3.Delete number
4.Display
5.Exit
```

| Choice | Action | Input |
| --- | --- | --- |
| `1` | Build the heaps from a list | A sequence of integers ending with `0` |
| `2` | Insert one value | An integer |
| `3` | Delete a value | An integer |
| `4` | Display a heap | `1` for min heap, `2` for max heap |
| `5` | Exit | — |

**Option 1 must be used first.** Until a primary list has been created, options `2`, `3`, and `4` print a warning and return to the menu.

The input list is terminated by `0`, so `0` cannot be stored as a heap value. Enter at least one non-zero value before `0`; the heap-building functions read `list[0]` and assume the list is not empty.

### Example session

Build from `5 3 8 1 9 0`:

- Min heap list: `[1, 3, 8, 5, 9]`
- Max heap list: `[9, 8, 5, 1, 3]`

Min heap tree view:

```text
      1
   3     8
 5   9
```

Max heap tree view:

```text
      9
   8     5
 1   3
```

Insert `2`, then display the min heap:

```text
List View:
[1, 3, 2, 5, 9, 8]

Tree View:
      1
   3     2
 5   9 8
```

Delete `3`, then display again to see the heap restored by sifting down:

```text
List View:
[1, 5, 2, 8, 9]

Tree View:
      1
   5     2
 8   9
```

## How It Works

### Array representation

A binary heap is a complete binary tree, so it can be stored in a `std::vector<int>` with no pointers. For a node at index `i`:

| Relationship | Index |
| --- | --- |
| Parent | `(i - 1) / 2` |
| Left child | `2 * i + 1` |
| Right child | `2 * i + 2` |

```text
index:        0
            /   \
          1       2
         / \     / \
        3   4   5   6

values:   1
        /   \
       3     8
      / \   /
     5   9 2
```

### Building the heaps

`create_minheap` and `create_maxheap` use the **repeated insertion** method:

1. Copy the first value into the heap vector.
2. For each remaining value, append it to the end.
3. **Sift up:** while the new value violates the heap property with its parent, swap them and move up.

For a min heap the comparison is `child < parent`; for a max heap it is `child > parent`.

This build method costs **O(n log n)**. A bottom-up heapify would build the same heap in **O(n)**, but repeated insertion is easier to follow and matches the insert operation.

### `insert_num(bool isMin, int number, vector<int>& heap)`

Appends `number` to the vector, then sifts it up:

- `isMin == true`: swap while `heap[child] < heap[parent]`.
- `isMin == false`: swap while `heap[child] > heap[parent]`.

Time: **O(log n)** because the height of a complete binary tree is `O(log n)`.

The menu calls this function twice, once for each heap, so both structures stay in sync.

### `find_index(int number, vector<int>& heap)`

Uses `std::find` to locate the first occurrence of a value. Returns the index or `-1`.

Time: **O(n)**. A heap is not ordered like a binary search tree, so locating an arbitrary value requires a scan.

### `Delet(bool isMin, int index, vector<int>& heap)`

Deletes the element at a known index:

1. Replace `heap[index]` with the last element.
2. Remove the last element with `pop_back()`.
3. **Sift down:** repeatedly compare the replacement with its children.
   - Min heap: swap with the smaller child while it is smaller than the parent.
   - Max heap: swap with the larger child while it is larger than the parent.
4. Stop when the heap property holds or the node becomes a leaf.

Time: **O(log n)** after the index is known. The menu's overall delete is **O(n)** because it first calls `find_index` on both heaps.

Deleting the root is the standard heap operation; this program supports deleting an arbitrary value by index, which is why it needs the linear search.

### `Disply(vector<int>& heap)`

Prints the heap level by level:

1. Computes the number of levels: `floor(log2(n)) + 1`.
2. For each level, prints a leading space block and then the nodes of that level with spaces between them.
3. Uses `1 << level` to count the nodes on a level (`2^level`).

It is a compact level-order visualization, not a fully connected tree drawing. The spacing is not adjusted for multi-digit numbers, and it does not print `/` and `\` connectors. The ANSI escape code `\033[1;32m` colors the values green.

Time: **O(n)** for visiting all nodes, plus the printed padding.

### Menu state

`main()` keeps a global-ish `status` flag (a local variable in `main`) to enforce the order:

- Choosing `1` sets `status = 1` and builds both heaps.
- If `status == 0`, options `2`–`4` are rejected with a warning.
- Choosing `1` again clears both the old primary list and the heaps and starts over.

## Complexity Summary

| Operation | Time | Extra space |
| --- | --- | --- |
| Build min heap | O(n log n) | O(n) |
| Build max heap | O(n log n) | O(n) |
| Insert into one heap | O(log n) | O(1) |
| Find an arbitrary value | O(n) | O(1) |
| Delete by value | O(n) + O(log n) = O(n) | O(1) |
| Delete at a known index | O(log n) | O(1) |
| Display | O(n) | O(1) |
| Both heaps | — | O(n) each |

## Notes and Limitations

- **`0` is a sentinel:** it terminates the input list and cannot be a heap value.
- **The first input must not be `0`:** if the list is empty, `create_minheap` and `create_maxheap` access `list[0]` out of bounds.
- **Repeated-insertion build:** this is O(n log n), not the optimal O(n) bottom-up heapify.
- **Arbitrary deletion is expensive:** finding a value in a heap is O(n); heaps are optimized for accessing/removing the root.
- **Duplicates:** `find_index` returns the first match, so deleting a duplicated value removes one occurrence. Both heaps receive the same deletion.
- **Two independent vectors:** the min heap and max heap are separate copies; there is no shared node structure.
- **No extract-min/extract-max command:** the menu deletes by value rather than removing the root directly.
- **Display is approximate:** the tree view uses fixed spacing and does not connect nodes with lines or align multi-digit values.
- **ANSI colors:** the output contains ANSI escape codes, which may not render in older Windows consoles or when redirected to a file.
- **Function name typo:** `Delet` is spelled without the final `e` in the source.
- **The list view uses backspace characters** (`\b\b`) to remove the trailing comma and space. This is a terminal trick, not a portable formatting strategy.

## What This Teaches

- How the array representation of a complete binary tree works.
- Why parent and child indices have simple formulas.
- How sift-up (insert) and sift-down (delete) restore the heap property.
- Why heap insertion and deletion are O(log n).
- Why searching for an arbitrary value in a heap is O(n).
- The difference between building a heap by repeated insertion and by bottom-up heapify.
- How a min heap and a max heap differ only in their comparison direction.
