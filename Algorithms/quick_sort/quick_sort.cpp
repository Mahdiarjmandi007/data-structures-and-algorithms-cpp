#include <iostream>
#include <vector>
using namespace std;

void create_primary_list(int number, vector<int>& list){
    if (number != 0){
        list.push_back(number);
    }
}

void display_list(const vector<int>& list){
    for (size_t i = 0; i < list.size(); i++){
        cout << list[i] << " ";
    }
    cout << endl;
}

int partitions(vector<int>& list,int low, int high){
    int pivot = list[high];
    int i = low -1;

    for (int j=low ; j<high; j++ ){
        if (list[j]<pivot){
        i++;
        swap(list[i],list[j]);
        }
    }

    swap(list[i+1],list[high]);
    return i+1;
}

void quick_sort(vector<int>& list,int low,int high){
    if (low < high){
        int p=partitions(list,low,high);

        quick_sort(list,low,p-1);
        quick_sort(list,p+1,high);

    }
}


int main(){
    int choice,num,n;
    vector<int> primary_list;
    vector<int> sorted_list;
    while(true){
        cout << "**********Menu**********\n" 
             << "1.Insert Primary number for list\n"
             << "2.Display\n"
             << "3.Exit\n";
        cin >> choice;
       

        switch(choice){
           case 1:
                primary_list.clear();
                sorted_list.clear();

                cout << "Insert your list (Enter 0 to finish):\n";

                while(true){
                    cin >> num;

                    if(num == 0){
                        break;
                    }

                    create_primary_list(num, primary_list);
                }

                sorted_list = primary_list;

                n = sorted_list.size();

                if(n > 1){
                    quick_sort(sorted_list, 0, n - 1);
                }

                break;
            case 2:
                cout << "Primary List: ";
                display_list(primary_list);

                cout << "Sorted List: ";
                display_list(sorted_list);
                break;

            case 3:
                return 0;
            default:
                 cout << "\033[1;31mWarninng!!! Please choice the correct number\n\033[0m";
                 break;
                
}
}
}