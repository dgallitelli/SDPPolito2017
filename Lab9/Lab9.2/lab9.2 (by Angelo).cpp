#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#define MAXPATH 256
#define BUFFLEN 1024
#define TYPE_FILE 1
#define TYPE_DIR 2
#define TYPE_DOT 3

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <io.h>

VOID copy_dir_tree(PTCHAR, PTCHAR);
VOID mod_file(PTCHAR);
static DWORD FileType(LPWIN32_FIND_DATA);

int _tmain(int argc, LPTSTR argv[])
{
	if (argc != 3) {
		_ftprintf(stderr, _T("Usage: program_name .\\name1 .\\name2\n"));
		return 1;
	}

	HANDLE direc1;
	TCHAR firstpath[MAXPATH];
	TCHAR secondpath[MAXPATH];
	TCHAR temppath[MAXPATH];

	GetCurrentDirectory(MAXPATH, temppath);
	_stprintf(secondpath, _T("%s\\%s"),temppath, argv[2]);
	_stprintf(firstpath, _T("%s\\%s"), temppath, argv[1]);

	copy_dir_tree(firstpath, secondpath);
	system("pause");
	return 0;
}

VOID copy_dir_tree(PTCHAR source, PTCHAR dest)
{
	HANDLE SearchHandle;
	WIN32_FIND_DATA FindData;
	DWORD FType, l;
	TCHAR NewPath[MAXPATH];
	INT res;

	_tprintf(_T("Creating directory : %s\n"),dest);
	CreateDirectory(dest, NULL);
	SetCurrentDirectory(source);
	SearchHandle = FindFirstFile(_T("*"), &FindData);

	do {
		FType = FileType(&FindData);
		l = _tcslen(dest);
		if (dest[l - 1] == '\\') {
			_stprintf(NewPath, _T("%s%s"), dest, FindData.cFileName);
		}
		else {
			_stprintf(NewPath, _T("%s\\%s"), dest, FindData.cFileName);
		}

		if (FType == TYPE_FILE) {
			CopyFile(FindData.cFileName, NewPath, FALSE);
			mod_file(NewPath);
		}
		if (FType == TYPE_DIR) {
			copy_dir_tree(FindData.cFileName, NewPath);
			SetCurrentDirectory(_T(".."));
		}
		
	} while (FindNextFile(SearchHandle, &FindData));

FindClose(SearchHandle);
return;
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

VOID mod_file(PTCHAR path)
{
	HANDLE hFile;
	LARGE_INTEGER filesize;
	DWORD nbytes;
	INT path_len;
	//PINT8 temp;		
	DWORD nread,nwrite;		
	INT8 first[BUFFLEN];	//usato come locazione temporanea per shiftare tutto il file di (path_len + 4) posizioni in avanti
	INT nbuff;

	path_len = _tcslen(path);
	hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileSizeEx(hFile, &filesize);
	nbytes = filesize.LowPart;
	nbuff = (INT)(nbytes / BUFFLEN) + 1;

	/*temp = (PINT8)malloc(nbytes);

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if (!(ReadFile(hFile, temp, nbytes, &nread, NULL) && nread == nbytes))
	{
		_tprintf(_T("Reading error.\n"));
		_tprintf(_T("Error 1.\n"));
	}

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if (!(WriteFile(hFile, path, sizeof(TCHAR)*path_len, &nwrite, NULL) && nwrite == sizeof(TCHAR)*path_len))
	{
		_tprintf(_T("Writing error.\n"));
		_tprintf(_T("Error 2.\n"));
	}
	if (!(WriteFile(hFile, &nbytes, sizeof(DWORD), &nwrite, NULL) && nwrite == sizeof(DWORD)))
	{
		_tprintf(_T("Writing error.\n"));
		_tprintf(_T("Error 3.\n"));
	}
	if (!(WriteFile(hFile, temp, nbytes, &nwrite, NULL) && nwrite == nbytes))
	{
		_tprintf(_T("Writing error.\n"));
		_tprintf(_T("Error 1.\n"));
	}*/

	//prova di spostamento senza allocazione dinamica
	if (nbytes < BUFFLEN)
	{
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		if (!(ReadFile(hFile, first, BUFFLEN, &nread, NULL) && nread == nbytes))
		{
			_tprintf(_T("Reading error.\n"));
			_tprintf(_T("Error 1.\n"));
		}
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		if (!(WriteFile(hFile, path, sizeof(TCHAR)*path_len, &nwrite, NULL) && nwrite == sizeof(TCHAR)*path_len))
		{
			_tprintf(_T("Writing error.\n"));
			_tprintf(_T("Error 2.\n"));
		}
		if (!(WriteFile(hFile, &nbytes, sizeof(DWORD), &nwrite, NULL) && nwrite == sizeof(DWORD)))
		{
			_tprintf(_T("Writing error.\n"));
			_tprintf(_T("Error 3.\n"));
		}
		if (!(WriteFile(hFile, first, nread, &nwrite, NULL) && nwrite == nread))
		{
			_tprintf(_T("Writing error.\n"));
			_tprintf(_T("Error 4.\n"));
		}
	}
	else
	{
		SetFilePointer(hFile, 0 - BUFFLEN, NULL, FILE_END);
		for (INT i = 0; i < (INT)(nbytes / BUFFLEN) + 1; i++)
		{
			if (ReadFile(hFile, first, sizeof(INT8), &nread, NULL) && nread > 0)
			{
				SetFilePointer(hFile, sizeof(TCHAR)*path_len + 4, NULL, FILE_CURRENT);
				if (!(WriteFile(hFile, first, nread, &nwrite, NULL) && nwrite == nread))
				{
					_tprintf(_T("Writing error.\n"));
					_tprintf(_T("Error 1.\n"));
				}
				SetFilePointer(hFile, 0 - (sizeof(TCHAR)*(path_len)+4) - BUFFLEN, NULL, FILE_CURRENT);
			}
			else
			{
				_tprintf(_T("Reading error.\n"));
				_tprintf(_T("Error 2.\n"));
			}
		}

		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		if (!(WriteFile(hFile, path, sizeof(TCHAR)*path_len, &nwrite, NULL) && nwrite > 0))
		{
			_tprintf(_T("Writing error.\n"));
			_tprintf(_T("Error 3.\n"));
		}
		if (!(WriteFile(hFile, &nbytes, sizeof(DWORD), &nwrite, NULL) && nwrite > 0))
		{
			_tprintf(_T("Writing error.\n"));
			_tprintf(_T("Error 4.\n"));
		}
	}


	return;
}