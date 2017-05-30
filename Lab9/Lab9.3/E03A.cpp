/*
Exercise 03 (version A)
---------------------------------

Visit N directory trees concurrently with N threads.

A C program receives N parameters on the command line.
Each parameter indicates a relative or an absolute path to
a file system directory tree.

The program has to run N threads (one for each argument).
Each thread recursively visits one of the directories, and,
for each directory entry, it prints-out its thread identifier
and the directory entry name.
The main thread awaits for the termination of all threads.
When all threads have visited their directory also the program
ends.

Version A
In the previous version (Version A), as all output lines are
generated independently by each single thread, printing messages
from different threads are interleaved on standard output.
To avoid interleaved output it is usually possibile to proceed
in several ways.
Then, extend version A as follows.
*/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <io.h>

#define TYPE_FILE 1
#define TYPE_DIR 2
#define TYPE_DOT 3

#define MAX_BUFF_SIZE 1024
#define ALL_FILES "*"

#define ERR_NO_ARGS -1

DWORD WINAPI threadFunction(LPVOID);
VOID copy_dir_tree(PTCHAR, DWORD);
static DWORD FileType(LPWIN32_FIND_DATA);

int _tmain(int argc, LPTSTR argv[]) {

	if (argc == 1) {
		_tprintf(_T("Bad syntax. Usage: %s [folder1] [folder2] ... \n"), argv[0]);
		return ERR_NO_ARGS;
	}

	INT nThreads = argc - 1;
	PDWORD myThreadID = (PDWORD)malloc(sizeof(DWORD)*nThreads);
	PHANDLE myThreadHandle = (PHANDLE)malloc(sizeof(HANDLE)*nThreads);

	// Create as many threads as argc-1
	for (INT i = 0; i < nThreads; i++) {
		myThreadID[i] = i;
		myThreadHandle[i] = CreateThread(0, 0, threadFunction, &argv[i + 1], 0, &myThreadID[i]);
	}

	// Main process waits for process termination
	WaitForMultipleObjects(nThreads, myThreadHandle, TRUE, INFINITE);

	system("pause");
	return 0;

}

DWORD WINAPI threadFunction(LPVOID myParameter) {
	// Each prints its thread ID and the directory entry name

	PTCHAR* temp = (PTCHAR*)myParameter;
	PTCHAR threadSource = *temp;

	copy_dir_tree(threadSource, GetCurrentThreadId());
	return 0;
}

static DWORD FileType(LPWIN32_FIND_DATA pFileData)
{
	BOOL flag_dir;
	DWORD FType;
	FType = TYPE_FILE;
	flag_dir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	if (flag_dir)
		if (lstrcmp(pFileData->cFileName, _T(".")) == 0 || lstrcmp(pFileData->cFileName, _T("..")) == 0)
			FType = TYPE_DOT;
		else
			FType = TYPE_DIR;
	return FType;
}

VOID copy_dir_tree(PTCHAR mySource, DWORD threadID)
{
	HANDLE SearchHandle;
	WIN32_FIND_DATA FindData;
	DWORD FType;
	LPTSTR tmpSrc, newSource = (LPTSTR) "", token;
	PTCHAR buffer, source;
	INT tokenCount = 0;

	// String for the source directory
	source = mySource;

	buffer = (PTCHAR)malloc(sizeof(TCHAR)*MAX_BUFF_SIZE);
	_sntprintf(buffer, MAX_PATH, source, _tcslen(source));
	_sntprintf(buffer + _tcslen(buffer), MAX_PATH, (PTCHAR)ALL_FILES, _tcslen((PTCHAR)ALL_FILES));
	SearchHandle = FindFirstFile(buffer, &FindData);

	do {
		FType = FileType(&FindData);

		if (FType == TYPE_FILE) {
			_tprintf(_T("[%d] Directory Entry - File: %s\n"), threadID, FindData.cFileName);
		}
		if (FType == TYPE_DIR) {
			_tprintf(_T("[%d] Directory Entry - Directory: %s \n"), threadID, FindData.cFileName);
			_sntprintf(buffer + _tcslen(buffer) - _tcslen((PTCHAR)ALL_FILES), MAX_PATH, FindData.cFileName, sizeof(FindData.cFileName));
			_sntprintf(buffer + _tcslen(buffer), MAX_PATH, (PTCHAR) "\\", _tcslen((PTCHAR) "\\"));
			copy_dir_tree(buffer, threadID);

			_sntprintf(buffer, MAX_PATH, source, _tcslen(source));
			_sntprintf(buffer + _tcslen(buffer), MAX_PATH, (PTCHAR) "\\", _tcslen((PTCHAR) "\\"));

		}

	} while (FindNextFile(SearchHandle, &FindData));

	FindClose(SearchHandle);
	return;
}