#include <iostream>
#include <ctime>
#include <condition_variable>



struct threadArgs{
    int* arr;
    int n;
    int num;
    bool actions[2];
    //[0] - to continue
    //[1] - to terminate
    std::condition_variable doAction;
    std::mutex actionMutex;
    threadArgs(int* _arr, int _n, int _num) : arr(_arr), num(_num),  n(_n){
        actions[0] = false;
        actions[1] = false;
    }
};