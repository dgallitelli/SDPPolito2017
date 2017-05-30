/*
Exercise 03 (version B)
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

Version B
This version uses files to avoid interleaving.
Each thread stores its output on a file whose name is generated (to avoid conflicts) based on the thread identifier. 
When all working threads end, the main thread copies all files on standard output with the desired strategy (and no interruption).

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
#define ERR_INV_HANDLE -2
#define ERR_WRITE_FILE -3
#define ERR_READ_FILE -4
#define ERR_GET_SIZE_FILE -5

#define UNEXPECTED_FTYPE -9000

#define STDOUT_FILE " Directory Entry - File: "
#define STDOUT_DIR " Directory Entry - Directory: "

DWORD WINAPI threadFunction(LPVOID);
VOID copy_dir_tree(PTCHAR, DWORD, PHANDLE);
static DWORD FileType(LPWIN32_FIND_DATA);

struct THREADINFO {
	DWORD ThreadID;
	PTCHAR folderToRead;
	PHANDLE pToThreadHandle;
};

struct FILEDIRINFO {
	DWORD FType;
	DWORD threadID;
	PTCHAR fileDirName;
};

INT nThreads;

int _tmain(int argc, LPTSTR argv[]) {

	if (argc == 1) {
		_tprintf(_T("Bad syntax. Usage: %s [folder1] [folder2] ... \n"), argv[0]);
		return ERR_NO_ARGS;
	}

	nThreads = argc - 1;
	THREADINFO* threadsInfoArr = (THREADINFO*)malloc(sizeof(THREADINFO)*nThreads);
	PHANDLE myThreadHandle = (PHANDLE)malloc(sizeof(HANDLE)*nThreads);
	DWORD nRead;

	// Create as many threads as argc-1
	for (INT i = 0; i < nThreads; i++) {
		threadsInfoArr[i].ThreadID = i;
		threadsInfoArr[i].folderToRead = argv[i + 1];

		myThreadHandle[i] = CreateThread(0, 0, threadFunction, &threadsInfoArr[i], 0, NULL);

		threadsInfoArr[i].pToThreadHandle = &myThreadHandle[i];
	}

	// Main process waits for process termination
	WaitForMultipleObjects(nThreads, myThreadHandle, TRUE, INFINITE);

	// Once all processes are done, collect results from files
	for (INT i = 0; i < nThreads; i++) {
		// Output - Reading from thread ID
		_tprintf(_T("Reading output generated from thread %d - directory: %s .\n"), threadsInfoArr[i].ThreadID, threadsInfoArr[i].folderToRead);
		// Read the file threadID.txt
		PTCHAR threadFile = (PTCHAR)malloc(sizeof(TCHAR)*MAX_BUFF_SIZE);
		_sntprintf(threadFile, MAX_PATH, _T("%d.txt"), threadsInfoArr[i].ThreadID, sizeof(".txt") + sizeof(threadsInfoArr[i].ThreadID));
		HANDLE myFile = CreateFile(threadFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (myFile == INVALID_HANDLE_VALUE)
			return ERR_INV_HANDLE;
		// Get file size to read #entries
		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(myFile, &fileSize))
			return ERR_GET_SIZE_FILE;
		INT numberOfEntries =  fileSize.LowPart / sizeof(FILEDIRINFO);
		// Loop for #entries
		for (INT i = 0; i < numberOfEntries; i++) {
			FILEDIRINFO* myEntry = (FILEDIRINFO*) malloc(sizeof(FILEDIRINFO));
			if (!ReadFile(myFile, myEntry, sizeof(FILEDIRINFO), &nRead, NULL))
				return ERR_READ_FILE;
			// Print the findings on stdout
			_tprintf(_T("[%d] "), myEntry->threadID);
			if (myEntry->FType == TYPE_DIR)
				_tprintf(_T(STDOUT_DIR));
			else if (myEntry->FType == TYPE_FILE)
				_tprintf(_T(STDOUT_FILE));
			else
				return UNEXPECTED_FTYPE;
			_tprintf(_T("%s\n"), myEntry->fileDirName);
			free(myEntry);
		}
		// Close the handle
		CloseHandle(myFile);
		_tprintf(_T("\n\n"));
	}

	system("pause");
	return 0;

}

DWORD WINAPI threadFunction(LPVOID myThreadInfoStruct) {
	// Each prints its thread ID and the directory entry name

	THREADINFO* threadInfo = (THREADINFO*)myThreadInfoStruct;

	threadInfo->ThreadID = GetCurrentThreadId();

	PTCHAR folderToRead = threadInfo->folderToRead;
	PTCHAR threadFile = (PTCHAR)malloc(sizeof(TCHAR)*MAX_BUFF_SIZE);

	_sntprintf(threadFile, MAX_PATH, _T("%d.txt"), threadInfo->ThreadID, sizeof(".txt")+sizeof(threadInfo->ThreadID));

	HANDLE myFile;
	myFile = CreateFile(threadFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (myFile == INVALID_HANDLE_VALUE)
		return ERR_INV_HANDLE;

	copy_dir_tree(folderToRead, threadInfo->ThreadID, &myFile);

	CloseHandle(myFile);
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

VOID copy_dir_tree(PTCHAR mySource, DWORD threadID, PHANDLE handleAddr)
{
	HANDLE SearchHandle;
	WIN32_FIND_DATA FindData;
	DWORD FType, nWritten;
	LPTSTR newSource = (LPTSTR) "";
	PTCHAR buffer, source;
	INT tokenCount = 0;

	PTCHAR buffToWrite = (PTCHAR)malloc(sizeof(TCHAR)*MAX_BUFF_SIZE);

	// String for the source directory
	source = mySource;
	HANDLE fileHandle = *handleAddr;

	buffer = (PTCHAR)malloc(sizeof(TCHAR)*MAX_BUFF_SIZE);
	_sntprintf(buffer, MAX_PATH, source, _tcslen(source));
	_sntprintf(buffer + _tcslen(buffer), MAX_PATH, (PTCHAR)ALL_FILES, _tcslen((PTCHAR)ALL_FILES));
	SearchHandle = FindFirstFile(buffer, &FindData);

	do {
		FType = FileType(&FindData);

		if (FType == TYPE_FILE) {

			FILEDIRINFO newFileInfo;
			newFileInfo.FType = FType;
			newFileInfo.threadID = threadID;
			newFileInfo.fileDirName = (PTCHAR)malloc(sizeof(TCHAR)*MAX_BUFF_SIZE);
			lstrcpy(newFileInfo.fileDirName, FindData.cFileName);

			if (!WriteFile(fileHandle, &newFileInfo, sizeof(FILEDIRINFO), &nWritten, NULL))
				exit(ERR_WRITE_FILE);
				
		}
		if (FType == TYPE_DIR) {

			FILEDIRINFO newDirInfo;
			newDirInfo.FType = FType;
			newDirInfo.threadID = threadID;
			newDirInfo.fileDirName = (PTCHAR)malloc(sizeof(TCHAR)*MAX_BUFF_SIZE);
			lstrcpy(newDirInfo.fileDirName, FindData.cFileName);

			if (!WriteFile(fileHandle, &newDirInfo, sizeof(FILEDIRINFO), &nWritten, NULL))
				exit(ERR_WRITE_FILE);
				
			// Append to the buffer the name of the newly found directory
			_sntprintf(buffer + _tcslen(buffer) - _tcslen((PTCHAR)ALL_FILES), MAX_PATH, FindData.cFileName, sizeof(FindData.cFileName));
			_sntprintf(buffer + _tcslen(buffer), MAX_PATH, (PTCHAR) "\\", _tcslen((PTCHAR) "\\"));
			
			// Recursively call on the new folder
			copy_dir_tree(buffer, threadID, handleAddr);

			// Take out from the buffer string the last folder - equivalent to cd ..
			_sntprintf(buffer, MAX_PATH, source, _tcslen(source));
			_sntprintf(buffer + _tcslen(buffer), MAX_PATH, (PTCHAR) "\\", _tcslen((PTCHAR) "\\"));

		}

	} while (FindNextFile(SearchHandle, &FindData));

	FindClose(SearchHandle);
	return;
}