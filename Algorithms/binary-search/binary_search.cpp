#include <iostream>
#include <typeinfo>
#include <string>
#include <cmath>


using namespace std;

int main(){
    int n=9;
    int nums[10]={1,2,3,4,5,6,7,8,9,10};
    int low=0;
    int high=sizeof(nums)/sizeof(nums[0]);
    int mid= (high + low)/2;
    while (1){
        if (n==nums[mid]){
            cout << mid;
            break;
        }
        else if(n>nums[mid]){
            low=mid+1;
            mid= (high + low)/2;
        }
        else if (nums[mid]>n){
            high=mid-1;
            mid= (high + low)/2;
        }
        else{
            cout << "n not in array";
            break;
        }
    }
    
    }
    
    
    

