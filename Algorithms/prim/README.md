# Prim's Minimum Spanning Tree in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

> **Status: algorithm core not implemented yet.** The graph model and a graph builder GUI exist, but the GUI in this folder is currently an unmodified copy of the Kruskal GUI, and `main.cpp` is still a placeholder. There is no Prim computation or Prim-specific visualization yet.

## Overview

A **minimum spanning tree (MST)** of a connected, weighted, undirected graph is a set of `V - 1` edges that connects all `V` vertices without cycles and has the smallest possible total weight.

**Prim's algorithm** grows the MST from a starting vertex:

1. Start with one vertex in the MST and all other vertices outside it.
2. Repeatedly choose the **cheapest edge** that connects a vertex inside the MST to a vertex outside it.
3. Add that edge and the new vertex to the MST.
4. Repeat until all vertices are in the tree (or no crossing edge remains, which means the graph is disconnected).

Unlike Kruskal, which sorts all edges globally, Prim focuses on the **frontier** of a growing tree. It is often preferred for dense graphs.

This folder currently contains a copy of the Kruskal graph model and GUI. The actual Prim implementation is still to be written.

## Files

| File | Purpose | Status |
| --- | --- | --- |
| [`graph.h`](graph.h) | Shared `Edge`/`Graph` model and graph operations | Implemented (copy of the Kruskal version) |
| [`gui.cpp`](gui.cpp) | raylib graph builder GUI | Implemented, but still the Kruskal UI (AI-generated) |
| [`main.cpp`](main.cpp) | Placeholder for the Prim implementation | Not implemented |
| [`build_gui.bat`](build_gui.bat) | Windows/MinGW build script for `gui.exe` | Implemented |

A built `gui.exe` may exist locally, but `*.exe` is ignored by `.gitignore`, so it is not part of the committed repository.

## Build and Run

### Requirements

