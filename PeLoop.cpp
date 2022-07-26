// PeLoop.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Windows.h"
#include <iostream>
#include <filesystem>
#include <shlwapi.h>
#include <string>

#pragma comment(lib,"shlwapi.lib")
using namespace std;

const short CONSOLE_ROW_REPEAT = 12;
const int MAX_LENGTH = 1024;

HANDLE hConsole = NULL;
WORD attributes = 0;

wstring input;
wstring output;
wstring pe2Json = L"pe2Json.exe";


void printHeader() {

    SetConsoleTextAttribute(hConsole,
        FOREGROUND_BLUE | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

    printf(" _______  _______  ___      _______  _______  _______ \n");
    printf("|       ||       ||   |    |       ||       ||       |\n");
    printf("|    _  ||    ___||   |    |   _   ||   _   ||    _  |\n");
    printf("|   |_| ||   |___ |   |    |  | |  ||  | |  ||   |_| |\n");
    printf("|    ___||    ___||   |___ |  |_|  ||  |_|  ||    ___|\n");
    printf("|   |    |   |___ |       ||       ||       ||   |    \n");
    printf("|___|    |_______||_______||_______||_______||___|    \n");
    printf("------------------------------------------------------\n");
    printf(" by: RudeNetworks.com | version: 0.8 beta\n");
    printf(" - administrator privileges required.\n");


    SetConsoleTextAttribute(hConsole,
        FOREGROUND_GREEN);

}

void printUsage() {
    printf(" Usage:\n");
    printf(" peloop.exe <Folder To Scan> <Output Folder> <Optional PE2Json>\n");
    printf(" peloop.exe \"c:\windows\" \"c:\rev\"\n");
    printf("---------------------------------------------------------------\n");
}

void executePe2Json(wstring localPath, wstring outputPath) {

    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO StartupInfo;

    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

    TCHAR fullArgs[MAX_LENGTH * 5];

    swprintf_s(fullArgs, L"%s %s %s", pe2Json.c_str(), localPath.c_str(), outputPath.c_str());

    if (CreateProcess(NULL, fullArgs,
        NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL,
        NULL, &StartupInfo, &ProcessInfo))
    {
        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
        CloseHandle(ProcessInfo.hThread);
        CloseHandle(ProcessInfo.hProcess);
    }

}

void createDirectoryRecursively(wstring path)
{
    signed int pos = 0;
    do
    {
        pos = path.find_first_of(L"\\/", pos + 1);
        CreateDirectory(path.substr(0, pos).c_str(), NULL);
    } while (pos != std::wstring::npos);
}

BOOL isFile(LPCWSTR file)
{
    DWORD attr = GetFileAttributes(file);

    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    return false;
}


bool isDirectory(LPCWSTR path) {
    DWORD attr = GetFileAttributes(path);
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    return false;
}

void resetConsoleCursor(short txtLenght) {
    wstring f(txtLenght, ' ');
    WriteConsole(hConsole, f.c_str(), f.length(), NULL, NULL);
    SetConsoleCursorPosition(hConsole, { 0, CONSOLE_ROW_REPEAT });
}

void printConsole(LPCWSTR fullPath) {

    int len = wcslen(fullPath);
    WCHAR msg[MAX_LENGTH * 5] = { 0 };

    swprintf_s(msg, L"%s", fullPath);

    CONSOLE_SCREEN_BUFFER_INFO csbi = {};
    BOOL ok = GetConsoleScreenBufferInfo(hConsole, &csbi);
    SetConsoleCursorPosition(hConsole, { 0, CONSOLE_ROW_REPEAT });
    WriteConsole(hConsole, msg, len, NULL, NULL);

}

void runRecursive(LPCWSTR lpPathFile)
{
    WCHAR pathCurr[MAX_LENGTH];
    WCHAR pathFullCurr[MAX_LENGTH];
    WCHAR pathFullOutput[MAX_LENGTH];

    wcscpy_s(pathCurr, lpPathFile);
    wcscat_s(pathCurr, L"\\*");

    WIN32_FIND_DATA wn;
    HANDLE hFindFile = FindFirstFile(pathCurr, &wn);


    do
    {

        if (hFindFile == INVALID_HANDLE_VALUE)
        {
            continue;
        }

        if (wcslen(wn.cFileName) == 0)
        {
            continue;
        }

        CharLowerBuff(wn.cFileName, wcslen(wn.cFileName));

        if (wcscmp(wn.cFileName, L"..") == 0 || wcscmp(wn.cFileName, L".") == 0)
        {
            continue;
        }

        swprintf_s(pathFullCurr, L"%s\\%s", lpPathFile, wn.cFileName);



        wstring dub(pathFullCurr);
        dub = dub.substr(3);

        swprintf_s(pathFullOutput, L"%s\\%s", output.c_str(), dub.c_str());

        if (isDirectory(pathFullCurr)) {

            createDirectoryRecursively(pathFullOutput);

            runRecursive(pathFullCurr);

            continue;
        }


        LPCWSTR extenstion = PathFindExtension(pathFullCurr);

        if (wcscmp(extenstion, L".dll") != 0 && wcscmp(extenstion, L".exe") != 0) {
            continue;
        }

        swprintf_s(pathFullOutput, L"%s.%s", pathFullOutput, L"json");

        printConsole(pathFullCurr);

        executePe2Json(pathFullCurr, pathFullOutput);

        resetConsoleCursor(wcslen(pathFullCurr));


    } while (FindNextFile(hFindFile, &wn));

    printf("\n");
}

int main(int argc, char* argv[])
{
    CONSOLE_SCREEN_BUFFER_INFO Info;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hConsole, &Info);
    attributes = Info.wAttributes;

    printHeader();


    if (argc < 3) {
        printUsage();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attributes);
        return -1;
    }


    WCHAR inputBuff[MAX_LENGTH] = {0};
    WCHAR outputBuff[MAX_LENGTH] = { 0 };

    MultiByteToWideChar(0, 0, argv[1], strlen(argv[1]), inputBuff, strlen(argv[1]));
    MultiByteToWideChar(0, 0, argv[2], strlen(argv[2]), outputBuff, strlen(argv[2]));

    input.assign(inputBuff);
    output.assign(outputBuff);

    if (!isDirectory(output.c_str())) {
        createDirectoryRecursively(output.c_str());
    }


    wprintf(L"input: %s\n", input.c_str());
    wprintf(L"output: %s\n", output.c_str());

    if (argc == 4) {
        
        WCHAR pe2JsonBuff[MAX_LENGTH] = { 0 };
        MultiByteToWideChar(0, 0, argv[3], strlen(argv[3]), pe2JsonBuff, strlen(argv[3]));
        
        pe2Json.assign(pe2JsonBuff);

        if (!isFile(pe2Json.c_str())) {
            printf("pe2Json.exe not exists!");
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attributes);
            return -1;
        }
    }

    if (!isDirectory(input.c_str())) {
        return -1;
    }

    runRecursive(input.c_str());

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attributes);

    return 0;
}

