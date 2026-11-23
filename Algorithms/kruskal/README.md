# Kruskal's Minimum Spanning Tree in C++

> Part of the [Data Structures and Algorithms in C++](../../README.md) collection.

> **Status: algorithm core not implemented yet.** The shared graph model and the interactive graph builder GUI are complete, but `main.cpp` is still a placeholder and there is no Kruskal/MST computation or visualization yet.

## Overview

A **minimum spanning tree (MST)** of a connected, weighted, undirected graph is a subset of edges that connects all vertices, contains no cycles, and has the minimum possible total edge weight. A spanning tree on `V` vertices always has exactly `V - 1` edges.

**Kruskal's algorithm** builds the MST by considering edges from the smallest weight to the largest:

1. Sort all edges by weight.
2. Start with every vertex in its own component.
3. Take the next smallest edge. If it connects two different components, add it to the MST and merge the components.
4. If it would create a cycle (both endpoints are already in the same component), skip it.
5. Stop after `V - 1` edges.

The "are these two vertices already connected?" question is answered by a **disjoint set union (DSU)**, also called **union-find**.

This folder currently contains the graph model and a raylib GUI for building weighted graphs. The actual Kruskal implementation is the next step.

## Files

| File | Purpose | Status |
| --- | --- | --- |
| [`graph.h`](graph.h) | Shared `Edge`/`Graph` model and graph operations | Implemented |
| [`gui.cpp`](gui.cpp) | raylib graph builder and visualizer shell | Implemented (AI-generated UI) |
| [`main.cpp`](main.cpp) | Placeholder for the console Kruskal implementation | Not implemented |
| [`build_gui.bat`](build_gui.bat) | Windows/MinGW build script for `gui.exe` | Implemented |

A built `gui.exe` may exist locally, but `*.exe` is ignored by `.gitignore`, so it is not part of the committed repository.

## Build and Run

### Requirements

