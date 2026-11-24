#pragma once

// Exact copy of the stack operations from stack.cpp, used by gui.cpp.
// stack.cpp is intentionally left unchanged and does not include this header.

#include <iostream>

//cross platform
#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

using namespace std;

const int MAX=100;
int stack[MAX];
int top=-1;

bool push(int value){
    if(top>=MAX-1){
        cout<<"stack is full";
        return false;
    }
    top++;
    stack[top]=value;
    return true;

}

bool pop(){
    if (top == -1){
        cout << "stack is empty\n";
        return false;
    }

    top--;
    return true;
}

void peek(){
    if (top == -1){
        cout << "stack is empty\n";
        return;
    }
    cout << "\npeek :"<< stack[top] << "\n";
}

void display(){
    int i=0;
    cout << "\nstack : ";
    while(i<=top){
        cout << stack[i] << " ";
        i++;
    }
    cout<<"\n";
}
