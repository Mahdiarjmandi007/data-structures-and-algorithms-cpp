#pragma once

// Exact copy of the queue operations from queue.cpp, used by gui.cpp.
// queue.cpp is intentionally left unchanged and does not include this header.

#include <iostream>

// Cross platform
#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

using namespace std;

const int MAX = 100;

int queue[MAX];
int front = -1;
int rear = -1;
int count = 0;

// Get the number of elements
int length() {
    return count;
}

// Check if queue is empty
bool isempty() {
    return count == 0;
}

// Check if queue is full
bool is_full() {
    return count == MAX;
}

// Add an element to the queue
bool enqueue(int value) {
    if (is_full()) {
        cout << "Queue is full\n";
        return false;
    }

    if (isempty()) {
        front = 0;
    }

    rear = (rear + 1) % MAX;
    queue[rear] = value;
    count++;

    cout << "Enqueue is successful\n";
    return true;
}

// Remove an element from the queue
bool dequeue() {
    if (isempty()) {
        cout << "Queue is empty\n";
        return false;
    }

    if (front == rear) {
        front = -1;
        rear = -1;
    }
    else {
        front = (front + 1) % MAX;
    }

    count--;

    cout << "Dequeue is successful\n";
    return true;
}

bool peek() {
    if (isempty()) {
        cout << "Queue is empty\n";
        return false;
    }

    cout << "Front element: " << queue[front] << "\n";
    return true;
}

void display() {
    if (isempty()) {
        cout << "Queue is empty\n";
        return;
    }

    cout << "Queue: ";

    int i = front;

    while (true) {
        cout << queue[i] << " ";

        if (i == rear) {
            break;
        }

        i = (i + 1) % MAX;
    }

    cout << "\n";
}
