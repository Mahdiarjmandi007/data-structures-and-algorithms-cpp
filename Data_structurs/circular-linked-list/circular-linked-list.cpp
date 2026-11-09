#include <iostream>
using namespace std;

struct node 
{
    int data;
    node* next;

};

node* head=NULL;

void insert_fotl(int value){
    node* newnode = new node;
    newnode->data = value;
    if (head==NULL){
        newnode->next=newnode;
        head=newnode;
        return;
    }
    node* x=head;
    while(x->next !=head){
        x=x->next;
    }
    newnode->next=head;
    x->next=newnode;
    head=newnode;    
}

void insert_eotl(int value){
    node* newnode = new node;
    newnode->data =value;
    

    if (head==NULL){
        newnode->next=newnode;
        head=newnode;
        return;
    }
    node* x=head;
    while(x->next !=head){
        x=x->next;
    }
    x->next=newnode;
    newnode->next=head;
}

void Delete(int value){
    if (head == NULL) {
    cout << "List is empty\n";
    return;
}
    node* curr =head;
    node* prev= NULL;
    if (head->data==value){
        
        if (head->next==head){
            delete head;
            head=NULL;
            return;
        }
        node* x =head;
        while(x->next != head){
            x=x->next;
        }
        x->next=head->next;
        head = head->next;
        delete curr;
        return;
    }
    prev=head;
    curr=head->next;
    while(curr != head){
        if (curr->data==value){
            prev->next=curr->next;
            delete curr;
            return;
        }
        prev=curr;
        curr=curr->next;
    }
}

int search(int value){
    if (head == NULL) {
        cout << "List is empty\n";
        return -1;
    }

    node* curr = head;
    int count = 0;

    do {
        if (curr->data == value) {
            return count;
        }

        curr = curr->next;
        count++;

    } while (curr != head);

    return -1;
}



void Display(){
    if (head==NULL){
        cout << "list is empty";
        return;
    }
    node* x =head;
    cout << "link list:\n";
    do{
        cout<<x->data<<"->";
        x=x->next;
    }while(x != head);
    cout << "(head)\n";
}

int main(){
    int choice,value;
    while(1){
        cout << "Menu\n 1.Insert First Of The List\n 2.Insert End Of The List\n 3.Delete value\n 4.search\n 5.Display\n 6.Exit\n";
        cout << "Enter The choice:";
        cin >> choice;
        system("cls");
        switch(choice){
            case 1:
                cout << "Insert First Of The List\n Enter The Value:";
                cin >> value;
                insert_fotl(value);
                system("cls");
                break;
            case 2:
                cout << "Insert End Of The List\n Enter The Value:";
                cin >> value;
                insert_eotl(value);
                system("cls");
                break;
            case 3:
                Display();
                cout <<"Enter The Value You Want to Delete: ";
                cin >> value;
                Delete(value);
                system("cls");
                break;
            case 4:{
                Display();

                cout << "Enter The Value You Want to search: ";
                cin >> value;
                int index = search(value); 
                if (index != -1) {
                    cout << "The index of value: " << index << endl;
                }
                else {
                    cout << "Value not found!\n";
                }

                char wait;
                cout << "Press any key to return: ";
                cin >> wait;
                break;
            }
            case 5:
                Display();
                break;
            case 6:
                return 0;
            default:
                cout << "invalid number!!\n";
                break;
        }
    }
}
