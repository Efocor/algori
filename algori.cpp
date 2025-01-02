
//..................................... @FECORO, 2025 .....................................//

/*
--------------------------------------------------------------------------------------------|
* Simulation code for sorting and searching algorithms in C++ and SFML.
*
* The sorting algorithms Bubble Sort, Quick Sort, Insertion Sort,
* Selection Sort, Merge Sort, Heap Sort, Bucket Sort, Gnome Sort, Tim Sort and Cycle Sort are included.
* Additionally, the search algorithms Linear Search, Binary Search and
* Ternary Search are included. The tree traversal algorithms BFS and DFS are also included.
*
* The simulation allows the sorting and searching process to be visualized in real time.
* The simulation can be paused, resumed and restarted at any time. In addition, the algorithm to be visualized and the type of visualization (sorting,
* searching or tree traversal) can be selected from a simple 'drop-down' menu.

* The code is structured as follows:
* 1. Global variables and constants.
* 2. Utility functions.
* 3. Sorting functions.
* 4. Search functions.
* 5. Tree traversal functions.
* 6. Initialization and reset functions.
* 7. Main function.
*
* The code is designed to be as simple and clear as possible. Regarding its operation,
* it may be that due to the handling of threads and data synchronization, the simulation does not take
* so you have to press start twice for it to start working, but they are all
* functional, perhaps in a future update the code can be improved so that it
* works correctly in that aspect.

* Requirements:
* - SFML library.
* - C++11 or higher.
* - A C++ compiler.
* Arial named font file in the same directory as the code.
*
* Author: Felipe Alexander Correa Rodríguez.
--------------------------------------------------------------------------------------------|
*/


//.....................................| Imported stack |.....................................//
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string>
#include <queue>
#include <stack>
#include <unordered_map>
#include <random>
#include <iostream>
#include <cmath>
#include <list>
#include <atomic>
#include <mutex>


//.....................................| Constants & global variabless |.....................................//
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 900;
const int BAR_WIDTH = 5;
const int NUM_BARS = WINDOW_WIDTH / BAR_WIDTH;
const int NODE_RADIUS = 20;
const int NODE_SPACING = 100;
const int TREE_START_X = 100;
const int TREE_START_Y = 100;
const int BOX_SIZE = 40;
const int BOX_SPACING = 5;
const int ARRAY_START_X = 100;
const int ARRAY_START_Y = 700;
const int MAX_ARRAY_SIZE = (WINDOW_WIDTH - ARRAY_START_X) / (BOX_SIZE + BOX_SPACING);

std::thread currentThread;
std::vector<int> array(NUM_BARS);
std::vector<sf::RectangleShape> bars(NUM_BARS);
std::vector<sf::CircleShape> nodes;
std::vector<sf::VertexArray> edges;
std::unordered_map<int, sf::Vector2f> nodePositions;
std::vector<sf::RectangleShape> boxes;
std::vector<sf::Text> boxTexts;
std::atomic<bool> isSorting(false);
std::atomic<bool> isPaused(false);
std::atomic<bool> resetRequested(false);
std::string currentAlgorithm = "bubble";
std::atomic<bool> isSearching(false);
int searchValue = 0;
std::atomic<bool> isTreeTraversal(false);
std::vector<std::vector<int>> tree;
std::vector<bool> visited;
std::string visualizationType = "sort";
bool showDropdown = false;
sf::Font font;
int targetNode = -1;
std::vector<int> traversalPath;
int currentTraversalIndex = 0;
bool isArraySorted = false;
bool isSearchArraySorted = true;
std::atomic<bool> searchCompleted(false);
std::atomic<bool> traversalCompleted(false);
std::mutex mtx;

//.....................................| Utility functions |.....................................//
void resetArray() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(50, 700);
    for (int i = 0; i < NUM_BARS; ++i) {
        array[i] = dist(gen);
        bars[i].setSize(sf::Vector2f(BAR_WIDTH, array[i]));
        bars[i].setPosition(i * BAR_WIDTH, WINDOW_HEIGHT - array[i]);
        bars[i].setFillColor(sf::Color::White);
    }
    isArraySorted = false;
}

void resetTree() {
    std::lock_guard<std::mutex> lock(mtx);
    
    nodes.clear();
    edges.clear();
    nodePositions.clear();
    visited.assign(tree.size(), false);
    traversalPath.clear();
    currentTraversalIndex = 0;
    traversalCompleted = false;
    isTreeTraversal = false;

    std::queue<std::pair<int, sf::Vector2f>> q;
    q.push({0, sf::Vector2f(WINDOW_WIDTH / 2, TREE_START_Y)});
    while (!q.empty()) {
        auto [node, pos] = q.front();
        q.pop();
        sf::CircleShape nodeShape(NODE_RADIUS);
        nodeShape.setFillColor(sf::Color::White);
        nodeShape.setPosition(pos - sf::Vector2f(NODE_RADIUS, NODE_RADIUS));
        nodePositions[node] = pos;
        nodes.push_back(nodeShape);

        int childCount = tree[node].size();
        float startX = pos.x - (childCount - 1) * NODE_SPACING / 2;
        for (int i = 0; i < childCount; ++i) {
            int child = tree[node][i];
            sf::Vector2f childPos(startX + i * NODE_SPACING, pos.y + NODE_SPACING);
            q.push({child, childPos});

            sf::VertexArray edge(sf::Lines, 2);
            edge[0].position = pos;
            edge[1].position = childPos;
            edge[0].color = sf::Color::White;
            edge[1].color = sf::Color::White;
            edges.push_back(edge);
        }
    }
}

