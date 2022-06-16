#include <windows.h>
#include <iostream>
#include<vector>
#include<string>
using namespace std;

double average;
struct fun {
    int min = 0;
    int max = 0;
    double* vec = NULL;
    int size = 0;
    double average = 0;
};



DWORD WINAPI Min_max(LPVOID aa) {
    fun* a = (fun*)aa;
    cout << "Thread Min_max started" << "\n";
    a->min = a->vec[0];
    a->max = a->vec[0];

    for (int i = 1; i <a->size; i++)
    {
        if (a->max < a->vec[i]) {
            a->max = a->vec[i];
        }Sleep(7);
        if (a->min > a->vec[i]) {
            a->min = a->vec[i];
        }Sleep(7);
    }

    cout << "min in array is: " << a->min << "\n";
    cout << "max in array is: " << a->max << "\n";
    cout << "Thread Min_max ended" << "\n";
    return 0;
}






