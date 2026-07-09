#include <iostream>
using namespace std;

#define TABLESIZE 10

enum State {
    FILLED,
    EMPTY,
    DELETED
};

struct node{
    int key;
    State state = EMPTY;
};

node HashTable[TABLESIZE];
int HashFunc(int key){
    return key%TABLESIZE;
}

void Insert_num(int num) {
    for (int i=0 ; i<TABLESIZE ;i++){
        int index = HashFunc(num+i)%TABLESIZE;
        if(HashTable[index].state==EMPTY){
            HashTable[index].key=num;
            HashTable[index].state=FILLED;
            return;    
        } 
    }
    cout << "TABLE is Full!!! \n";
    return;
}

void Delete_num(int num){
    for (int i=0;i<10;i++){
        int index =HashFunc(num +i)%TABLESIZE;
        
        if (HashTable[index].key==num){
            HashTable[index].state=DELETED;
            HashTable[index].key=0;
            return;
        }
    }
    cout << "The value is not in hash table or Deleted." << endl;
    return;
}
void display() {
    for (int i = 0; i < TABLESIZE; i++) {
        if (HashTable[i].state == FILLED) {
            cout << i << " -> " << HashTable[i].key << endl;
        } else {
            switch (HashTable[i].state)
            {
            case 1:
            cout << i << " -> " << "empty" << endl;
                break;
            
            case 2:
                cout << i << " -> " << "deleted" << endl;
                break;
            }
        }
    }
    return;
}

int main(){
    int choice,num;
    while(1){
        cout << "menu:\n 1.Insert:\n 2.Delet:\n 3.Display:\n 4.Exit:\n"; 
        cout << "Enter Your Choice:";
        cin >> choice;
        system("cls");
        switch(choice){
            case 1:
                cout << "Enter The Number To Insert";
                cin >> num;
                Insert_num(num);
                break;
            case 2:
                cout << "Enter The Number To Delete";
                cin >> num;
                Delete_num(num);
                break;
            case 3:
                display();
                break;
            case 4:
                return 0;
            default:
                cout << "please choice the correct number";
        }
    }
}