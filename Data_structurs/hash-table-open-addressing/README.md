# Hash Table with Open Addressing (Linear Probing) in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

## Overview

A **hash table** stores key-value pairs and uses a hash function to map a key to an array index. When two keys map to the same index, a **collision** occurs. This implementation resolves collisions with **open addressing** and **linear probing**: if a slot is occupied, the insertion tries the next slot, then the next, wrapping around at the end of the table.

The table has a fixed size of 10 integers and uses three slot states:

| State | Meaning |
| --- | --- |
| `EMPTY` | The slot has never been used and is available |
| `FILLED` | The slot currently holds a key |
| `DELETED` | The slot held a key that was deleted (a tombstone) |

The program is an interactive console menu for inserting, deleting, and displaying the table.

## Files

| File | Purpose |
| --- | --- |
| [`hash-table-open-addressing.cpp`](hash-table-open-addressing.cpp) | Complete implementation: states, hash function, linear probing, and menu |

## Build and Run

```bash
g++ -std=c++17 hash-table-open-addressing.cpp -o hash-table
./hash-table
```

On Windows:

```powershell
g++ -std=c++17 hash-table-open-addressing.cpp -o hash-table.exe
.\hash-table.exe
```

> The menu calls `system("cls")`, which is Windows-specific. On Linux/macOS, replace it with `system("clear")` in your local copy.

## How to Use

```text
menu:
 1.Insert:
 2.Delet:
 3.Display:
 4.Exit:
Enter Your Choice:
```

| Choice | Action | Input |
| --- | --- | --- |
| `1` | Insert | An integer key |
| `2` | Delete | An integer key |
| `3` | Display | — (prints every slot) |
| `4` | Exit | — |

### Example session

Insert `12`, `22`, and `32`:

```text
Enter Your Choice:1
Enter The Number To Insert12

Enter Your Choice:1
Enter The Number To Insert22

Enter Your Choice:1
Enter The Number To Insert32
```

Display:

```text
0 -> empty
1 -> empty
2 -> 12
3 -> 22
4 -> 32
5 -> empty
6 -> empty
7 -> empty
8 -> empty
9 -> empty
```

All three keys hash to index `2`, so linear probing places them at `2`, `3`, and `4`.

Now delete `12` and insert `42`. Both keys hash to index `2`:

```text
2 -> deleted
3 -> 22
4 -> 32
5 -> 42
```

The tombstone at index `2` is not treated as free by the current insert function; it keeps probing, finds `3` and `4` occupied, and places `42` at index `5`.

## How It Works

### Constants and states

```cpp
#define TABLESIZE 10

enum State {
    FILLED,
    EMPTY,
    DELETED
};

struct node {
    int key;
    State state = EMPTY;
};

node HashTable[TABLESIZE];
```

`FILLED`, `EMPTY`, and `DELETED` have the integer values `0`, `1`, and `2`. The default member initializer makes every slot `EMPTY` at program start. The table is global and stores only integer keys (the key is also the value).

### `HashFunc(int key)`

```cpp
return key % TABLESIZE;
```

The index is the remainder of the key divided by 10, so keys `12`, `22`, and `32` all map to `2`.

### `Insert_num(int num)`

```text
for i = 0 .. 9:
    index = (hash(num + i)) % 10
    if table[index] is EMPTY:
        store num at index
        mark FILLED
        return
print "TABLE is Full"
```

Because `HashFunc` already applies the modulo, the expression `HashFunc(num + i) % TABLESIZE` applies it twice; the result is the same. The loop checks every slot, so it can find a free slot even after wrapping around the end of the array.

The function only accepts `EMPTY` slots. It does **not** reuse `DELETED` (tombstone) slots. This is a deliberate simplification for learning, but it causes a problem described in the limitations section.

Time: **O(n)** in the worst case (scanning the table), **O(1)** on average when the table is not too full.

### `Delete_num(int num)`

```text
for i = 0 .. 9:
    index = hash(num + i) % 10
    if table[index].key == num:
        mark index DELETED
        set key to 0
        return
print "The value is not in hash table or Deleted."
```

Deletion does not remove the slot physically. Marking it `DELETED` preserves the probing chain: if the slot became `EMPTY`, a later search for a key that had probed past it might stop too early. The tombstone says "this slot is free for reuse, but it is not the end of a cluster."

The current search loop checks `key` without checking `state`, so a key of `0` can match an untouched slot whose default key is also `0`. See the limitations.

Time: **O(n)** worst case, **O(1)** average.

### `display()`

Iterates over all 10 slots:

- `FILLED` prints `index -> key`.
- `EMPTY` prints `index -> empty` (the `case 1` branch).
- `DELETED` prints `index -> deleted` (the `case 2` branch).

Time: **O(TABLESIZE)** = **O(1)** for a fixed table.

### `main()`

The usual menu loop reads a choice, dispatches it, and clears the screen with `system("cls")`. The table is global, so all operations see the same state.

## Complexity Summary

| Operation | Average | Worst | Space |
| --- | --- | --- | --- |
| Insert | O(1) | O(n) | O(1) |
| Delete | O(1) | O(n) | O(1) |
| Display | O(n) | O(n) | O(1) |
| Entire table | — | — | O(n) fixed at 10 |

Average-case performance depends on the **load factor** `n / TABLESIZE` and on how uniformly the hash function distributes keys. Linear probing degrades as the table fills because occupied slots form contiguous clusters.

## Notes and Limitations

- **Deleted slots are not reused by insert:** because `Insert_num` only accepts `EMPTY`, tombstones accumulate. After enough deletions, the table can report `"TABLE is Full"` even though `DELETED` slots are available.
- **Delete checks the key but not the state:** empty slots have `key == 0`, so deleting `0` can mark an arbitrary empty slot as `DELETED` and report success. Keys should be positive in this demo.
- **Negative keys are unsafe:** in C++, `-1 % 10` is `-1`, which would index outside the array. Use non-negative keys.
- **Fixed capacity:** `TABLESIZE` is 10 and there is no resizing or rehashing.
- **No duplicate check:** inserting the same key twice stores it twice; deletion removes the first matching slot it encounters.
- **Linear probing only:** no quadratic probing, double hashing, or separate chaining.
- **The delete loop scans the entire table:** it does not stop at the first `EMPTY` slot, so the implementation is simple but does more work than necessary.
- **The `switch` in `display()` is indirect:** it relies on the numeric values of the enum (`EMPTY == 1`, `DELETED == 2`) instead of comparing the enum names.
- **Windows-only clearing:** `system("cls")` is used in `main()`.
- **No key-value pairs:** each slot stores only one integer, so the key is also the payload.

## What This Teaches

- How a hash function maps keys to array indices.
- Why collisions are unavoidable and how linear probing resolves them.
- Why deletion in open addressing needs tombstones instead of simply clearing a slot.
- How the load factor and clustering affect performance.
- The difference between average-case O(1) and worst-case O(n) behavior.
- Why a fixed-size table eventually runs out of usable slots without resizing.

## A More Complete Version

A production-quality open-addressing table would typically:

1. Reuse `DELETED` slots during insertion and stop at the first `EMPTY` during lookup.
2. Check the `state` before comparing keys.
3. Resize and rehash when the load factor crosses a threshold.
4. Store explicit key-value pairs and support generic key types.
5. Use a stronger probing strategy (for example double hashing) to reduce clustering.
