#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <Winsock2.h>
#include <process.h>
#include "fileMang.h"

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define HAVE_STRUCT_TIMESPEC

#include <pthread.h>
#include <experimental/filesystem> // http://en.cppreference.com/w/cpp/experimental/fs

#pragma comment(lib,"ws2_32.lib")
pthread_mutex_t mutex12 = PTHREAD_MUTEX_INITIALIZER;

using namespace std;
int totalFiles = 0;
int totalVectors = 100;
fileMang FM;

//vector<string> addedClients;

string mainRunFunction(string inputFromThread,string threadId) {

    int com = 1;
    //pthread_mutex_lock(&mutex12);
    string command = inputFromThread.substr(0, inputFromThread.find('|'));
    inputFromThread.erase(0, inputFromThread.find("|") + 1);

    if (command == "0") {
        string fileName = inputFromThread.substr(0, inputFromThread.find("|"));
        if (FM.fifFileExists(fileName) == 1) {
            string result = FM.fcloseFile(fileName,threadId);
            return result;
        }
    }

    if (command == "1") {

        string fileName = inputFromThread.substr(0, inputFromThread.find("|"));
        if (FM.fifFileExists(fileName) == 1) {
            string data = "The file '" + fileName + "' already prsent in the directory";
            return data;
        }
        else {
            FM.fcreateFile(fileName);
            return "File Created Succeefully";
        }

    }
    if (command == "2") {
        string fileName = inputFromThread.substr(0, inputFromThread.find('|'));
        if (FM.fifFileExists(fileName)) {
            FM.fdeleteFile(fileName);
            return "File Deleted Successfully";
        }
        else {
            string data = "File with this name (" + fileName + ") is not available in the directory";
            return data;
        }
    }
    if (command == "3") {
        string fileName = inputFromThread.substr(0, inputFromThread.find("|"));
        inputFromThread = inputFromThread.erase(0, inputFromThread.find("|") + 1);
        if (FM.fifFileExists(fileName) == 1) {
            string opentype = inputFromThread.substr(0, inputFromThread.find("|"));
            inputFromThread = inputFromThread.erase(0, inputFromThread.find("|") + 1);

            string oRes = FM.ffileAlreadyOpen(threadId, fileName);
            /*if (oRes.size() > 0) {
                return oRes;
            }
            else {
               
            }*/
            if (opentype == "r" || opentype == "w" || opentype == "a") {
                string text = inputFromThread.substr(0, inputFromThread.find("|"));
                return FM.fopenFile(fileName, opentype, text, threadId);
            }
            else if (opentype == "rf") {
                int position = stoi(inputFromThread.substr(0, inputFromThread.find('|')));
                inputFromThread = inputFromThread.erase(0, inputFromThread.find('|') + 1);

                int length = stoi(inputFromThread.substr(0, inputFromThread.find('|')));
                inputFromThread = inputFromThread.erase(0, inputFromThread.find('|') + 1);
                FM.freadFrom(fileName, position, length);
            }
            
        }
        else {
            string data = "The file does (" + fileName + ") not exits in the directory";
            return data;
        }
    }
    if (command == "4") {
        string sourcefilename, destinationfileName;
        sourcefilename = inputFromThread.substr(0, inputFromThread.find('|'));
        inputFromThread = inputFromThread.erase(0, inputFromThread.find('|') + 1);

        if (FM.fifFileExists(sourcefilename)) {
            destinationfileName = inputFromThread.substr(0, inputFromThread.find('|'));
            inputFromThread = inputFromThread.erase(0, inputFromThread.find('|') + 1);

            if (FM.fifFileExists(destinationfileName)) {
                FM.fmoveFile(sourcefilename, destinationfileName);
                return "File Move Successfully";
            }
            else {
                return "The target File does not exist: " + destinationfileName;
            }
        }
        else {
            return "The source File does not exist: " + sourcefilename;
        }
    }
    if (command == "5") {
        string data = FM.fprintToCheck();
        return data;
    }
    /*if (command == "6") {
        FM.fprintDeleteVector();
    }*/

    if (command == "6") {
        FM.fprintMap();
        return "Map Checking";
    }

    if (command != "1" || command != "2" || command != "3" || command != "4" || command != "5" || command != "0"|| command != "6") {
        return "INVALID COMMAND IS ENTERED";
    }

    //pthread_mutex_unlock(&mutex12);
}

void* ServClient(void* data) {

    SOCKET* client = (SOCKET*)data;
    SOCKET Client = *client;

    cout << "Client Connected (" << GetCurrentThreadId() << ")" << endl;
    char chunk[512];

    char name[100];
    int ij = 0;


    string responseFromFunction = "";
    while (true) {


        memset(chunk, '\0', 512);
        if (recv(Client, chunk, 512, 0)) {
            if (chunk[0] == '-') {
                break;
            }
            cout << chunk << "\t" << GetCurrentThreadId() << endl;
            string myText(chunk);
            if (ij == 0) {
                FM.addedClients.push_back(myText);
                cout << "Name of client: " << myText << endl;
                const char* dataSend = "You are connected";
                send(Client, dataSend, (int)strlen(dataSend), 0);
            }
            else {
                std::ostringstream stream;
                stream << GetCurrentThreadId();
                string threadId = stream.str();
                responseFromFunction = mainRunFunction(myText,threadId);
                const char* dataSend = responseFromFunction.c_str();
                send(Client, dataSend, (int)strlen(dataSend), 0);
                //chunk[0] = '\0';
            }

            chunk[0] = '\0';
        }
        ij++;
    }
    cout << "Client disconnected (" << GetCurrentThreadId() << ")" << endl;
    return NULL;
}


int main() {

    WSADATA wsaData;
    int iResult;
    sockaddr_in addr;
    SOCKET sock, client;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(95);
    //addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    addr.sin_addr.S_un.S_addr = INADDR_ANY;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult) {
        cout << "WSA startup failed" << endl;
        return 0;
    }


    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET) {
        cout << "Invalid socket" << endl;
        return 0;
    }

    iResult = bind(sock, (sockaddr*)&addr, sizeof(sockaddr_in));

    if (iResult) {
        cout << "bind failed :" << GetLastError << endl;
        return 0;
    }

    iResult = listen(sock, SOMAXCONN);

    if (iResult) {
        cout << "iResult failed" << GetLastError << endl;
        return 0;
    }

    pthread_t* threads = new pthread_t[100];
    int j = 0;
    while (client = accept(sock, 0, 0)) {

        if (client == INVALID_SOCKET) {
            cout << "Invalid client Socket" << GetLastError << endl;
            continue;
        }
        pthread_create(&threads[j], NULL, ServClient, (void*)&client);

        j++;
    }

    /* for (int i = 0; i < j; i++) {
         pthread_join(threads[i], NULL);
     }*/

     /*for (int i = 0; i < addedClients.size(); i++) {
         cout << addedClients.at(i) << endl;
     }*/

    FM.fprintToCheck();
    cout << endl;

    return 0;

}