# Quick Sort in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

## Overview

**Quick sort** is a divide-and-conquer sorting algorithm. It chooses a **pivot**, partitions the array so that elements smaller than the pivot are on its left and larger elements are on its right, then recursively sorts the two partitions.

This implementation uses the **Lomuto partition scheme** with the **last element as the pivot**. It is wrapped in an interactive menu: the user enters a list, the program sorts a copy of it, and both the original and sorted lists can be displayed.

## Files

| File | Purpose |
| --- | --- |
| [`quick_sort.cpp`](quick_sort.cpp) | Complete implementation: partition, recursive quick sort, and menu |

## Build and Run

```bash
g++ -std=c++17 quick_sort.cpp -o quick_sort
./quick_sort
```

On Windows:

```powershell
g++ -std=c++17 quick_sort.cpp -o quick_sort.exe
.\quick_sort.exe
```

## How to Use

```text
**********Menu**********
1.Insert Primary number for list
2.Display
3.Exit
```

| Choice | Action | Input |
| --- | --- | --- |
| `1` | Enter a new list and sort it | Integers ending with `0` |
| `2` | Display the original and sorted lists | — |
| `3` | Exit | — |

- `0` terminates the input list and is not stored.
- Choosing `1` again clears the previous lists and starts fresh.
- Sorting happens immediately after the input is finished.
- The original list is preserved; quick sort runs on a copy.

### Example session

Input:

```text
1
5 3 8 1 2 0
2
3
```

Output:

```text
**********Menu**********
1.Insert Primary number for list
2.Display
3.Exit
Insert your list (Enter 0 to finish):
**********Menu**********
1.Insert Primary number for list
2.Display
3.Exit
Primary List: 5 3 8 1 2 
Sorted List: 1 2 3 5 8 
**********Menu**********
1.Insert Primary number for list
2.Display
3.Exit
```

## How It Works

### `create_primary_list(int number, vector<int>& list)`

Appends `number` to the input vector if it is not `0`. The main loop already stops at `0`, so the check is redundant in the current flow but keeps the helper safe on its own.

### `display_list(const vector<int>& list)`

Prints every element separated by a space, then a newline. An empty list simply prints a blank line.

### `partitions(vector<int>& list, int low, int high)`

This is the Lomuto partition:

```cpp
int pivot = list[high];
int i = low - 1;

for (int j = low; j < high; j++) {
    if (list[j] < pivot) {
        i++;
        swap(list[i], list[j]);
    }
}

swap(list[i + 1], list[high]);
return i + 1;
```

- `pivot` is the last element of the current range.
- `i` marks the end of the "smaller than pivot" region.
- `j` scans the range.
- When `list[j] < pivot`, the element is swapped into the smaller region.
- After the scan, the pivot is swapped into `i + 1`, which is its final sorted position.
- The function returns that position.

Time: **O(n)** for a range of `n` elements (one pass).

### Trace of one partition

Starting range: `[5, 3, 8, 1, 2]`, pivot `2`, `i = -1`.

| `j` | `list[j]` | `list[j] < 2`? | Action | Array |
| --- | --- | --- | --- | --- |
| 0 | 5 | no | — | `[5, 3, 8, 1, 2]` |
| 1 | 3 | no | — | `[5, 3, 8, 1, 2]` |
| 2 | 8 | no | — | `[5, 3, 8, 1, 2]` |
| 3 | 1 | yes | `i = 0`, swap `list[0]` and `list[3]` | `[1, 3, 8, 5, 2]` |
| 4 | 5 | no | — | `[1, 3, 8, 5, 2]` |

Final swap: `list[1]` and `list[4]` → `[1, 2, 8, 5, 3]`, return `1`. The pivot `2` is now at its final position.

### `quick_sort(vector<int>& list, int low, int high)`

```cpp
if (low < high) {
    int p = partitions(list, low, high);
    quick_sort(list, low, p - 1);
    quick_sort(list, p + 1, high);
}
```

The base case is a range with zero or one element (`low >= high`), which is already sorted. The partition returns the pivot's final index, so the pivot is excluded from both recursive calls.

Recursion tree for the example:

```text
[5, 3, 8, 1, 2]
        pivot 2 at index 1
       /                  \
    [1]                 [8, 5, 3]
                          pivot 3 at index 2
                         /                  \
                      []                  [5, 8]
                                            pivot 8 at index 4
                                           /
                                        [5]
```

The final array is `[1, 2, 3, 5, 8]`.

### `main()`

The menu loop reads a choice:

- `1` clears both vectors, reads values until `0`, copies the primary list into `sorted_list`, and calls `quick_sort` if there is more than one element.
- `2` prints both lists.
- `3` exits.
- Any other choice prints a red warning message.

## Complexity Summary

| Case | Time | Recursion space |
| --- | --- | --- |
| Best | O(n log n) — balanced partitions | O(log n) |
| Average | O(n log n) | O(log n) |
| Worst | O(n²) — maximally unbalanced partitions | O(n) |

The algorithm sorts **in place** inside `sorted_list` and uses only the recursion stack and a few indices. It is **not stable**: equal elements can change their relative order because of the swaps.

### Why the worst case happens

With the last element as pivot:

- An already sorted array produces a partition with `n - 1` elements on one side and `0` on the other at every level.
- A reverse-sorted array does the same.
- The recurrence becomes `T(n) = T(n - 1) + O(n)`, which is `O(n²)`.

Randomized or median-of-three pivot selection reduces the chance of this behavior. A three-way partition handles many duplicate values better.

## Notes and Limitations

- **`0` is a sentinel:** it terminates input and cannot be sorted as a value.
- **Pivot choice:** always the last element, so sorted and reverse-sorted inputs hit the worst case.
- **Recursion depth:** worst-case depth is `O(n)`, which could overflow the stack for very large inputs.
- **Not stable:** equal elements may be reordered.
- **Only integers:** the implementation is hard-coded for `vector<int>`.
- **Sorting happens on a copy:** `sorted_list = primary_list` preserves the original order for comparison, at the cost of O(n) extra memory.
- **No sorting on demand:** the list is sorted immediately after entry; option `2` only displays the stored result.
- **Empty input:** if the first value is `0`, both lists are empty. `display_list` prints blank lines, and sorting is skipped because `n > 1` is false.
- **Menu input errors:** non-numeric input can leave `cin` in a failed state; the `default` branch only handles out-of-range numeric choices.
- **Cosmetic warning typo:** the message says `"Warninng!!!"`.
- **No step-by-step visualization:** the partition trace exists in this README, but the program only prints the final result.

## What This Teaches

- The divide-and-conquer structure of quick sort.
- How Lomuto partitioning places the pivot in its final position.
- Why the average case is O(n log n) and the worst case is O(n²).
- Why pivot selection matters.
- The difference between in-place sorting and stability.
- How recursion depth depends on how balanced the partitions are.
