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

int main(){
    bool init;
    char wait;
    int choice,value;
    while (1){
        cout<<"Menu\n"
            <<"1.enqueue\n"
            <<"2.dequeue\n"
            <<"3.peek\n"
            <<"4.exit\n";
        cout<<"Enter The choice :";
        cin >> choice;
        switch(choice){
            case 1:
                cout<<"<<enqueue>>\n";
                cout<<"Enter the value :";
                cin >> value;
                init = enqueue(value);
                system("cls");
                if (init == true){
                    cout<<"\nenqueue is succesfully\n";
                }
                
                display();

                cout << "Press any key to return: \n";
                cin >> wait;
                system("cls");
                break;
            case 2:
                cout<<"<<dequeue>>\n";
                init=dequeue();
                system("cls");
                if (init==true){
                    cout<<"\ndequeue is succesfully\n";  
                }
                
                display();

                cout << "Press any key to return: \n";
                cin >> wait;
                system("cls");
                break;
                
            case 3:
                cout<<"<<peek>>\n";
                peek();

                cout << "Press any key to return: \n";
                cin >> wait;
                system("cls");
                break;
            case 4:
                return 0;
            default:
              cout<<"invalid number!!";
              break; 
        }        
    }
}

