#include <iostream>
#include <fstream>
#include <Windows.h>
#include <vector>
#include <string>
#include <list>
#include"../wstr.h"
/*В кольцевой очереди элемент, который выходит (удаляется) из очереди помещается в ее конец */
int main() {
    std::cout << "Enter file name: ";
    std::string filename;
    std::cin >> filename;

    std::cout << "Enter number of records: ";
    int recordsNumber;
    std::cin >> recordsNumber;

    std::ofstream file(filename, std::ofstream::binary);
    file.close();

    std::cout << "Enter number of processes: ";
    int processesNumber;
    std::cin >> processesNumber;
    if (processesNumber < 1) {
        std::cerr << "wrong number of threads\n";
        return 0;
    }

    std::vector<HANDLE> startEvents(processesNumber);//массив событий о начале процесса
    std::vector<HANDLE> finishEvents(processesNumber);//массив событий о конце процесса

    std::string finishAllEventName = "finishAll";//конец всех событий

    HANDLE finishAll = CreateEvent(NULL,// атрибуты защиты
                                   true, //событие с ручным сбросом,
                                   false,//несигнальное состояние
                                   reinterpret_cast<LPCSTR>(convertToWideString(finishAllEventName).c_str()));// имя события

    std::vector<STARTUPINFO> si(processesNumber);//STARTUPINFO используется функцией 
    //CreateProcess для определения свойств главного окна
    std::vector<PROCESS_INFORMATION> procInfo(processesNumber);//PROCESS_INFORMATION содержит идентификаторы и дескрипторы нового создаваемого процесса

    std::string startEventName;
    std::string finishEventName;
    std::string cmdLine;

    const std::string readSemaphore = "readSemaphore";
    const std::string writeSemaphore = "writeSemaphore";
    const std::string mutexName = "mutex";//для защиты бин файла

    HANDLE readSem = CreateSemaphore(NULL,//атрибуты защиты
                                     0,// начальное значение семафора
                                     recordsNumber,//максимальное значение семафора
                                     reinterpret_cast<LPCSTR>(convertToWideString(readSemaphore).c_str()));//имя семафора
    HANDLE writeSem = CreateSemaphore(NULL, recordsNumber, recordsNumber,
                                      reinterpret_cast<LPCSTR>(convertToWideString(writeSemaphore).c_str()));
    HANDLE mutex = CreateMutex(NULL,//дескриптор мьютекса не наследуется и доступ к мьютексу имеют все пользователи.
                               false,// созданный мьютекс свободен.
                               reinterpret_cast<LPCSTR>(convertToWideString(mutexName).c_str()));


    for (int i = 0; i < processesNumber; ++i) {

        startEventName = "startEvent" + std::to_string(i);
        startEvents[i] = CreateEvent(
                NULL,
                false,
                false,
                reinterpret_cast<LPCSTR>(convertToWideString(startEventName).c_str())
        );


        finishEventName = "finishEvent" + std::to_string(i);
        finishEvents[i] = CreateEvent(
                NULL,
                false,
                false,
                reinterpret_cast<LPCSTR>(convertToWideString(finishEventName).c_str())
        );
        /*вид главного окна запускаемого приложения определяется по умолчанию самой операционной системой Windows*/
        ZeroMemory(&si[i], sizeof(STARTUPINFO));
        si[i].cb = sizeof(STARTUPINFO);

        cmdLine = "Sender.exe " +
                  filename + " " +
                  startEventName + " " +
                  readSemaphore + " " +
                  writeSemaphore + " " +
                  mutexName + " " +
                  finishAllEventName + " " +
                  finishEventName;

        if (!CreateProcess(
                NULL, // имя исполняемого модуля
                reinterpret_cast<LPSTR>((LPWSTR) convertToWideString(cmdLine).c_str()), // командная строка
                NULL, // атрибуты защиты процесса
                NULL, //потока
                false,// наследуемый ли дескриптор
                CREATE_NEW_CONSOLE,
                NULL, NULL,
                &si[i],
                &procInfo[i])) {

            std::cerr << "error creating process";
            std::cerr << GetLastError();
            return 0;
        }
        else {
            std::cout << "start process ext\n";

        }
    }

    WaitForMultipleObjects(processesNumber, // количество объектов
                           &startEvents[0],// массив дескрипторов объектов
                           true,
                           INFINITE);

    std::cout << "Enter r to read or any other string to exit\n";
    std::string input = "r";
    std::string message;
    std::ifstream fin{};
    std::string cachedFile;

    while (true) {
        std::cout << "enter next action: ";
        std::cin >> input;
        if (input != "r") {
            break;
        }

        WaitForSingleObject(readSem, INFINITE);//ждем ответа от семафора чтения
        WaitForSingleObject(mutex, INFINITE);

        fin.open(filename, std::ifstream::binary);
        std::getline(fin, message);
        std::cout << "new message: " << message << '\n';

        while (std::getline(fin, message)) {
            cachedFile += message + '\n';
        }
        fin.close();
        file.open(filename, std::ofstream::binary | std::ofstream::trunc);
        file << cachedFile;
        file.close();
        ReleaseMutex(mutex);
        // После того как необходимость в работе с объектом отпала нужно вызвать функцию ReleaseSemaphore(), чтобы освободить счетчик.
        ReleaseSemaphore(writeSem,//// хенд семафора
                         1,// на сколько изменять счетчик
                         NULL);// предыдущее значение

        cachedFile = "";
    }

    SetEvent(finishAll);//завершаем 
    WaitForMultipleObjects(processesNumber, &finishEvents[0], true, INFINITE);//ждем ответа от событий о конце

    CloseHandle(readSem);
    CloseHandle(writeSem);
    CloseHandle(mutex);
    CloseHandle(finishAll);

    for (int i = 0; i < processesNumber; ++i) {
        CloseHandle(finishEvents[i]);
        CloseHandle(startEvents[i]);
        CloseHandle(procInfo[i].hThread);
        CloseHandle(procInfo[i].hProcess);
    }

}