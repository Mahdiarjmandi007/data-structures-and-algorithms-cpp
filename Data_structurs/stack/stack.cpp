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

int main(){
    bool init;
    char wait;
    int choice,value;
    while(1){
        cout<<"Menu\n"
            <<"1.push\n"
            <<"2.pop\n"
            <<"3.peek\n"
            <<"4.exit\n";
        cout<<"Enter The choice :";
        cin >> choice;
        switch(choice){
            case 1:
                cout<<"<<push>>\n";
                cout<<"Enter the value :";
                cin >> value;
                init = push(value);
                system("cls");
                if (init == true){
                    cout<<"\npush is succesfully\n";
                }
                
                display();

                cout << "Press any key to return: \n";
                cin >> wait;
                system("cls");
                break;

            case 2:
                cout<<"<<pop>>\n";
                init=pop();
                system("cls");
                if (init==true){
                    cout<<"\npop is succesfully\n";  
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
