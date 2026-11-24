
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![License](https://img.shields.io/badge/license-MIT-green)

# Data Structures and Algorithms in C++

A collection of fundamental data structures and algorithms implemented from scratch in C++ for learning and practice. It covers linked lists, stacks, queues, hash tables, trees, heaps, searching, sorting, and graph algorithms.

> **Project status:** active learning repository. Some parts are complete, while Kruskal and Prim are still in progress; their algorithm cores are not finished yet.
>
> **Contributions and teaching use are welcome.** See [Contributing, Contact, and Teaching Use](#contributing-contact-and-teaching-use).

## AI Assistance

The graphical UI in this project was generated entirely with AI. All data structure and algorithm implementations were written by me (Mahdi Arjmandi).

I used AI for the UI because this is an educational repository: the goal is to learn and teach algorithms, not to spend time on window management, layout, fonts, and mouse handling. A visual, interactive interface makes graph algorithms easier to demonstrate and understand. AI handled the GUI boilerplate so I could keep my focus on implementing the algorithms.

### AI usage policy for future contributions

- AI may be used for **UI, documentation, build scripts, and repetitive boilerplate**.
- AI must **not** be used to generate the algorithm implementations that this repository is meant to teach.
- Any contribution that includes AI-generated code must **say so in the pull request**.
- Algorithm contributions should be explainable, tested, and consistent with the educational purpose of the repository.

## Two Versions: CLI and UI

Some folders provide two completely separate versions:

- **CLI version:** the original console program, written by the author without AI. It is the reference implementation and the best place to study the actual code: functions, arrays, indices, control flow, and edge cases.
- **UI version:** a separate raylib program, generated entirely with AI. In the completed pairs (Stack and Queue), it uses an exact copy of the CLI operations to visualize them. It is the best place to build intuition: push/pop, enqueue/dequeue, animations, overflow, front/rear, and wrap-around.

The two versions are independent:

- separate source files (`*.cpp` vs `gui.cpp`);
- separate executables (`*.exe` vs `gui.exe`);
- no runtime dependency between them;
- for Stack and Queue, the UI uses a header copy (`stack.h` / `queue.h`) of the CLI functions, so behavior stays the same.

The **CLI version** exists to understand **how the data structure or algorithm is implemented in code**. The **UI version** exists to understand **what the data structure does visually**, which makes learning and explaining it easier. A good workflow is to use the UI first to build intuition, then read the CLI code to see how that behavior is implemented.

Which versions are available?

- **Stack** and **Queue:** both CLI and UI.
- **Kruskal** and **Prim:** UI graph builder; their CLI algorithm cores are still pending.
- **Other folders:** CLI only.

**No AI was used for the CLI versions.** The Stack and Queue UI versions are AI-generated copies created for visualization. The Kruskal and Prim UI is an AI-generated graph-builder shell; their algorithm cores are still pending.

## Contents

**To use any algorithm or data structure, read the `README.md` inside its folder.** This main README is only an index. Each folder README contains the build/run instructions, usage guide, code walkthrough, complexity, and limitations.

### Data Structures

- [Circular Linked List](Data_structurs/circular-linked-list/README.md) — CLI
- [Complete Binary Tree](Data_structurs/complete-binary-tree/README.md) — CLI
- [Hash Table (Open Addressing)](Data_structurs/hash-table-open-addressing/README.md) — CLI
- [Heap (Min and Max)](Data_structurs/heap/README.md) — CLI
- [Queue (Circular Array)](Data_structurs/queue/README.md) — CLI + UI
- [Stack (Array)](Data_structurs/stack/README.md) — CLI + UI

### Algorithms

- [Binary Search](Algorithms/binary-search/README.md) — CLI
- [Quick Sort](Algorithms/quick_sort/README.md) — CLI
- [Kruskal's MST](Algorithms/kruskal/README.md) — UI graph builder (CLI pending)
- [Prim's MST](Algorithms/prim/README.md) — UI graph builder (CLI pending)

## Project Structure

```text
data-structures-and-algorithms-cpp
│
├── Algorithms
│   ├── binary-search
│   ├── quick_sort
│   ├── kruskal
│   └── prim
│
├── Data_structurs
│   ├── circular-linked-list
│   ├── complete-binary-tree
│   ├── hash-table-open-addressing
│   ├── heap
│   ├── queue
│   └── stack
│
├── LICENSE
└── README.md
```

Every folder contains its own `README.md`.

## Contributing, Contact, and Teaching Use

### Found a bug? Want to contribute?

Bug reports, suggestions, and pull requests are welcome.

- **Open an issue:** [GitHub Issues](https://github.com/Mahdiarjmandi007/data-structures-and-algorithms-cpp/issues)
- **Submit a pull request:** [GitHub Pull Requests](https://github.com/Mahdiarjmandi007/data-structures-and-algorithms-cpp/pulls)
- For small fixes such as a typo, a documentation improvement, or a clear bug fix, feel free to open a pull request directly.
- For larger changes such as a new data structure, a new algorithm, or a major GUI feature, please open an issue first so we can agree on the approach.

If your contribution includes AI-generated code, please mention it in the pull request description and follow the [AI usage policy](#ai-usage-policy-for-future-contributions).

When reporting a bug, please include:

- what you expected to happen and what actually happened;
- the steps needed to reproduce the problem;
- your operating system and compiler;
- the file and line number, if you can identify them;
- a screenshot for GUI issues.

### Contact

- **Telegram:** [@daedaluus](https://t.me/daedaluus)
- **Email:** [mahdiarjmandi007@gmail.com](mailto:mahdiarjmandi007@gmail.com)
- **GitHub:** [@Mahdiarjmandi007](https://github.com/Mahdiarjmandi007)

You can use any of these to report a bug, ask a question, suggest an improvement, or share feedback about the repository.

### Using this repository for teaching

You are welcome to use, fork, and share this repository for self-study, classroom examples, workshops, and teaching data structures and algorithms.

The project is released under the **MIT License**. You may reuse it, including for teaching, as long as the copyright and license notice are preserved in copies or substantial portions of the code. A link back to this repository is appreciated but not required.

If you use it in a course or workshop, I would be happy to hear about it. This is educational code, not production software; review it before using it in critical systems.

## Note on AI-generated UI

Currently, **Kruskal**, **Prim**, **Stack**, and **Queue** have a graphical UI. The UI code in those folders was generated with AI. The other folders are console-based.

The graphical UI is currently a graph builder, not an algorithm visualizer. It allows users to create and edit graphs, but it does not execute Kruskal's or Prim's algorithm yet.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

Author: **Mahdi Arjmandi**
