# Algorithm Visualizer

## Overview

Algorithm Visualizer is a C++ application designed to provide an interactive and visual representation of various sorting, searching, and tree traversal algorithms. Leveraging the power of [SFML (Simple and Fast Multimedia Library)](https://www.sfml-dev.org/), this tool helps users understand the inner workings of algorithms through dynamic animations and intuitive UI elements.

## Features

### Sorting Algorithms
- **Bubble Sort**
- **Quick Sort**
- **Insertion Sort**
- **Selection Sort**
- **Merge Sort**
- **Heap Sort**
- **Bucket Sort**
- **Gnome Sort**
- **Tim Sort**
- **Cycle Sort**

### Searching Algorithms
- **Linear Search**
- **Binary Search**
- **Ternary Search**

### Tree Traversal Algorithms
- **Breadth-First Search (BFS)**
- **Depth-First Search (DFS)**

## Technologies Used

- **C++11**
- **[SFML](https://www.sfml-dev.org/)**: For graphics rendering and window management.
- **C++ Standard Library**: Utilized for data structures and threading.

## Installation

### Prerequisites

- **C++ Compiler**: Ensure you have a C++ compiler that supports C++11.
- **SFML Library**: Download and install SFML from the [official website](https://www.sfml-dev.org/download.php).
- **Font**: An 'arial.ttf' named font in the same folder of the project.

### Compilation

Clone the repository and navigate to the project directory:

```bash
git clone https://github.com/yourusername/algori.git
cd algori
```

Use the following command to compile the project:

```bash
g++ -o algori.exe algori.cpp -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 -lglu32 -mwindows
```

## Usage

Run the compiled executable to launch the Algorithm Visualizer:

```bash
./algori.exe
```

### Controls

- **Select Algorithm**: Choose from the dropdown menu to select the desired sorting, searching, or tree traversal algorithm.
- **Start/Pause**: Begin or pause the visualization.
- **Reset**: Reset the current visualization to its initial state.

### Visualization Types

- **Sort**: Visualize various sorting algorithms with dynamic bar animations.
- **Search**: Observe the step-by-step process of different search algorithms.
- **Tree**: Explore tree traversal techniques using graphical representations of trees.

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgements

- [SFML](https://www.sfml-dev.org/) for providing an excellent multimedia library.

© 2025 Efocor/Felipe Correa Rodríguez. All rights reserved. 
