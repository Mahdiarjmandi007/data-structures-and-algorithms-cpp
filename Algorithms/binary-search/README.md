# Binary Search in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

> **Version:** CLI only. This folder does not include a UI version.

## Overview

**Binary search** finds a target value in a **sorted** array by repeatedly cutting the search interval in half. It compares the target with the middle element:

- If they are equal, the index is found.
- If the target is larger, the answer must be in the right half.
- If the target is smaller, the answer must be in the left half.

Each comparison discards half of the remaining elements, which gives **O(log n)** time.

This file is a small fixed-array demonstration rather than an interactive program. The target is hard-coded as `n = 9`, and the program prints the 0-based index where that value is found.

## Files

| File | Purpose |
| --- | --- |
| [`binary_search.cpp`](binary_search.cpp) | Fixed-array binary search demo |

## Build and Run

```bash
g++ -std=c++17 binary_search.cpp -o binary_search
./binary_search
```

On Windows:

```powershell
g++ -std=c++17 binary_search.cpp -o binary_search.exe
.\binary_search.exe
```

The program reads no input. It prints:

```text
8
```

`9` is at index `8` in the array `{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}`.

To test other values, edit the line `int n = 9;` in `main()` and rebuild.

## How It Works

### The data

```cpp
int n = 9;
int nums[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int low = 0;
int high = sizeof(nums) / sizeof(nums[0]);   // 10
int mid = (high + low) / 2;                   // 5
```

- `nums` is sorted in ascending order, which is required for binary search.
- `low` is the first candidate index.
- `high` is initialized to the **number of elements** (10), not the last index (9). The code still finds the hard-coded target, but this is a nonstandard bound; see the limitations.
- `mid` starts at `(0 + 10) / 2 = 5`, whose value is `6`.

### The loop

```cpp
while (1) {
    if (n == nums[mid]) {
        cout << mid;
        break;
    }
    else if (n > nums[mid]) {
        low = mid + 1;
        mid = (high + low) / 2;
    }
    else if (nums[mid] > n) {
        high = mid - 1;
        mid = (high + low) / 2;
    }
    else {
        cout << "n not in array";
        break;
    }
}
```

In words:

1. Compare the target with `nums[mid]`.
2. If equal, print `mid` and stop.
3. If the target is larger, move `low` to `mid + 1` and recompute `mid`.
4. If the target is smaller, move `high` to `mid - 1` and recompute `mid`.
5. The final `else` is intended for "not found".

The index printed is **0-based**, so `9` is reported as `8`.

### Trace for `n = 9`

| Step | `low` | `high` | `mid` | `nums[mid]` | Decision |
| --- | --- | --- | --- | --- | --- |
| 1 | 0 | 10 | 5 | 6 | `9 > 6` → go right |
| 2 | 6 | 10 | 8 | 9 | equal → print `8` |

Only two comparisons are needed for ten elements. A linear search would need nine comparisons to reach the same value.

## Complexity Summary

| Case | Time | Space |
| --- | --- | --- |
| Best | O(1) — target is the first middle checked | O(1) |
| Average | O(log n) | O(1) |
| Worst | O(log n) for a valid array range | O(1) |

The array is scanned only through index arithmetic; no extra data structures are allocated.

## Notes and Limitations

This is an educational demo, and it makes several simplifying assumptions that a robust binary search should not make:

- **No bounds check / termination guard:** the loop is `while (1)` and has no `low <= high` condition. If the target is outside the array (for example `0` or `11`), the indices can move out of range, causing an out-of-bounds read or an infinite loop.
- **`high` is the array length:** `high = 10` points one past the last valid index (`9`). The standard initialization is `high = n - 1`. The hard-coded target happens to be found anyway.
- **The final `else` is unreachable:** if `n != nums[mid]`, then either `n > nums[mid]` or `nums[mid] > n`. The `"n not in array"` branch can never execute, so a missing value is never reported.
- **Fixed input:** the array and target are hard-coded. There is no user input, no function parameter, and no return value.
- **No duplicate handling:** the algorithm returns the first middle that matches; it does not find the first or last occurrence of a duplicated value.
- **Assumes a sorted, ascending array:** binary search is invalid on unsorted data.
- **Integer midpoint:** `(low + high) / 2` can overflow if both indices are near `INT_MAX`. The safer form is `low + (high - low) / 2`.
- **No newline after output:** `cout << mid` does not print a line break.

### A robust version

For reference, a conventional iterative implementation looks like this:

```cpp
int binary_search(const vector<int>& a, int target) {
    int low = 0;
    int high = (int)a.size() - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (a[mid] == target) return mid;
        if (a[mid] < target) low = mid + 1;
        else                 high = mid - 1;
    }

    return -1;   // not found
}
```

That version handles missing values, avoids out-of-range access, and uses the overflow-safe midpoint formula.

## What This Teaches

- Why binary search requires sorted data.
- How halving the interval produces logarithmic time.
- How `low`, `high`, and `mid` move during the search.
- Why the loop condition and bounds checks are essential for correctness.
- How a seemingly harmless bound choice (`high = size` instead of `size - 1`) and a missing termination condition can create edge-case bugs.
- Why 0-based indexing matters when interpreting the result.
