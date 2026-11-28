#pragma once

// Exact copy of the circular linked list operations from circular-linked-list.cpp.
// circular-linked-list.cpp is intentionally left unchanged and does not include this header.

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


void revers(){
    if (head == NULL) {
        cout << "List is empty\n";
        return;
    }

    node* temp;
    node* prev = NULL;
    node* curr = head;
    node* last = head;

    while (curr->next != head){
        temp = curr->next;
        curr->next = prev;
        prev = curr;
        curr = temp;
    }

    curr->next = prev;
    head->next = curr;
    head = curr;
}

int get_length(){
    if (head==NULL){
        cout << "list is empty";
        return 0;
    }
    node* curr = head;
    int count=0;
    do{
        curr=curr->next;
        count++;

    }while(curr!=head);
    return count;
}
// 1 2 3 4 5
void insert_at_position(int position, int value) {

    int length = get_length();

    if (position < 0 || position > length) {
        cout << "Out of range\n";
        return;
    }

    if (position == 0) {
        insert_fotl(value);
        return;
    }

    if (position == length) {
        insert_eotl(value);
        return;
    }

    node* curr = head;
    node* prev = NULL;
    int count = 0;

    while (count < position) {
        prev = curr;
        curr = curr->next;
        count++;
    }

    node* newnode = new node;
    newnode->data = value;

    prev->next = newnode;
    newnode->next = curr;
}

void delete_at_position(int position){
    int length = get_length();

    if (position < 0 || position >= length) {
        cout << "Out of range\n";
        return;
    }

    if (position == 0) {
        Delete(head->data);
        return;
    }

    node* curr = head;
    node* prev = NULL;
    int count = 0;

    while (count < position) {
        prev = curr;
        curr = curr->next;
        count++;
    }
    if (position == length - 1 ) {
        prev->next =  head;
        delete curr;
        return;
    }
    prev->next = curr->next;
    delete curr;
        
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