void resetBoxes() { // -> reseting array boxes
    boxes.clear();
    boxTexts.clear();
    for (int i = 0; i < MAX_ARRAY_SIZE; ++i) {
        sf::RectangleShape box(sf::Vector2f(BOX_SIZE, BOX_SIZE));
        box.setPosition(ARRAY_START_X + i * (BOX_SIZE + BOX_SPACING), ARRAY_START_Y);
        box.setFillColor(sf::Color::White);
        boxes.push_back(box);

        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(array[i]));
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::Black);
        text.setPosition(ARRAY_START_X + i * (BOX_SIZE + BOX_SPACING) + BOX_SIZE / 4, ARRAY_START_Y + BOX_SIZE / 4);
        boxTexts.push_back(text);
    }
    searchCompleted = false;
}

void resetSearchArray() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 150);
    array.resize(MAX_ARRAY_SIZE);
    for (int i = 0; i < MAX_ARRAY_SIZE; ++i) {
        array[i] = dist(gen);
    }
    std::sort(array.begin(), array.end());
    isSearchArraySorted = true;
    resetBoxes();
}

void drawArray(sf::RenderWindow& window) {// -> drawing the array
    for (int i = 0; i < NUM_BARS; ++i) {
        window.draw(bars[i]);
    }
}

void drawTree(sf::RenderWindow& window) {
    for (const auto& edge : edges) {
        window.draw(edge);
    }
    for (int i = 0; i < nodes.size(); ++i) {
        window.draw(nodes[i]);
        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(i));
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::Black);
        text.setPosition(nodePositions[i] - sf::Vector2f(NODE_RADIUS / 2, NODE_RADIUS / 2));
        window.draw(text);
    }
}

void drawBoxes(sf::RenderWindow& window) {
    for (const auto& box : boxes) {
        window.draw(box);
    }
    for (const auto& text : boxTexts) {
        window.draw(text);
    }
}

