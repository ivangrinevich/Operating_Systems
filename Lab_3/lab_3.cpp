#include <iostream>
#include <vector>
#include <windows.h>
#include <process.h>
#include "Thread_Args.h"
using std::cin;

CRITICAL_SECTION cs;
HANDLE startThreadsEvent;
std::vector<HANDLE> threadEvents;



void printArray(int* arr, int n){
    EnterCriticalSection(&cs);
    for(int i = 0; i < n; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
    LeaveCriticalSection(&cs);
}

UINT WINAPI marker(void *p){
    threadArgs* args = static_cast<threadArgs*>(p);
    WaitForSingleObject(startThreadsEvent, INFINITE);
    srand(args->num);
    printf("Thread #%d started.\n", args->num);

    int count = 0;
    while(true){
        EnterCriticalSection(&cs);
        int i = rand() % args->n;
        if(args->arr[i] == 0){
            Sleep(5);
            args->arr[i] = args->num;
            ++count;
            LeaveCriticalSection(&cs);
            Sleep(5);
        } else {
            printf("Thread #%d. Marked %d elems. Unable to mark a[%d].\n", args->num, count, i);
            LeaveCriticalSection(&cs);
            SetEvent(threadEvents[args->num-1]);
            int action = WaitForMultipleObjects(2,args->actions, FALSE, INFINITE) - WAIT_OBJECT_0;
            if(action == 1){
                for(int i = 0; i < args->n; i++){
                    if(args->arr[i] == args->num){
                        args->arr[i] = 0;
                    }
                }
                printf("Thread #%d is terminated.\n", args->num);
                break;
            }
        }
    }
    return NULL;
}

int main() {
    //creating array
    int n;
    std::cout << "Enter number of elements in array";
    std::cin >> n;
    int* arr = new int[n];
    for(int i = 0; i < n; i++){
        arr[i] = 0;
    }
    printf("Array of %d elements is created.\n", n);

    //creating threads
    int threadCount;
    std::cout << "Enter number of threads";
    std::cin >> threadCount;
    startThreadsEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    std::vector<HANDLE> threads;
    HANDLE currThread;
    std::vector<threadArgs*> argsVec;
    threadArgs* currArgs;
    bool* terminated = new bool[threadCount];
    for(int i = 0; i < threadCount; i++){
        currArgs = new threadArgs(arr, n, i+1);
        currThread =
                (HANDLE)_beginthreadex(NULL, 0, marker, currArgs, 0, NULL);
        if(currThread == NULL) {
            printf("Thread can't be created.\n");
            return GetLastError();
        }
        terminated[i] = false;
        threadEvents.push_back(CreateEvent(NULL, TRUE, FALSE, NULL));
        argsVec.push_back(currArgs);
        threads.push_back(currThread);
    }
    printf("%d threads are ready to start.\n" , threadCount);

    //starting threads
    InitializeCriticalSection(&cs);
    PulseEvent(startThreadsEvent);
    int terminatedCount = 0, k;
    while(terminatedCount != threadCount) {
        WaitForMultipleObjects(threadCount, &threadEvents[0], TRUE, INFINITE);
        printArray(arr, n);
        printf("All threads are paused. Which one is to terminate?\n");
        cin >> k;
        if(k <= 0 || k > threadCount || terminated[k - 1]){
            printf("Invalid index. Try again.\n");
            continue;
        }
        terminated[k-1] = true;
        SetEvent(argsVec[k - 1]->actions[1]);
        WaitForSingleObject(threads[k-1], INFINITE);
        ++terminatedCount;
        printArray(arr, n);
        for(int i = 0; i < threadCount; ++i){
            if(terminated[i])
                continue;
            ResetEvent(threadEvents[i]);
            SetEvent(argsVec[i]->actions[0]);
        }
    }
    DeleteCriticalSection(&cs);
    delete[] arr;
    return 0;
}