- Windows
- MinGW-w64 `g++` with C++17 support
- [raylib](https://www.raylib.com/) 6.2

### Build

```bat
cd Algorithms\prim
build_gui.bat
```

The script defaults to:

- `RAYLIB_HOME=D:\toll\raylib-6.2-ucrt`
- compiler `D:\toll\mingw64\bin\g++.exe` (falls back to `g++` from `PATH` if missing)

Set `RAYLIB_HOME` if raylib is installed elsewhere:

```bat
set RAYLIB_HOME=C:\path\to\raylib
build_gui.bat
```

### Run

If a `gui.exe` is already present in the folder, you can run it directly. Otherwise build it with `build_gui.bat` first.

```bat
gui.exe
```

Useful switches:

```bat
gui.exe --demo          REM open a 6-vertex demo graph
gui.exe --smoke-test    REM draw 30 frames and exit
```

## Current GUI (Copied from Kruskal)

> **This is important:** `gui.cpp` in this folder is currently an exact copy of `Algorithms/kruskal/gui.cpp`. The window title is still `"Graph Builder - Kruskal"` and the subtitle still says `"Kruskal visualizer"`. The controls build a weighted graph; they do not run Prim.

### Setup screen

1. Enter a vertex count from **1 to 30** using the number keys, the `+`/`-` buttons, or the up/down arrow keys.
2. Press **Enter** or click **Create Graph**.
3. Press **Esc** to quit.

### Graph builder controls

| Action | Control |
| --- | --- |
| Select the first endpoint | Left-click a vertex |
| Choose the second endpoint | Left-click another vertex |
| Open the weight dialog | Automatically, after choosing two vertices |
| Confirm an edge | Type a positive integer and press **Enter** or click **Add Edge** |
| Cancel edge creation | **Esc**, right-click, or **Cancel** |
| Clear the current selection | **Esc**, right-click, or click empty canvas |
| Move a vertex | Press and drag it (more than 4 pixels) |
| Start a new graph | Click **New Graph** or press **N** |

The visual states match the Kruskal builder:

| Visual | Meaning |
| --- | --- |
| Blue vertex | Vertex with at least one edge |
| Gray vertex | Isolated vertex |
| Yellow vertex | Selected source |
| Orange vertex | Vertex being dragged |
| Top bar | Vertex count, edge count, isolated count |
| Bottom bar | Status message and key hints |

The builder validates that edges are not self-loops, weights are positive integers, and duplicate undirected edges are rejected. It does not support edge deletion, weight editing, undo, save/load, or more than 30 vertices.

## How the Graph Model Works (`graph.h`)

```cpp
struct Edge {
    int src;
    int dest;
    int weight;
};

struct Graph {
    int V;
    vector<Edge> edges;
};
```

The current `Graph` is an **edge list**, not an adjacency list. Every edge is stored once and treated as undirected. The operations are:

| Function | Behavior | Time |
| --- | --- | --- |
| `create_graph(number)` | Creates a graph with `V = number` | O(1) |
| `add_edge(graph, src, dest, weight)` | Appends an edge, without validation | O(1) amortized |
| `edge_exists(graph, src, dest)` | Checks both orientations for a duplicate | O(E) |
| `unuse_vertex(graph)` | Returns vertices with no incident edges | O(V × E) |
| `is_connected(graph)` | Returns `true` if there are no isolated vertices | O(V × E) |

> **Connectivity caveat:** `is_connected` is not a real connectivity check. A graph with edges `(0,1)` and `(2,3)` has no isolated vertices, so the function returns `true`, even though there are two components. Prim needs a proper visited/component check, and disconnected graphs must produce a forest or an explicit "no spanning tree" result.

For Prim, this edge list is inconvenient: the algorithm repeatedly needs the neighbors of a vertex. A typical implementation first converts the edges into an adjacency list:

```cpp
struct Neighbor {
    int to;
    int weight;
};

vector<vector<Neighbor>> adjacency(graph.V);
for (const Edge& e : graph.edges) {
    adjacency[e.src].push_back({e.dest, e.weight});
    adjacency[e.dest].push_back({e.src, e.weight});
}
```

That conversion costs **O(V + E)** and must happen before the algorithm runs.

## How Prim's Algorithm Works

### High-level idea

```text
visited = {start}
mst     = {}

while visited does not contain all vertices:
    find the minimum-weight edge (u, v)
        where u is in visited and v is not
    if no such edge exists:
        graph is disconnected; stop or start another component
    add (u, v) to mst
    add v to visited
```

At every step, the edge added is the cheapest way to grow the tree by one vertex. This greedy choice is safe because any spanning tree must connect the visited set to the rest of the graph through some edge, and choosing the cheapest crossing edge cannot make the final result worse.

### Array implementation — O(V²)

This is the simplest version and works well for dense graphs:

```cpp
vector<int> key(V, INT_MAX);    // cheapest known edge into the tree
vector<int> parent(V, -1);      // which MST vertex provides that edge
vector<bool> inMST(V, false);

key[start] = 0;

for (int count = 0; count < V; count++) {
    // Pick the unvisited vertex with the smallest key.
    int u = -1;
    for (int i = 0; i < V; i++) {
        if (!inMST[i] && (u == -1 || key[i] < key[u])) {
            u = i;
        }
    }

    if (u == -1 || key[u] == INT_MAX) {
        break;  // disconnected graph: no crossing edge
    }

    inMST[u] = true;

    for (const Neighbor& edge : adjacency[u]) {
        int v = edge.to;
        if (!inMST[v] && edge.weight < key[v]) {
            key[v] = edge.weight;
            parent[v] = u;
        }
    }
}
```

The `parent` array describes the MST: for every vertex `v != start` that was reached, `parent[v] — v` is an MST edge.

The selection loop runs `V` times and scans `V` vertices each time, so the total is **O(V²)**. With an adjacency matrix this is the standard dense-graph implementation.

### Priority-queue implementation — O(E log V)

For sparse graphs, store candidate edges in a min-heap keyed by weight:

```cpp
priority_queue<pair<int, int>,
               vector<pair<int, int>>,
               greater<pair<int, int>>> pq;

pq.push({0, start});

while (!pq.empty()) {
    auto [weight, u] = pq.top();
    pq.pop();

    if (inMST[u]) continue;  // stale entry
    inMST[u] = true;

    for (const Neighbor& edge : adjacency[u]) {
        if (!inMST[edge.to]) {
            pq.push({edge.weight, edge.to});
        }
    }
}
```

Each edge can be pushed at most twice (once from each endpoint), so the heap holds **O(E)** entries and the total time is **O(E log V)**. A Fibonacci heap can improve the theoretical bound to **O(E + V log V)**, but it is rarely used in practice.

### Example trace

Using the demo graph from `gui.cpp`:

```text
(0,1,4)  (1,2,8)  (2,3,7)  (3,4,9)  (4,5,10)  (5,0,6)
```

Start at vertex `0`:

| Step | Visited set | Cheapest crossing edge | Added |
| --- | --- | --- | --- |
| 1 | `{0}` | `0-1` (4) vs `0-5` (6) | `0-1`, weight 4 |
| 2 | `{0,1}` | `0-5` (6) vs `1-2` (8) | `0-5`, weight 6 |
| 3 | `{0,1,5}` | `1-2` (8) vs `5-4` (10) | `1-2`, weight 8 |
| 4 | `{0,1,2,5}` | `2-3` (7) vs `5-4` (10) | `2-3`, weight 7 |
| 5 | `{0,1,2,3,5}` | `3-4` (9) vs `5-4` (10) | `3-4`, weight 9 |

MST total weight: **4 + 6 + 8 + 7 + 9 = 34**.

The same MST could be produced starting from any vertex; the set of edges is the same when weights are distinct.

## Complexity Summary (Once Implemented)

| Implementation | Time | Space | Best for |
| --- | --- | --- | --- |
| Array + linear minimum | O(V²) | O(V) extra | Dense graphs |
| Binary heap + adjacency list | O(E log V) | O(V + E) | Sparse graphs |
| Edge-list scan (naive) | O(V × E) | O(V) extra | Small demos only |

The current graph model already stores `O(V + E)` data, but it must be converted to an adjacency list before the heap version can run.

## What the Prim Implementation Still Needs

### Algorithm work

- Convert `graph.edges` into an adjacency list.
- Add a start-vertex parameter (or default to vertex `0`).
- Implement the key/parent arrays and the visited set.
- Select the minimum crossing edge each iteration.
- Collect the MST edges and compute the total weight.
- Detect disconnected graphs and either produce a spanning forest or report that no spanning tree exists.
- Handle isolated vertices explicitly.

### GUI work

- Change the window title and subtitle from "Kruskal" to "Prim".
- Add a **Run Prim** button.
- Let the user choose the start vertex.
- Highlight MST edges in a dedicated color.
- Display the total weight and the number of selected edges.
- Add step-by-step or animated playback.
- Show the current frontier and candidate edges.
- Reset the visualization when the graph changes or a new graph is created.
- Remove the Kruskal-specific terminology from the copied UI.

### Project structure

- `graph.h` is duplicated between the Kruskal and Prim folders. A shared header (or a common library) would avoid divergence.
- `gui.cpp` is also duplicated. Prim-specific controls should eventually replace the copied Kruskal UI.
- The placeholder comments in `main.cpp` still describe Kruskal steps (`sort edges by weight`, `disjoint set`, `kruskal()`), confirming that the folder is a copy waiting to be adapted.

## Kruskal vs. Prim

| Aspect | Kruskal | Prim |
| --- | --- | --- |
| Strategy | Global edge sort + DSU | Grow one tree from a start vertex |
| Main data structure | Disjoint set union | Min-heap or key array |
| Time | O(E log E) | O(E log V) with heap; O(V²) with array |
| Good for | Sparse graphs | Dense graphs |
| Disconnected input | Produces a spanning forest naturally | Produces one tree per component; needs restart logic |
| Start vertex | Not applicable | Required (or defaulted) |
| Cycle handling | Explicit DSU check | Implicit: only crossing edges are considered |

Both algorithms produce an MST with the same total weight when all edge weights are distinct. With equal weights, different MSTs can be valid.

## Notes and Limitations

### Current algorithm status

- **`main.cpp` is a placeholder.** It includes `graph.h` and contains no Prim function, no key/parent arrays, and no output.
- The GUI does not execute Prim, does not show a start vertex, and does not highlight an MST.
- The GUI window and footer text still identify the application as Kruskal.
- The `--demo` and `--smoke-test` modes only exercise graph creation and edge drawing.

### Graph model caveats

- `is_connected` only checks for isolated vertices, not full connectivity.
- The graph is stored as an edge list; Prim needs an adjacency list conversion.
- `add_edge` does not validate; the GUI validates self-loops, duplicates, and weights.
- Edges are undirected but stored once.
- No edge deletion, weight editing, undo, or save/load.
- Vertex count is limited to 30; weights are positive `int` values.
- The maximum weight input length in the dialog is 6 digits.

### GUI caveats

- The UI code (`gui.cpp` and `build_gui.bat`, including layout, drawing, input, and build configuration) was **generated entirely with AI**. The graph model in `graph.h` is the author's shared graph implementation. The Prim algorithm itself is the author's pending work. For the AI disclosure and reasoning, see the [AI Assistance](../../README.md#ai-assistance) section in the main README.
- The copied UI still contains Kruskal strings and Kruskal-oriented layout choices.
- Fonts are loaded from Windows paths with a raylib default fallback.
- The window is fixed at `1180 × 760`.
- Closing the window is the main exit path; `Esc` is used for cancellation.

## Checklist

- [x] Edge and graph structures
- [x] Shared graph operations
- [x] Graph builder GUI shell
- [x] Input validation for self-loops, duplicate edges, and weights
- [ ] Adapt the GUI text and controls for Prim
- [ ] Convert the edge list to an adjacency list
- [ ] Add start-vertex selection
- [ ] Implement key/parent/visited arrays
- [ ] Implement `prim()`
- [ ] Console program in `main.cpp`
- [ ] MST edge highlighting and total weight
- [ ] Step-by-step animation
- [ ] Disconnected-graph handling
- [ ] Tests for small, dense, sparse, and disconnected graphs

## What This Teaches

- The difference between growing a tree from a vertex (Prim) and merging components globally (Kruskal).
- Why the cheapest crossing edge is always safe to add.
- How the key/parent arrays encode the growing MST.
- Why Prim is O(V²) with a linear minimum scan and O(E log V) with a heap.
- Why an edge-list graph is natural for Kruskal but needs conversion for Prim.
- How a GUI shell can be reused across algorithms, and what must change to make it algorithm-specific.