//.....................................| Funciones de Ordenamiento |.....................................//
void bubbleSort() {
    for (int i = 0; i < NUM_BARS - 1; ++i) {
        for (int j = 0; j < NUM_BARS - i - 1; ++j) {
            if (array[j] > array[j + 1]) {
                std::swap(array[j], array[j + 1]);
                bars[j].setFillColor(sf::Color::Red);
                bars[j + 1].setFillColor(sf::Color::Red);
                bars[j].setSize(sf::Vector2f(BAR_WIDTH, array[j]));
                bars[j].setPosition(j * BAR_WIDTH, WINDOW_HEIGHT - array[j]);
                bars[j + 1].setSize(sf::Vector2f(BAR_WIDTH, array[j + 1]));
                bars[j + 1].setPosition((j + 1) * BAR_WIDTH, WINDOW_HEIGHT - array[j + 1]);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                bars[j].setFillColor(sf::Color::White);
                bars[j + 1].setFillColor(sf::Color::White);
            }
            if (isPaused) {
                while (isPaused) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            if (resetRequested) {
                resetRequested = false;
                return;
            }
        }
    }
    isArraySorted = true;
}

void insertionSort() {
    for (int i = 1; i < NUM_BARS; ++i) {
        int key = array[i];
        int j = i - 1;
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            bars[j + 1].setFillColor(sf::Color::Red);
            bars[j + 1].setSize(sf::Vector2f(BAR_WIDTH, array[j + 1]));
            bars[j + 1].setPosition((j + 1) * BAR_WIDTH, WINDOW_HEIGHT - array[j + 1]);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bars[j + 1].setFillColor(sf::Color::White);
            j--;
            if (isPaused) {
                while (isPaused) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            if (resetRequested) {
                resetRequested = false;
                return;
            }
        }
        array[j + 1] = key;
        bars[j + 1].setSize(sf::Vector2f(BAR_WIDTH, array[j + 1]));
        bars[j + 1].setPosition((j + 1) * BAR_WIDTH, WINDOW_HEIGHT - array[j + 1]);
    }
    isArraySorted = true;
}

void selectionSort() {
    for (int i = 0; i < NUM_BARS - 1; ++i) {
        int minIndex = i;
        for (int j = i + 1; j < NUM_BARS; ++j) {
            if (array[j] < array[minIndex]) {
                minIndex = j;
            }
        }
        std::swap(array[i], array[minIndex]);
        bars[i].setFillColor(sf::Color::Red);
        bars[minIndex].setFillColor(sf::Color::Red);
        bars[i].setSize(sf::Vector2f(BAR_WIDTH, array[i]));
        bars[i].setPosition(i * BAR_WIDTH, WINDOW_HEIGHT - array[i]);
        bars[minIndex].setSize(sf::Vector2f(BAR_WIDTH, array[minIndex]));
        bars[minIndex].setPosition(minIndex * BAR_WIDTH, WINDOW_HEIGHT - array[minIndex]);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bars[i].setFillColor(sf::Color::White);
        bars[minIndex].setFillColor(sf::Color::White);
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }
    isArraySorted = true;
}

void quickSort(int low, int high) {
    if (low < high) {
        int pivot = array[high];
        int i = low - 1;
        for (int j = low; j < high; ++j) {
            if (array[j] < pivot) {
                i++;
                std::swap(array[i], array[j]);
                bars[i].setFillColor(sf::Color::Red);
                bars[j].setFillColor(sf::Color::Red);
                bars[i].setSize(sf::Vector2f(BAR_WIDTH, array[i]));
                bars[i].setPosition(i * BAR_WIDTH, WINDOW_HEIGHT - array[i]);
                bars[j].setSize(sf::Vector2f(BAR_WIDTH, array[j]));
                bars[j].setPosition(j * BAR_WIDTH, WINDOW_HEIGHT - array[j]);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                bars[i].setFillColor(sf::Color::White);
                bars[j].setFillColor(sf::Color::White);
            }
            if (isPaused) {
                while (isPaused) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            if (resetRequested) {
                resetRequested = false;
                return;
            }
        }
        std::swap(array[i + 1], array[high]);
        bars[i + 1].setFillColor(sf::Color::Red);
        bars[high].setFillColor(sf::Color::Red);
        bars[i + 1].setSize(sf::Vector2f(BAR_WIDTH, array[i + 1]));
        bars[i + 1].setPosition((i + 1) * BAR_WIDTH, WINDOW_HEIGHT - array[i + 1]);
        bars[high].setSize(sf::Vector2f(BAR_WIDTH, array[high]));
        bars[high].setPosition(high * BAR_WIDTH, WINDOW_HEIGHT - array[high]);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bars[i + 1].setFillColor(sf::Color::White);
        bars[high].setFillColor(sf::Color::White);
        int pi = i + 1;
        quickSort(low, pi - 1);
        quickSort(pi + 1, high);
    }
    isArraySorted = true;
}

void mergeSort(int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(left, mid);
        mergeSort(mid + 1, right);
        std::vector<int> temp(right - left + 1);
        int i = left, j = mid + 1, k = 0;
        while (i <= mid && j <= right) {
            if (array[i] <= array[j]) {
                temp[k++] = array[i++];
            } else {
                temp[k++] = array[j++];
            }
        }
        while (i <= mid) {
            temp[k++] = array[i++];
        }
        while (j <= right) {
            temp[k++] = array[j++];
        }
        for (int i = left; i <= right; ++i) {
            array[i] = temp[i - left];
            bars[i].setFillColor(sf::Color::Red);
            bars[i].setSize(sf::Vector2f(BAR_WIDTH, array[i]));
            bars[i].setPosition(i * BAR_WIDTH, WINDOW_HEIGHT - array[i]);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bars[i].setFillColor(sf::Color::White);
            if (isPaused) {
                while (isPaused) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            if (resetRequested) {
                resetRequested = false;
                return;
            }
        }
    }
    isArraySorted = true;
}

void heapify(int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && array[left] > array[largest]) {
        largest = left;
    }

    if (right < n && array[right] > array[largest]) {
        largest = right;
    }

    if (largest != i) {
        std::swap(array[i], array[largest]);
        bars[i].setFillColor(sf::Color::Red);
        bars[largest].setFillColor(sf::Color::Red);
        bars[i].setSize(sf::Vector2f(BAR_WIDTH, array[i]));
        bars[i].setPosition(i * BAR_WIDTH, WINDOW_HEIGHT - array[i]);
        bars[largest].setSize(sf::Vector2f(BAR_WIDTH, array[largest]));
        bars[largest].setPosition(largest * BAR_WIDTH, WINDOW_HEIGHT - array[largest]);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bars[i].setFillColor(sf::Color::White);
        bars[largest].setFillColor(sf::Color::White);
        heapify(n, largest);
    }
}

void heapSort() {
    for (int i = NUM_BARS / 2 - 1; i >= 0; i--) {
        heapify(NUM_BARS, i);
    }

    for (int i = NUM_BARS - 1; i > 0; i--) {
        std::swap(array[0], array[i]);
        bars[0].setFillColor(sf::Color::Red);
        bars[i].setFillColor(sf::Color::Red);
        bars[0].setSize(sf::Vector2f(BAR_WIDTH, array[0]));
        bars[0].setPosition(0 * BAR_WIDTH, WINDOW_HEIGHT - array[0]);
        bars[i].setSize(sf::Vector2f(BAR_WIDTH, array[i]));
        bars[i].setPosition(i * BAR_WIDTH, WINDOW_HEIGHT - array[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bars[0].setFillColor(sf::Color::White);
        bars[i].setFillColor(sf::Color::White);
        heapify(i, 0);
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }
    isArraySorted = true;
}

void bucketSort() { // -> sorting algorithm, basically this implementation works like taking the array and dividing it into buckets.
    int maxVal = *std::max_element(array.begin(), array.end());
    int minVal = *std::min_element(array.begin(), array.end());
    int bucketSize = 10;
    int bucketCount = (maxVal - minVal) / bucketSize + 1;
    std::vector<std::vector<int>> buckets(bucketCount);

    for (int i = 0; i < NUM_BARS; ++i) {
        int bucketIndex = (array[i] - minVal) / bucketSize;
        if (bucketIndex >= bucketCount) bucketIndex = bucketCount - 1;
        buckets[bucketIndex].push_back(array[i]);
    }

    int index = 0;
    for (int i = 0; i < bucketCount; ++i) {
        std::sort(buckets[i].begin(), buckets[i].end());
        for (int j = 0; j < buckets[i].size(); ++j) {
            array[index] = buckets[i][j];
            bars[index].setFillColor(sf::Color::Red);
            bars[index].setSize(sf::Vector2f(BAR_WIDTH, array[index]));
            bars[index].setPosition(index * BAR_WIDTH, WINDOW_HEIGHT - array[index]);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bars[index].setFillColor(sf::Color::White);
            index++;
            if (isPaused) {
                while (isPaused) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            if (resetRequested) {
                resetRequested = false;
                return;
            }
        }
    }
    isArraySorted = true;
}

void gnomeSort() {
    int index = 0;
    while (index < NUM_BARS) {
        if (index == 0 || array[index] >= array[index - 1]) {
            index++;
        } else {
            std::swap(array[index], array[index - 1]);
            bars[index].setFillColor(sf::Color::Red);
            bars[index - 1].setFillColor(sf::Color::Red);
            bars[index].setSize(sf::Vector2f(BAR_WIDTH, array[index]));
            bars[index].setPosition(index * BAR_WIDTH, WINDOW_HEIGHT - array[index]);
            bars[index - 1].setSize(sf::Vector2f(BAR_WIDTH, array[index - 1]));
            bars[index - 1].setPosition((index - 1) * BAR_WIDTH, WINDOW_HEIGHT - array[index - 1]);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bars[index].setFillColor(sf::Color::White);
            bars[index - 1].setFillColor(sf::Color::White);
            index--;
        }
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }
    isArraySorted = true;
}

void timSort() {
    const int RUN = 32;
    for (int i = 0; i < NUM_BARS; i += RUN) {
        int left = i;
        int right = std::min(i + RUN - 1, NUM_BARS - 1);
        for (int j = left + 1; j <= right; ++j) {
            int key = array[j];
            int k = j - 1;
            while (k >= left && array[k] > key) {
                array[k + 1] = array[k];
                bars[k + 1].setFillColor(sf::Color::Red);
                bars[k + 1].setSize(sf::Vector2f(BAR_WIDTH, array[k + 1]));
                bars[k + 1].setPosition((k + 1) * BAR_WIDTH, WINDOW_HEIGHT - array[k + 1]);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                bars[k + 1].setFillColor(sf::Color::White);
                k--;
                if (isPaused) {
                    while (isPaused) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
                if (resetRequested) {
                    resetRequested = false;
                    return;
                }
            }
            array[k + 1] = key;
            bars[k + 1].setSize(sf::Vector2f(BAR_WIDTH, array[k + 1]));
            bars[k + 1].setPosition((k + 1) * BAR_WIDTH, WINDOW_HEIGHT - array[k + 1]);
        }
    }

    for (int size = RUN; size < NUM_BARS; size = 2 * size) {
        for (int left = 0; left < NUM_BARS; left += 2 * size) {
            int mid = left + size - 1;
            int right = std::min(left + 2 * size - 1, NUM_BARS - 1);

            int n1 = mid - left + 1;
            int n2 = right - mid;

            std::vector<int> L(n1), R(n2);

            for (int i = 0; i < n1; ++i) {
                L[i] = array[left + i];
            }
            for (int i = 0; i < n2; ++i) {
                R[i] = array[mid + 1 + i];
            }

            int i = 0, j = 0, k = left;
            while (i < n1 && j < n2) {
                if (L[i] <= R[j]) {
                    array[k] = L[i];
                    bars[k].setFillColor(sf::Color::Red);
                    bars[k].setSize(sf::Vector2f(BAR_WIDTH, array[k]));
                    bars[k].setPosition(k * BAR_WIDTH, WINDOW_HEIGHT - array[k]);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    bars[k].setFillColor(sf::Color::White);
                    i++;
                } else {
                    array[k] = R[j];
                    bars[k].setFillColor(sf::Color::Red);
                    bars[k].setSize(sf::Vector2f(BAR_WIDTH, array[k]));
                    bars[k].setPosition(k * BAR_WIDTH, WINDOW_HEIGHT - array[k]);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    bars[k].setFillColor(sf::Color::White);
                    j++;
                }
                k++;
                if (isPaused) {
                    while (isPaused) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
                if (resetRequested) {
                    resetRequested = false;
                    return;
                }
            }

            while (i < n1) {
                array[k] = L[i];
                bars[k].setFillColor(sf::Color::Red);
                bars[k].setSize(sf::Vector2f(BAR_WIDTH, array[k]));
                bars[k].setPosition(k * BAR_WIDTH, WINDOW_HEIGHT - array[k]);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                bars[k].setFillColor(sf::Color::White);
                i++;
                k++;
                if (isPaused) {
                    while (isPaused) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
                if (resetRequested) {
                    resetRequested = false;
                    return;
                }
            }

            while (j < n2) {
                array[k] = R[j];
                bars[k].setFillColor(sf::Color::Red);
                bars[k].setSize(sf::Vector2f(BAR_WIDTH, array[k]));
                bars[k].setPosition(k * BAR_WIDTH, WINDOW_HEIGHT - array[k]);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                bars[k].setFillColor(sf::Color::White);
                j++;
                k++;
                if (isPaused) {
                    while (isPaused) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
                if (resetRequested) {
                    resetRequested = false;
                    return;
                }
            }
        }
    }
    isArraySorted = true;
}

void insertionSort(int left, int right) {
    for (int i = left + 1; i <= right; ++i) {
        int key = array[i];
        int j = i - 1;
        while (j >= left && array[j] > key) {
            array[j + 1] = array[j];
            bars[j + 1].setFillColor(sf::Color::Red);
            bars[j + 1].setSize(sf::Vector2f(BAR_WIDTH, array[j + 1]));
            bars[j + 1].setPosition((j + 1) * BAR_WIDTH, WINDOW_HEIGHT - array[j + 1]);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bars[j + 1].setFillColor(sf::Color::White);
            j--;
            if (isPaused) {
                while (isPaused) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            if (resetRequested) {
                resetRequested = false;
                return;
            }
        }
        array[j + 1] = key;
        bars[j + 1].setSize(sf::Vector2f(BAR_WIDTH, array[j + 1]));
        bars[j + 1].setPosition((j + 1) * BAR_WIDTH, WINDOW_HEIGHT - array[j + 1]);
    }
}

void mergeSort(int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; ++i) {
        L[i] = array[left + i];
    }
    for (int i = 0; i < n2; ++i) {
        R[i] = array[mid + 1 + i];
    }

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            array[k] = L[i];
            bars[k].setFillColor(sf::Color::Red);
            bars[k].setSize(sf::Vector2f(BAR_WIDTH, array[k]));
            bars[k].setPosition(k * BAR_WIDTH, WINDOW_HEIGHT - array[k]);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bars[k].setFillColor(sf::Color::White);
            i++;
        } else {
            array[k] = R[j];
            bars[k].setFillColor(sf::Color::Red);
            bars[k].setSize(sf::Vector2f(BAR_WIDTH, array[k]));
            bars[k].setPosition(k * BAR_WIDTH, WINDOW_HEIGHT - array[k]);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bars[k].setFillColor(sf::Color::White);
            j++;
        }
        k++;
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }

    while (i < n1) {
        array[k] = L[i];
        bars[k].setFillColor(sf::Color::Red);
        bars[k].setSize(sf::Vector2f(BAR_WIDTH, array[k]));
        bars[k].setPosition(k * BAR_WIDTH, WINDOW_HEIGHT - array[k]);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bars[k].setFillColor(sf::Color::White);
        i++;
        k++;
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }

    while (j < n2) {
        array[k] = R[j];
        bars[k].setFillColor(sf::Color::Red);
        bars[k].setSize(sf::Vector2f(BAR_WIDTH, array[k]));
        bars[k].setPosition(k * BAR_WIDTH, WINDOW_HEIGHT - array[k]);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bars[k].setFillColor(sf::Color::White);
        j++;
        k++;
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }
}

void cycleSort() {
    for (int cycleStart = 0; cycleStart < NUM_BARS - 1; ++cycleStart) {
        int item = array[cycleStart];
        int pos = cycleStart;

        for (int i = cycleStart + 1; i < NUM_BARS; ++i) {
            if (array[i] < item) {
                pos++;
            }
        }

        if (pos == cycleStart) {
            continue;
        }

        while (item == array[pos]) {
            pos++;
        }

        if (pos != cycleStart) {
            std::swap(item, array[pos]);
            bars[pos].setFillColor(sf::Color::Red);
            bars[pos].setSize(sf::Vector2f(BAR_WIDTH, array[pos]));
            bars[pos].setPosition(pos * BAR_WIDTH, WINDOW_HEIGHT - array[pos]);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bars[pos].setFillColor(sf::Color::White);
        }

        while (pos != cycleStart) {
            pos = cycleStart;

            for (int i = cycleStart + 1; i < NUM_BARS; ++i) {
                if (array[i] < item) {
                    pos++;
                }
            }

            while (item == array[pos]) {
                pos++;
            }

            if (item != array[pos]) {
                std::swap(item, array[pos]);
                bars[pos].setFillColor(sf::Color::Red);
                bars[pos].setSize(sf::Vector2f(BAR_WIDTH, array[pos]));
                bars[pos].setPosition(pos * BAR_WIDTH, WINDOW_HEIGHT - array[pos]);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                bars[pos].setFillColor(sf::Color::White);
            }

            if (isPaused) {
                while (isPaused) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            if (resetRequested) {
                resetRequested = false;
                return;
            }
        }
    }
    isArraySorted = true;
}

//.....................................| Search functions |.....................................//
void linearSearch() {
    for (int i = 0; i < MAX_ARRAY_SIZE; ++i) {
        if (array[i] == searchValue) {
            boxes[i].setFillColor(sf::Color::Green);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            searchCompleted = true;
            return;
        }
        boxes[i].setFillColor(sf::Color::Red);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }
    searchCompleted = true;
}

void binarySearch() {
    int left = 0, right = MAX_ARRAY_SIZE - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        boxes[mid].setFillColor(sf::Color::Yellow);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (array[mid] == searchValue) {
            boxes[mid].setFillColor(sf::Color::Green);
            searchCompleted = true;
            return;
        }
        if (array[mid] < searchValue) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }
    searchCompleted = true;
}

void ternarySearch() {
    int left = 0, right = MAX_ARRAY_SIZE - 1;
    while (left <= right) {
        int mid1 = left + (right - left) / 3;
        int mid2 = right - (right - left) / 3;
        boxes[mid1].setFillColor(sf::Color::Yellow);
        boxes[mid2].setFillColor(sf::Color::Yellow);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (array[mid1] == searchValue) {
            boxes[mid1].setFillColor(sf::Color::Green);
            searchCompleted = true;
            return;
        }
        if (array[mid2] == searchValue) {
            boxes[mid2].setFillColor(sf::Color::Green);
            searchCompleted = true;
            return;
        }
        if (searchValue < array[mid1]) {
            right = mid1 - 1;
        } else if (searchValue > array[mid2]) {
            left = mid2 + 1;
        } else {
            left = mid1 + 1;
            right = mid2 - 1;
        }
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }
    searchCompleted = true;
}

//.....................................| Tree traversal functions |.....................................//
void bfs(int start) {
    std::lock_guard<std::mutex> guard(mtx);

    std::queue<int> q;
    q.push(start);
    visited[start] = true;
    traversalPath.push_back(start);
    while (!q.empty()) {
        int node = q.front();
        q.pop();
        nodes[node].setFillColor(sf::Color::Green);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (node == targetNode) {
            traversalCompleted = true;
            return;
        }
        for (int neighbor : tree[node]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
                traversalPath.push_back(neighbor);
            }
        }
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }
    traversalCompleted = true;
    isTreeTraversal = false;
}

void dfs(int start) {
    std::lock_guard<std::mutex> guard(mtx);

    std::stack<int> s;
    s.push(start);
    visited[start] = true;
    traversalPath.push_back(start);
    while (!s.empty()) {
        int node = s.top();
        s.pop();
        nodes[node].setFillColor(sf::Color::Blue);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (node == targetNode) {
            traversalCompleted = true;
            return;
        }
        for (int neighbor : tree[node]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                s.push(neighbor);
                traversalPath.push_back(neighbor);
            }
        }
        if (isPaused) {
            while (isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (resetRequested) {
            resetRequested = false;
            return;
        }
    }
    traversalCompleted = true;
    isTreeTraversal = false;
}

//.....................................| Init. funcs |.....................................//
void startSorting() {
    
    if (currentThread.joinable()) {
        currentThread.join();
    }
    
    if (currentAlgorithm == "bubble") {
        bubbleSort();
    } else if (currentAlgorithm == "insertion") {
        insertionSort();
    } else if (currentAlgorithm == "selection") {
        selectionSort();
    } else if (currentAlgorithm == "quick") {
        quickSort(0, NUM_BARS - 1);
    } else if (currentAlgorithm == "merge") {
        mergeSort(0, NUM_BARS - 1);
    } else if (currentAlgorithm == "heap") {
        heapSort();
    } else if (currentAlgorithm == "bucket") {
        bucketSort();
    } else if (currentAlgorithm == "gnome") {
        gnomeSort();
    } else if (currentAlgorithm == "tim") {
        timSort();
    } else if (currentAlgorithm == "cycle") {
        cycleSort();
    }
    isSorting = false;
}

void startSearching() {

    if (currentThread.joinable()) {
        currentThread.join();
    }

    if (currentAlgorithm == "linear") {
        linearSearch();
    } else if (currentAlgorithm == "binary") {
        binarySearch();
    } else if (currentAlgorithm == "ternary") {
        ternarySearch();
    }
    isSearching = false;
}

void startTreeTraversal() {

    if (currentThread.joinable()) {
        currentThread.join();
    }

    if (currentAlgorithm == "bfs") {
        bfs(0);
    } else if (currentAlgorithm == "dfs") {
        dfs(0);
    }
    isTreeTraversal = false;
}

void resetAll() {
    
    resetRequested = true;

    if (currentThread.joinable()) {
        currentThread.join();
    }

    isSorting = false;
    isSearching = false;
    isTreeTraversal = false;
    isPaused = false;
    searchCompleted = false;
    traversalCompleted = false;
    if (visualizationType == "sort") {
        resetArray();
    } else if (visualizationType == "tree") {
        resetTree();
    } else if (visualizationType == "search") {
        resetSearchArray();
    }
}

//.....................................| Main function |.....................................//
int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), " ALGORI @FECORO | Algorithm Visualizer");
    if (!font.loadFromFile("arial.ttf")) {
        return -1;
    }

    sf::Text algorithmText;
    algorithmText.setFont(font);
    algorithmText.setCharacterSize(20);
    algorithmText.setFillColor(sf::Color::White);
    algorithmText.setPosition(10, 10);

    sf::RectangleShape startButton(sf::Vector2f(100, 50));
    startButton.setPosition(10, 50);
    startButton.setFillColor(sf::Color(144, 238, 144)); // ... pastel green ...

    sf::Text startText;
    startText.setFont(font);
    startText.setString("Start");
    startText.setCharacterSize(20);
    startText.setFillColor(sf::Color::Black);
    startText.setPosition(20, 60);

    sf::RectangleShape pauseButton(sf::Vector2f(100, 50));
    pauseButton.setPosition(120, 50);
    pauseButton.setFillColor(sf::Color(255, 182, 193)); // ... pastel pink ...

    sf::Text pauseText;
    pauseText.setFont(font);
    pauseText.setString("Pause");
    pauseText.setCharacterSize(20);
    pauseText.setFillColor(sf::Color::Black);
    pauseText.setPosition(130, 60);

    sf::RectangleShape resetButton(sf::Vector2f(100, 50));
    resetButton.setPosition(230, 50);
    resetButton.setFillColor(sf::Color(255, 160, 122)); // ... pastel orange ...

    sf::Text resetText;
    resetText.setFont(font);
    resetText.setString("Reset");
    resetText.setCharacterSize(20);
    resetText.setFillColor(sf::Color::Black);
    resetText.setPosition(240, 60);

    sf::RectangleShape sortButton(sf::Vector2f(150, 50));
    sortButton.setPosition(10, 110);
    sortButton.setFillColor(sf::Color(173, 216, 230)); // ... pastel blue ...

    sf::Text sortText;
    sortText.setFont(font);
    sortText.setString("Sort");
    sortText.setCharacterSize(20);
    sortText.setFillColor(sf::Color::Black);
    sortText.setPosition(20, 120);

    sf::RectangleShape searchButton(sf::Vector2f(150, 50));
    searchButton.setPosition(10, 170);
    searchButton.setFillColor(sf::Color(221, 160, 221)); // ... pastel purple ...

    sf::Text searchText;
    searchText.setFont(font);
    searchText.setString("Search");
    searchText.setCharacterSize(20);
    searchText.setFillColor(sf::Color::Black);
    searchText.setPosition(20, 180);

    sf::RectangleShape treeButton(sf::Vector2f(150, 50));
    treeButton.setPosition(10, 230);
    treeButton.setFillColor(sf::Color(255, 218, 185)); // ... pastel peach ...

    sf::Text treeText;
    treeText.setFont(font);
    treeText.setString("Tree");
    treeText.setCharacterSize(20);
    treeText.setFillColor(sf::Color::Black);
    treeText.setPosition(20, 240);

    sf::RectangleShape bubbleSortButton(sf::Vector2f(150, 50));
    bubbleSortButton.setPosition(10, 290);
    bubbleSortButton.setFillColor(sf::Color(173, 216, 230)); // ... pastel blue ...

    sf::Text bubbleSortText;
    bubbleSortText.setFont(font);
    bubbleSortText.setString("Bubble Sort");
    bubbleSortText.setCharacterSize(20);
    bubbleSortText.setFillColor(sf::Color::Black);
    bubbleSortText.setPosition(20, 300);

    sf::RectangleShape quickSortButton(sf::Vector2f(150, 50));
    quickSortButton.setPosition(10, 350);
    quickSortButton.setFillColor(sf::Color(221, 160, 221)); // ... pastel purple ...

    sf::Text quickSortText;
    quickSortText.setFont(font);
    quickSortText.setString("Quick Sort");
    quickSortText.setCharacterSize(20);
    quickSortText.setFillColor(sf::Color::Black);
    quickSortText.setPosition(20, 360);

    sf::RectangleShape insertionSortButton(sf::Vector2f(150, 50));
    insertionSortButton.setPosition(10, 410);
    insertionSortButton.setFillColor(sf::Color(255, 182, 193)); // ... pastel pink ...

    sf::Text insertionSortText;
    insertionSortText.setFont(font);
    insertionSortText.setString("Insertion Sort");
    insertionSortText.setCharacterSize(20);
    insertionSortText.setFillColor(sf::Color::Black);
    insertionSortText.setPosition(20, 420);

    sf::RectangleShape selectionSortButton(sf::Vector2f(150, 50));
    selectionSortButton.setPosition(10, 470);
    selectionSortButton.setFillColor(sf::Color(144, 238, 144)); // ... pastel green ...

    sf::Text selectionSortText;
    selectionSortText.setFont(font);
    selectionSortText.setString("Selection Sort");
    selectionSortText.setCharacterSize(20);
    selectionSortText.setFillColor(sf::Color::Black);
    selectionSortText.setPosition(20, 480);

    sf::RectangleShape mergeSortButton(sf::Vector2f(150, 50));
    mergeSortButton.setPosition(10, 530);
    mergeSortButton.setFillColor(sf::Color(255, 160, 122)); // ... pastel orange ...

    sf::Text mergeSortText;
    mergeSortText.setFont(font);
    mergeSortText.setString("Merge Sort");
    mergeSortText.setCharacterSize(20);
    mergeSortText.setFillColor(sf::Color::Black);
    mergeSortText.setPosition(20, 540);

    sf::RectangleShape heapSortButton(sf::Vector2f(150, 50));
    heapSortButton.setPosition(10, 590);
    heapSortButton.setFillColor(sf::Color(173, 216, 230)); // ... pastel blue ...

    sf::Text heapSortText;
    heapSortText.setFont(font);
    heapSortText.setString("Heap Sort");
    heapSortText.setCharacterSize(20);
    heapSortText.setFillColor(sf::Color::Black);
    heapSortText.setPosition(20, 600);

    sf::RectangleShape bucketSortButton(sf::Vector2f(150, 50));
    bucketSortButton.setPosition(10, 650);
    bucketSortButton.setFillColor(sf::Color(221, 160, 221)); // ... pastel purple ...

    sf::Text bucketSortText;
    bucketSortText.setFont(font);
    bucketSortText.setString("Bucket Sort");
    bucketSortText.setCharacterSize(20);
    bucketSortText.setFillColor(sf::Color::Black);
    bucketSortText.setPosition(20, 660);

    sf::RectangleShape gnomeSortButton(sf::Vector2f(150, 50));
    gnomeSortButton.setPosition(10, 710);
    gnomeSortButton.setFillColor(sf::Color(255, 182, 193)); // ... pastel pink ...

    sf::Text gnomeSortText;
    gnomeSortText.setFont(font);
    gnomeSortText.setString("Gnome Sort");
    gnomeSortText.setCharacterSize(20);
    gnomeSortText.setFillColor(sf::Color::Black);
    gnomeSortText.setPosition(20, 720);

    sf::RectangleShape timSortButton(sf::Vector2f(150, 50));
    timSortButton.setPosition(10, 770);
    timSortButton.setFillColor(sf::Color(144, 238, 144)); // ... pastel green ...

    sf::Text timSortText;
    timSortText.setFont(font);
    timSortText.setString("Tim Sort");
    timSortText.setCharacterSize(20);
    timSortText.setFillColor(sf::Color::Black);
    timSortText.setPosition(20, 780);

    sf::RectangleShape cycleSortButton(sf::Vector2f(150, 50));
    cycleSortButton.setPosition(10, 830);
    cycleSortButton.setFillColor(sf::Color(255, 160, 122)); // ... pastel orange ...

    sf::Text cycleSortText;
    cycleSortText.setFont(font);
    cycleSortText.setString("Cycle Sort");
    cycleSortText.setCharacterSize(20);
    cycleSortText.setFillColor(sf::Color::Black);
    cycleSortText.setPosition(20, 840);

    sf::RectangleShape linearSearchButton(sf::Vector2f(150, 50));
    linearSearchButton.setPosition(10, 290);
    linearSearchButton.setFillColor(sf::Color(255, 160, 122)); // ... pastel orange ...

    sf::Text linearSearchText;
    linearSearchText.setFont(font);
    linearSearchText.setString("Linear Search");
    linearSearchText.setCharacterSize(20);
    linearSearchText.setFillColor(sf::Color::Black);
    linearSearchText.setPosition(20, 300);

    sf::RectangleShape binarySearchButton(sf::Vector2f(150, 50));
    binarySearchButton.setPosition(10, 350);
    binarySearchButton.setFillColor(sf::Color(173, 216, 230)); // ... pastel blue ...

    sf::Text binarySearchText;
    binarySearchText.setFont(font);
    binarySearchText.setString("Binary Search");
    binarySearchText.setCharacterSize(20);
    binarySearchText.setFillColor(sf::Color::Black);
    binarySearchText.setPosition(20, 360);

    sf::RectangleShape ternarySearchButton(sf::Vector2f(150, 50));
    ternarySearchButton.setPosition(10, 410);
    ternarySearchButton.setFillColor(sf::Color(221, 160, 221)); // ... pastel purple ...

    sf::Text ternarySearchText;
    ternarySearchText.setFont(font);
    ternarySearchText.setString("Ternary Search");
    ternarySearchText.setCharacterSize(20);
    ternarySearchText.setFillColor(sf::Color::Black);
    ternarySearchText.setPosition(20, 420);

    sf::RectangleShape bfsButton(sf::Vector2f(150, 50));
    bfsButton.setPosition(10, 290);
    bfsButton.setFillColor(sf::Color(144, 238, 144)); // ... pastel green ...

    sf::Text bfsText;
    bfsText.setFont(font);
    bfsText.setString("BFS");
    bfsText.setCharacterSize(20);
    bfsText.setFillColor(sf::Color::Black);
    bfsText.setPosition(20, 300);

    sf::RectangleShape dfsButton(sf::Vector2f(150, 50));
    dfsButton.setPosition(10, 350);
    dfsButton.setFillColor(sf::Color(255, 160, 122)); // ... pastel orange ...

    sf::Text dfsText;
    dfsText.setFont(font);
    dfsText.setString("DFS");
    dfsText.setCharacterSize(20);
    dfsText.setFillColor(sf::Color::Black);
    dfsText.setPosition(20, 360);

    sf::Text searchValueText;
    searchValueText.setFont(font);
    searchValueText.setCharacterSize(20);
    searchValueText.setFillColor(sf::Color::White);
    searchValueText.setPosition(10, 790);

    sf::Text targetNodeText;
    targetNodeText.setFont(font);
    targetNodeText.setCharacterSize(20);
    targetNodeText.setFillColor(sf::Color::White);
    targetNodeText.setPosition(10, 820);

    resetArray();
    resetSearchArray();
    tree = {
        {1, 2}, {3, 4}, {5, 6}, {}, {}, {}, {}
    };
    resetTree();

//.....................................| Main loop |.....................................//
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (startButton.getGlobalBounds().contains(mousePos) && !isSorting && !isSearching && !isTreeTraversal) {
                    if (visualizationType == "sort") {
                        isSorting = true;
                        std::thread sortingThread(startSorting);
                        sortingThread.detach();
                    } else if (visualizationType == "search") {
                        isSearching = true;
                        searchValue = array[rand() % MAX_ARRAY_SIZE];
                        searchCompleted = false;
                        std::thread searchThread(startSearching);
                        searchThread.detach();
                    } else if (visualizationType == "tree") {
                        resetTree();
                        isTreeTraversal = true;
                        targetNode = rand() % tree.size();
                        traversalCompleted = false;
                        std::thread traversalThread(startTreeTraversal);
                        traversalThread.detach();
                    }
                }
                if (pauseButton.getGlobalBounds().contains(mousePos) && (isSorting || isSearching || isTreeTraversal)) {
                    isPaused = !isPaused;
                }
                if (resetButton.getGlobalBounds().contains(mousePos)) {
                    resetAll();
                }
                if (sortButton.getGlobalBounds().contains(mousePos)) {
                    visualizationType = "sort";
                    currentAlgorithm = "bubble";
                    showDropdown = true;
                }
                if (searchButton.getGlobalBounds().contains(mousePos)) {
                    visualizationType = "search";
                    currentAlgorithm = "linear";
                    showDropdown = true;
                }
                if (treeButton.getGlobalBounds().contains(mousePos)) {
                    visualizationType = "tree";
                    currentAlgorithm = "bfs";
                    showDropdown = true;
                }
                if (bubbleSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "bubble";
                    showDropdown = false;
                }
                if (quickSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "quick";
                    showDropdown = false;
                }
                if (insertionSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "insertion";
                    showDropdown = false;
                }
                if (selectionSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "selection";
                    showDropdown = false;
                }
                if (mergeSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "merge";
                    showDropdown = false;
                }
                if (heapSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "heap";
                    showDropdown = false;
                }
                if (bucketSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "bucket";
                    showDropdown = false;
                }
                if (gnomeSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "gnome";
                    showDropdown = false;
                }
                if (timSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "tim";
                    showDropdown = false;
                }
                if (cycleSortButton.getGlobalBounds().contains(mousePos) && visualizationType == "sort") {
                    currentAlgorithm = "cycle";
                    showDropdown = false;
                }
                if (linearSearchButton.getGlobalBounds().contains(mousePos) && visualizationType == "search") {
                    currentAlgorithm = "linear";
                    showDropdown = false;
                }
                if (binarySearchButton.getGlobalBounds().contains(mousePos) && visualizationType == "search") {
                    currentAlgorithm = "binary";
                    showDropdown = false;
                }
                if (ternarySearchButton.getGlobalBounds().contains(mousePos) && visualizationType == "search") {
                    currentAlgorithm = "ternary";
                    showDropdown = false;
                }
                if (bfsButton.getGlobalBounds().contains(mousePos) && visualizationType == "tree") {
                    currentAlgorithm = "bfs";
                    showDropdown = false;
                }
                if (dfsButton.getGlobalBounds().contains(mousePos) && visualizationType == "tree") {
                    currentAlgorithm = "dfs";
                    showDropdown = false;
                }
            }
        }

        algorithmText.setString("Algorithm: " + currentAlgorithm);
        searchValueText.setString("Number searched: " + std::to_string(searchValue));
        targetNodeText.setString("Target node: " + std::to_string(targetNode));

        window.clear(sf::Color(30, 30, 30)); // ... dark background ...

        if (visualizationType == "sort") {
            drawArray(window);
        } else if (visualizationType == "tree") {
            drawTree(window);
        } else if (visualizationType == "search") {
            drawBoxes(window);
            window.draw(searchValueText);
        }

        window.draw(startButton); // -> drawing stuff on screen
        window.draw(startText);
        window.draw(pauseButton);
        window.draw(pauseText);
        window.draw(resetButton);
        window.draw(resetText);
        window.draw(sortButton);
        window.draw(sortText);
        window.draw(searchButton);
        window.draw(searchText);
        window.draw(treeButton);
        window.draw(treeText);

        if (showDropdown) {
            if (visualizationType == "sort") {
                window.draw(bubbleSortButton);
                window.draw(bubbleSortText);
                window.draw(quickSortButton);
                window.draw(quickSortText);
                window.draw(insertionSortButton);
                window.draw(insertionSortText);
                window.draw(selectionSortButton);
                window.draw(selectionSortText);
                window.draw(mergeSortButton);
                window.draw(mergeSortText);
                window.draw(heapSortButton);
                window.draw(heapSortText);
                window.draw(bucketSortButton);
                window.draw(bucketSortText);
                window.draw(gnomeSortButton);
                window.draw(gnomeSortText);
                window.draw(timSortButton);
                window.draw(timSortText);
                window.draw(cycleSortButton);
                window.draw(cycleSortText);
            } else if (visualizationType == "search") {
                window.draw(linearSearchButton);
                window.draw(linearSearchText);
                window.draw(binarySearchButton);
                window.draw(binarySearchText);
                window.draw(ternarySearchButton);
                window.draw(ternarySearchText);
            } else if (visualizationType == "tree") {
                window.draw(bfsButton);
                window.draw(bfsText);
                window.draw(dfsButton);
                window.draw(dfsText);
            }
        }

        window.draw(algorithmText);
        if (visualizationType == "search") {
            window.draw(searchValueText);
        }
        if (visualizationType == "tree") {
            window.draw(targetNodeText);
        }

        window.display();
    }

    return 0;
}

//.....................................| Code End |.....................................//
//@Feri, 2025. All rights reserved.

//.....................................| Generic Compile command |.....................................//
// g++ -o main29.exe algori.cpp -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 -lglu32 -mwindows