- Windows
- MinGW-w64 `g++` with C++17 support
- [raylib](https://www.raylib.com/) 6.2

### Build

```bat
cd Algorithms\kruskal
build_gui.bat
```

The script defaults to:

- `RAYLIB_HOME=D:\toll\raylib-6.2-ucrt`
- compiler `D:\toll\mingw64\bin\g++.exe` (falls back to `g++` from `PATH` if missing)

Set `RAYLIB_HOME` before running if raylib is installed elsewhere:

```bat
set RAYLIB_HOME=C:\path\to\raylib
build_gui.bat
```

The script compiles `gui.cpp` with `-std=c++17 -O2 -Wall -Wextra -mwindows` and writes `gui.exe` in the same folder. The VS Code task **Build Graph GUI (raylib)** runs the same script.

### Run

If a `gui.exe` is already present in the folder, you can run it directly. Otherwise build it with `build_gui.bat` first.

```bat
gui.exe
```

Useful switches:

```bat
gui.exe --demo          REM open a demo graph with 6 vertices and 6 edges
gui.exe --smoke-test    REM draw 30 frames and exit (automated startup check)
```

## How to Use the GUI

### Setup screen

1. Enter a vertex count from **1 to 30** using the number keys, the `+`/`-` buttons, or the up/down arrow keys.
2. Press **Enter** or click **Create Graph**.
3. Press **Esc** to quit.

The default count is 6.

### Graph builder screen

| Action | Control |
| --- | --- |
| Select the first endpoint | Left-click a vertex |
| Choose the second endpoint | Left-click another vertex |
| Open the weight dialog | Automatically, after choosing two vertices |
| Confirm an edge | Type a positive integer and press **Enter** or click **Add Edge** |
| Cancel edge creation | **Esc**, right-click, or **Cancel** |
| Clear the current selection | **Esc**, right-click, or click empty canvas |
| Move a vertex | Press and drag it (more than 4 pixels of movement) |
| Start a new graph | Click **New Graph** or press **N** |
| Quit | Close the window (the window close button is the exit path; `Esc` is used for cancellation) |

The selected vertex turns yellow. Edges incident to it are highlighted. The pending edge follows the mouse until the weight dialog opens. Edge weights are drawn in small rounded labels offset perpendicular to the edge so that crossing labels are less likely to overlap.

### Reading the display

| Visual | Meaning |
| --- | --- |
| Blue vertex | Vertex with at least one incident edge |
| Gray vertex | Isolated vertex (no edges) |
| Yellow vertex | Currently selected source |
| Orange vertex | Vertex being dragged |
| Light blue vertex | Hovered vertex |
| Top bar | Vertex count, edge count, and number of isolated vertices |
| Bottom bar | Status message and controls hint |

The status dot at the bottom changes color: blue for information, green for success, red for errors, and yellow for selection/active states.

### Current behavior and validation

The GUI enforces these rules before calling `add_edge`:

- An edge must connect two **different** vertices (no self-loops).
- The weight must be a **positive integer**.
- The same undirected edge cannot be added twice; `edge_exists` checks both orientations.
- The maximum vertex count is 30.
- The weight input accepts up to 6 digits.

There is no way to delete an existing edge, edit a weight, undo an action, or save/load a graph yet. Creating a new graph discards the current one.

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

The graph is undirected, but each edge is stored **once** as an `Edge` record. There is no adjacency list; all graph operations scan the edge vector.

### `create_graph(int number)`

Creates a `Graph` with `V = number` and an empty edge list. **O(1)**.

### `add_edge(Graph& graph, int src, int dest, int weight)`

Builds an `Edge` and appends it to `graph.edges`. It performs no validation; the GUI is responsible for rejecting self-loops, duplicate edges, and non-positive weights. **O(1)** amortized (vector growth may occasionally reallocate).

### `edge_exists(Graph& graph, int src, int dest)`

Scans all edges and returns `true` if an edge exists in either direction (`src→dest` or `dest→src`), because the graph is undirected. **O(E)**.

### `unuse_vertex(Graph& graph)`

Returns a vector of vertices with **no incident edges**. For each vertex `i`, it scans every edge and checks whether `i` appears as `src` or `dest`. **O(V × E)**.

These are the vertices shown in gray and counted as "Isolated" in the top bar.

### `is_connected(Graph& graph)`

Returns `true` when `unuse_vertex` returns an empty list. In other words, it only checks whether **every vertex has at least one edge**.

> **Important caveat:** this is **not** a true connectivity test. A graph with edges `(0,1)` and `(2,3)` has no isolated vertices, so `is_connected` returns `true`, even though the graph has two separate components and no single spanning tree. A real connectivity check needs BFS/DFS or DSU over all vertices. This matters for Kruskal: the algorithm should handle a disconnected graph by producing a **minimum spanning forest**, or by reporting that no spanning tree exists.

### Graph operation summary

| Operation | Time | Space |
| --- | --- | --- |
| `create_graph` | O(1) | O(1) |
| `add_edge` | O(1) amortized | O(1) |
| `edge_exists` | O(E) | O(1) |
| `unuse_vertex` | O(V × E) | O(V) |
| `is_connected` | O(V × E) | O(V) |

## How the GUI Is Organized (`gui.cpp`)

The GUI is a raylib application with two screens:

```cpp
enum class Screen {
    Setup,
    Builder
};
```

`AppState` holds the current screen, the `Graph` object, vertex positions, selection/drag state, the weight dialog state, status text, and the smoke-test/demo flags.

### Layout and colors

The top of the file defines:

- Window size `1180 × 760`.
- Top bar height `88`, bottom bar height `64`, margin `24`.
- A dark color palette (`BG`, `PANEL`, `BORDER`, `TEXT`, `MUTED`, `ACCENT`, ...).
- Node colors for normal, hovered, dragged, isolated, and selected states.

### Small helpers

| Helper | Role |
| --- | --- |
| `clampf` | Keeps a float inside a range |
| `setStatus` | Updates the status message and its color |
| `measureText` / `drawText` / `drawTextCentered` | Font-aware text drawing |
| `buttonClicked` / `buttonHovered` | Mouse hit-testing |
| `drawButton` | Rounded button with hover and disabled states |
| `loadFonts` | Tries Segoe UI/Arial, falls back to raylib's default font |

### Setup flow

- `rebuildLayout(count)` places vertices evenly on a circle and shrinks the node radius when there are many vertices.
- `startGraph(count)` clamps the count to `1..30`, calls `create_graph`, resets selection/drag state, computes isolated vertices with `unuse_vertex`, rebuilds the layout, and switches to the builder screen.
- `openSetup()` returns to the setup screen and preserves the previous vertex count as the new default.

### Edge creation flow

- `clickNode(vertex)` implements the two-click state machine: first click selects a source, second click opens the weight dialog. Clicking the same vertex clears the selection.
- `confirmEdge()` validates the endpoints and weight, calls `edge_exists`, then calls `add_edge` and refreshes `unusedVertices`.
- `cancelEdgeDialog()` and `updateWeightDialog()` handle the modal dialog, including keyboard input, backspace, Enter, Esc, and right-click cancellation.

All graph mutation goes through `graph.h`; the GUI does not maintain a second copy of the graph data.

### Input and drawing

- `updateSetup()` handles digit input, backspace, arrow keys, +/- buttons, and Enter.
- `updateBuilder()` handles the New Graph button, the weight dialog, selection, dragging, hover state, and cursor changes. Dragging uses a 4-pixel threshold (`dx*dx + dy*dy > 16`) to distinguish a click from a drag.
- `drawSetup()` draws the title, setup panel, input box, blinking cursor, buttons, status, and footer.
- `drawBuilder()` draws the dot grid, top stats bar, edges and weight labels, pending edge, vertices, bottom status bar, and the modal weight dialog.
- `drawEdgeWeight()` computes the perpendicular offset for the weight label and draws a rounded "pill".
- `main()` initializes raylib, loads fonts, handles `--demo`/`--smoke-test`, runs the update/draw loop, and unloads custom fonts.

## What the Kruskal Implementation Still Needs

The algorithm belongs in [`main.cpp`](main.cpp). The intended structure is:

### 1. Sort the edges

```cpp
sort(graph.edges.begin(), graph.edges.end(),
     [](const Edge& a, const Edge& b) {
         return a.weight < b.weight;
     });
```

Sorting costs **O(E log E)**.

### 2. Implement a disjoint set union

```cpp
struct DSU {
    vector<int> parent;
    vector<int> rank_;

    explicit DSU(int n) : parent(n), rank_(n, 0) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }

    int find(int x) {
        if (parent[x] != x) parent[x] = find(parent[x]); // path compression
        return parent[x];
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;

        if (rank_[a] < rank_[b]) swap(a, b);
        parent[b] = a;
        if (rank_[a] == rank_[b]) rank_[a]++;
        return true;
    }
};
```

With path compression and union by rank, each operation is **O(α(V))** amortized, where `α` is the inverse Ackermann function and is effectively constant.

### 3. Run Kruskal

```cpp
vector<Edge> kruskal(Graph& graph) {
    vector<Edge> sorted = graph.edges;
    sort(sorted.begin(), sorted.end(),
         [](const Edge& a, const Edge& b) { return a.weight < b.weight; });

    DSU dsu(graph.V);
    vector<Edge> mst;

    for (const Edge& e : sorted) {
        if (dsu.unite(e.src, e.dest)) {
            mst.push_back(e);
            if ((int)mst.size() == graph.V - 1) break;
        }
    }

    return mst;
}
```

After the loop:

- If `mst.size() == V - 1`, the MST is complete.
- If `mst.size() < V - 1`, the graph was disconnected and the result is a **minimum spanning forest**.
- An isolated vertex cannot be connected by any edge, so a connected spanning tree is impossible unless `V == 1`.

### 4. Add the visualization

The GUI still needs a way to actually run the algorithm and display the result. Reasonable next steps:

- Add a **Run Kruskal** button.
- Highlight the MST edges in a distinct color.
- Show the total MST weight and the number of selected edges.
- Animate edge consideration step by step (smallest to largest).
- Show rejected edges briefly (cycle detected).
- Handle disconnected graphs and isolated vertices explicitly.
- Reset the visualization when the graph changes.

## Complexity Summary (Once Implemented)

| Step | Time |
| --- | --- |
| Copy edges | O(E) |
| Sort edges | O(E log E) |
| DSU initialization | O(V) |
| Find/union for every edge | O(E α(V)) |
| Total | **O(E log E)** |
| Space | O(V + E) |

Kruskal is a good fit for **sparse** graphs because its runtime is dominated by sorting the edge list. Prim's algorithm is often preferred for **dense** graphs.

## Notes and Limitations

### Current algorithm status

- **`main.cpp` is empty** (only includes `graph.h` and comments). There is no `kruskal()` function, no DSU, and no console output.
- The GUI is a **graph builder**, not a Kruskal visualizer yet. It draws edges and vertices but never runs or displays the MST.
- The graph model is an edge list, which is appropriate for Kruskal but requires conversion to an adjacency list for Prim.

### Graph model caveats

- `is_connected` only detects isolated vertices, not multiple components.
- `add_edge` accepts any data; the GUI is the only validation layer.
- `edge_exists` is O(E), and adding many edges repeatedly is O(E²) overall in the GUI.
- The graph is undirected but stores one direction; every algorithm must treat `src→dest` and `dest→src` as the same edge.
- There is no edge deletion or weight editing.
- The maximum is 30 vertices.
- Weights are positive `int` values only (up to 6 digits in the dialog).
- No save/load, undo, or export.
- The GUI keeps vertex positions only in memory; they are lost when the program exits.

### GUI caveats

- The UI code (`gui.cpp` and `build_gui.bat`, including layout, drawing, input, and build configuration) was **generated entirely with AI**. The graph model in `graph.h` is the author's shared graph implementation, and the Kruskal algorithm itself is the author's pending work. For the AI disclosure and reasoning, see the [AI Assistance](../../README.md#ai-assistance) section in the main README.
- The same `gui.cpp` is currently duplicated in the Prim folder. It has not been generalized into a shared UI module.
- Fonts are loaded from Windows paths (`C:/Windows/Fonts/...`) with a raylib default fallback.
- The window is fixed at `1180 × 760`; there is no resizing or high-DPI handling.
- Closing the window is the main exit path; `Esc` cancels selections/dialogs rather than quitting.

## Checklist

- [x] Edge and graph structures
- [x] Shared graph operations (`create_graph`, `add_edge`, `edge_exists`, `unuse_vertex`)
- [x] Interactive graph builder GUI
- [x] Input validation for self-loops, duplicate edges, and weights
- [ ] Sort edges by weight
- [ ] Disjoint set union (union-find)
- [ ] `kruskal()` implementation
- [ ] Console program in `main.cpp`
- [ ] MST edge highlighting and total weight
- [ ] Step-by-step animation
- [ ] Correct connectivity handling for disconnected graphs
- [ ] Tests for small, dense, sparse, and disconnected graphs

## What This Teaches

- The definition of a minimum spanning tree and the `V - 1` edge property.
- Why sorting edges by weight and skipping cycles produces an MST.
- How DSU detects cycles and merges components efficiently.
- Why Kruskal is `O(E log E)` and when it is preferable to Prim.
- How a graph GUI can serve as a teaching tool even before the algorithm itself is visualized.
- Why a proper connectivity check requires more than counting isolated vertices.
