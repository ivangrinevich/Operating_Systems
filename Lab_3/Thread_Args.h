#include <ostream>
#include <ctime>
#include <windows.h>
#include <process.h>

struct threadArgs{
    int* arr;
    int n;
    int num;
    HANDLE actions[2];
    threadArgs(int* _arr, int _n, int _num) : arr(_arr), num(_num),  n(_n){
        //Second action is stop thread
        actions[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
        actions[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
};
