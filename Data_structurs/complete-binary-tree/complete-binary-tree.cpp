#include <iostream>
#include <queue>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace std;

struct node
{
    int data;
    node* right;
    node* left;
};

node* head=NULL;
std::queue<node*> q;

void insert_bigroot(int value){
    node* newnode=new node;
    newnode->data=value;
    if (head==NULL){
        newnode->right=NULL;
        newnode->left=NULL;
        head=newnode;
        q.push(newnode);
        return;
    }
}

void insert_child(int value){
    while(!q.empty()){
        
        node* x=q.front();

        if (x->left==NULL){
            node* newnode=new node;
            x->left=newnode;
            newnode->data=value;
            newnode->right=NULL;
            newnode->left=NULL;
            q.push(newnode);
            break;
        }
        else if(x->right==NULL){
            node* newnode=new node;
            x->right=newnode;
            newnode->data=value;
            newnode->right=NULL;
            newnode->left=NULL;
            q.push(newnode);
            break;
        }
        else{
            q.pop();
        }
    }
}

int height(node* root) {
    if (!root) return 0;
    return 1 + max(height(root->left), height(root->right));
}

void print_tree(node* root) {
    if (!root) return;

    int h = height(root);
    int max_width = pow(2, h) * 2; 

    vector<node*> curr_level;
    curr_level.push_back(root);

    for (int level = 0; level < h; level++) {
        int spaces = max_width / (pow(2, level + 1));

        for (int i = 0; i < curr_level.size(); i++) {
            if (i == 0) cout << string(spaces, ' ');
            else cout << string(spaces * 2, ' ');

            if (curr_level[i]) cout << curr_level[i]->data;
            else cout << " ";
        }
        cout << "\n";

        for (int i = 0; i < curr_level.size(); i++) {
            if (i == 0) cout << string(spaces, ' ');
            else cout << string(spaces * 2, ' ');

            if (curr_level[i]) {
                cout << (curr_level[i]->left ? "/" : " ");
                cout << (curr_level[i]->right ? "\\" : " ");
            } else {
                cout << "  ";
            }
        }
        cout << "\n";

        vector<node*> next_level;
        for (auto n : curr_level) {
            if (n) {
                next_level.push_back(n->left);
                next_level.push_back(n->right);
            } else {
                next_level.push_back(nullptr);
                next_level.push_back(nullptr);
            }
        }
        curr_level = next_level;
    }
}

int main(){
    int value;
    cin >> value;
    insert_bigroot(value);
    while (true){
        cin >> value;
        if (value==0){
            break;
        }
        else{
        insert_child(value);
        }
    }
    print_tree(head);
}

