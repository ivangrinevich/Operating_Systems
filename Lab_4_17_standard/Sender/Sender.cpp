#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
#include "../wstr.h"

//  cmd line: Sender.exe file startEvent readSem writeSem mutex finishAllEvent finishEvent
int main(int argc, char* argv[]) {
    if (argc != 8) {
        std::cerr << "wrong number of arguments\n";
        return 0;
    }
    std::cout << "start process in\n";

    HANDLE startEvent = OpenEvent(EVENT_ALL_ACCESS,//устанавливает все возможные флаги доступа к объекту события
                                  false,//дескриптор не может наследоваться.
                                  reinterpret_cast<LPCSTR>(convertToWideString(argv[2]).c_str()));

    HANDLE readSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false,
                                   reinterpret_cast<LPCSTR>(convertToWideString(argv[3]).c_str()));
    HANDLE writeSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false,
                                    reinterpret_cast<LPCSTR>(convertToWideString(argv[4]).c_str()));

    HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, false, reinterpret_cast<LPCSTR>(convertToWideString(argv[5]).c_str()));

    HANDLE finishAllEvent = OpenEvent(EVENT_ALL_ACCESS, false,
                                      reinterpret_cast<LPCSTR>(convertToWideString(argv[6]).c_str()));
    HANDLE finishEvent = OpenEvent(EVENT_ALL_ACCESS, false,
                                   reinterpret_cast<LPCSTR>(convertToWideString(argv[7]).c_str()));

    if (!startEvent || !readSem || !writeSem || !mutex) {
        std::cerr << "cannot open objects with such names\n";
        return 0;
    }

    std::string filename = argv[1];
    std::ofstream fout{};

    SetEvent(startEvent);

    std::string message;
    HANDLE waitArr[] = { writeSem, finishAllEvent };

    while (true) {
        std::cout << "enter message(or f to finish this process): ";
        std::getline(std::cin, message);
        if (message == "f") {
            break;
        }
        if (WaitForMultipleObjects(2, waitArr, false, INFINITE) == WAIT_OBJECT_0 + 1) {//завершение
            break;
        }

        WaitForSingleObject(mutex, INFINITE);
        fout.open(filename, std::ofstream::binary | std::ofstream::app);
        fout << message << '\n';
        fout.close();


        /*освобождается мьютекс*/
        ReleaseMutex(mutex);//Для освобождения объекта
        /*Увеличить значение семафора можно посредством вызова функции: */
        ReleaseSemaphore(readSem, 1, //на сколько изменять счетчик
                         NULL);//предыдущее значение
    }
    SetEvent(finishEvent);//Для перевода любого события в сигнальное состояние
}